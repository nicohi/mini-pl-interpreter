#include "scanner.h"
#include <iostream>

namespace Scanner {

std::string TokenName[]{
    // Single-character tokens
    "LEFT_PAREN",  // (
    "RIGHT_PAREN", // )
    "LEFT_BRACE",  // [
    "RIGHT_BRACE", // ]
    "SEMICOLON",   // ;
    "COLON",       // :
    "MINUS",       // -
    "PLUS",        // +
    "SLASH",       // /
    "ASTERISK",    // *
    "EQUAL",       // =
    "AND",         // &
    "NOT",         // !

    // 2-character tokens
    "ASSIGN", // :=
    "RANGE",  // ..

    // Reserved keywords
    "VAR",    // "var"
    "FOR",    // "for"
    "END",    // "end"
    "IN",     // "in"
    "DO",     // "do"
    "READ",   // "read"
    "PRINT",  // "print"
    "INT",    // "int"
    "STRING", // "string"
    "BOOL",   // "bool"
    "ASSERT", // "assert"

    // Multi-character tokens and literals
    "COMMENT",     // "// ... \n" | "/* ... */"
    "IDENTIFIER",  // letter ( digit | "_" | letter )*
    "STRING_LIT",  // """ character character* """
    "INTEGER_LIT", // digit+
    "BOOLEAN_LIT", // "true" | "false"

    // Extra tokens
    "ERROR",
    "SCAN_EOF",
};

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
