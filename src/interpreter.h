#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <string>

namespace Interpreter {

enum class InterpretResult {
  OK,
  RUNTIME_ERROR,
  COMPILE_ERROR,
};

InterpretResult interpret(const std::string source);

} // namespace Interpreter

#endif // INTERPRETER_H_
