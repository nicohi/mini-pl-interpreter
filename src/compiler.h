#ifndef COMPILER_H_
#define COMPILER_H_

#include <string>

namespace Compiler {

void runScanner(const std::string source);
void runParser(const std::string source);

} // namespace Compiler

#endif // COMPILER_H_
