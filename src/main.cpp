#include <string>

#include "u-llvm.h"

int main(int argc, char const *argv[]) {
  std::string program = R"(Hello, World!)";

  ULangLLVM vm;

  vm.execute(program);

  return 0;
}
