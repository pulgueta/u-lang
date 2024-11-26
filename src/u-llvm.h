#ifndef U_LANG_LLVM_h
#define U_LANG_LLVM_h

#include <memory>
#include <string>
#include <system_error>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

class ULangLLVM {
 public:
  ULangLLVM() { init(); }

  void execute(const std::string &program) {
    module->print(llvm::outs(), nullptr);

    saveFile("./output.ll");
  }

 private:
  std::unique_ptr<llvm::LLVMContext> ctx;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<llvm::IRBuilder<>> builder;

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

  llvm::Function *createFn(const std::string &name, llvm::FunctionType *type) {}

  llvm::Value *gen() {
    return builder->getInt32(42);
  }
};

#endif
