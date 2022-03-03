#include "compiler.h"
#include "scanner.h"
#include <cstdio>
#include <iostream>

namespace Compiler {
void compile(const std::string source) {
  Scanner::init(source);
  int line = -1;
  for (;;) {
    Scanner::Token token = Scanner::scanToken();
    if (token.line != line) {
      line = token.line;
    }
    printf("%-10s '%.*s'\n", Scanner::getName(token).c_str(), token.length,
           token.start);

    if (token.type == Scanner::TokenType::SCAN_EOF)
      break;
  }
}
} // namespace Compiler
