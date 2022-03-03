#include "scanner.h"
#include <iostream>

namespace Scanner {

#define F(name, desc) #name,
std::string TokenName[]{TOKEN_TYPES(F)};
#undef F

struct Scanner {
  const char *src;
  const char *start;
  const char *current;
  int line;
};

Scanner scanner;

void init(const std::string source) {
  scanner.src = source.c_str();
  scanner.start = scanner.src;
  scanner.current = scanner.src;
  scanner.line = 1;
}

std::string getName(Token t) { return TokenName[static_cast<int>(t.type)]; }

Token scanToken() {
  Token t;
  t.type = TokenType::SCAN_EOF;
  return t;
}

} // namespace Scanner
