#include "compiler.h"
#include "scanner.h"
#include <cstdio>

namespace Compiler {
void compile(const std::string source) {
  Scanner::init(source);
  int line = -1;
  for (;;) {
    Scanner::Token token = Scanner::scanToken();
    if (token.line != line) {
      printf("%4d ", token.line);
      line = token.line;
    } else {
      printf("   | ");
    }
    printf("%-12s '%.*s' %s\n", Scanner::getName(token).c_str(), token.length,
           token.start, token.message);

    if (token.type == Scanner::TokenType::SCAN_EOF)
      break;
  }
}
} // namespace Compiler
