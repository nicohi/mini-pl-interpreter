#include "interpreter.h"
#include "compiler.h"
#include <iostream>

namespace Interpreter {

InterpretResult interpret(const std::string source) {
  try {
    Compiler::compile(source);
  } catch (int e) {
    return InterpretResult::COMPILE_ERROR;
  }
  return InterpretResult::OK;
}

} // namespace Interpreter