#ifndef TOKENS_H_
#define TOKENS_H_

enum class TokenType {
  // Single-character tokens
  LEFT_PAREN,  // (
  RIGHT_PAREN, // )
  LEFT_BRACE,  // [
  RIGHT_BRACE, // ]
  SEMICOLON,   // ;
  COLON,       // :
  MINUS,       // -
  PLUS,        // +
  SLASH,       // /
  ASTERISK,    // *
  EQUAL,       // =
  AND,         // &
  NOT,         // !

  // 2-character tokens
  ASSIGN, // :=
  RANGE,  // ..

  // Reserved keywords
  VAR,    // "var"
  FOR,    // "for"
  END,    // "end"
  IN,     // "in"
  DO,     // "do"
  READ,   // "read"
  PRINT,  // "print"
  INT,    // "int"
  STRING, // "string"
  BOOL,   // "bool"
  ASSERT, // "assert"

  // Multi-character tokens and literals
  COMMENT,     // "// ... \n" | "/* ... */"
  IDENTIFIER,  // letter ( digit | "_" | letter )*
  STRING_LIT,  // """ character character* """
  INTEGER_LIT, // digit+
  BOOLEAN_LIT, // "true" | "false"

  // Extra tokens
  ERROR,
  EOF,
};
#endif // TOKENS_H_
