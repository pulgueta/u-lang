#ifndef U_LANG_LLVM_h
#define U_LANG_LLVM_h

#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <system_error>
#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "u-parser.h"

using namespace std;

using syntax::ULangParser;

class ULangLLVM {
 public:
  ULangLLVM() : parser(std::make_unique<ULangParser>()) {
    init();
    setupExternalFns();
  }

  void execute(const std::string &program) {
    auto ast = parser->parser(program);

    compile(ast);

    module->print(llvm::outs(), nullptr);

    cout << endl;

    saveFile("./output.ll");
  }

 private:
  std::unique_ptr<llvm::LLVMContext> ctx;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::unique_ptr<ULangParser> parser;
  llvm::Function *fn;

  void init() {
    ctx = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("ULangMain", *ctx);
    builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
  }

  void saveFile(const std::string &fileName) {
    std::error_code err;
    llvm::raw_fd_ostream outLL(fileName, err);
    module->print(outLL, nullptr);
  }

  void compile(const Expression &ast) {
    fn = createFn("main", llvm::FunctionType::get(builder->getInt32Ty(), false));

    auto res = gen(ast);
    auto i32Res = builder->CreateIntCast(res, builder->getInt32Ty(), true);

    builder->CreateRet(i32Res);
  }

  void setupExternalFns() {
    auto bytePtrType = builder->getInt8Ty()->getPointerTo();

    module->getOrInsertFunction("printf", llvm::FunctionType::get(
                                              builder->getInt32Ty(), bytePtrType, true));
  }

  llvm::Function *createFn(const std::string &name, llvm::FunctionType *type) {
    auto fn = module->getFunction(name);

    if (fn == nullptr) {
      fn = createFnPrototype(name, type);
    }

    createFnBlock(fn);

    return fn;
  }

  llvm::Function *createFnPrototype(const std::string &name, llvm::FunctionType *type) {
    auto fn = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, *module);

    verifyFunction(*fn);

    return fn;
  }

  void createFnBlock(llvm::Function *fn) {
    auto entry = createBasicBlock("entry", fn);

    builder->SetInsertPoint(entry);
  }

  llvm::BasicBlock *createBasicBlock(const std::string &name, llvm::Function *fn = nullptr) {
    return llvm::BasicBlock::Create(*ctx, name, fn);
  }

  llvm::Value *gen(const Expression &expr) {
    switch (expr.type) {
      case ExpressionType::NUMBER:
        return builder->getInt32(expr.number);

      case ExpressionType::STRING: {
        auto re = std::regex("\\\\n");
        auto str = std::regex_replace(expr.string, re, "\n");

        return builder->CreateGlobalStringPtr(str);
      }

      case ExpressionType::SYMBOL:
        return genSymbol(expr.string);

      case ExpressionType::LIST:
        auto t = expr.list[0];

        if (t.type == ExpressionType::SYMBOL) {
          auto op = t.string;

          if (op == "printf") {
            auto printFn = module->getFunction("printf");

            vector<llvm::Value *> args{str};

            for (auto i = 0; i < expr.list.size(); i++) {
              args.push_back(gen(expr.list[i]));
            }

            return builder->CreateCall(printFn, args);
          }
        }
    };

    return builder->getInt32(0);
  }
};

#endif
