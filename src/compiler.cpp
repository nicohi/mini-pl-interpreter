#include "compiler.h"
#include "parser.h"
#include "scanner.h"

namespace Compiler {

// Runs only the scanner and prints to stdout
void runScanner(const std::string source) {
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

void runParser(const std::string source) { Parser::parse(source); }
} // namespace Compiler
