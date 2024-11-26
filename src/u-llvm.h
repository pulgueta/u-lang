#ifndef U_LANG_LLVM_h
#define U_LANG_LLVM_h

#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

using namespace std;

class ULangLLVM {
 public:
  ULangLLVM() {
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

  void compile(const std::string &ast) {
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

  llvm::Value *gen() {
    auto str = builder->CreateGlobalStringPtr("Generated from LLVM");

    auto printFn = module->getFunction("printf");

    vector<llvm::Value *> args{str};

    return builder->CreateCall(printFn, args)
  }
};

#endif
