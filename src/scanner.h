#ifndef SCANNER_H_
#define SCANNER_H_

#include "string"

namespace Scanner {

#define TOKEN_TYPES(F)                                                         \
  F(LEFT_PAREN, "(")                                                           \
  F(RIGHT_PAREN, ")")                                                          \
  F(LEFT_BRACE, "[")                                                           \
  F(RIGHT_BRACE, "]")                                                          \
  F(MINUS, "-")                                                                \
  F(PLUS, "+")                                                                 \
  F(SLASH, "/")                                                                \
  F(ASTERISK, "*")                                                             \
  F(LESS, "<")                                                                 \
  F(EQUAL, "=")                                                                \
  F(AND, "&")                                                                  \
  F(NOT, "!")                                                                  \
  F(SEMICOLON, ";")                                                            \
  F(COLON, ":")                                                                \
  F(ASSIGN, ":=")                                                              \
  F(RANGE, "..")                                                               \
  F(VAR, "var")                                                                \
  F(FOR, "for")                                                                \
  F(END, "end")                                                                \
  F(IN, "in")                                                                  \
  F(INT, "int")                                                                \
  F(DO, "do")                                                                  \
  F(READ, "read")                                                              \
  F(PRINT, "print")                                                            \
  F(STRING, "string")                                                          \
  F(BOOL, "bool")                                                              \
  F(ASSERT, "assert")                                                          \
  F(COMMENT, "// ... \n | /* ... */")                                          \
  F(IDENTIFIER, "letter ( digit | _ | letter )*")                              \
  F(STRING_LIT, "character character*")                                        \
  F(INTEGER_LIT, "digit+")                                                     \
  F(BOOLEAN_LIT, "true | false")                                               \
  F(ERROR, "")                                                                 \
  F(SCAN_EOF, "")

#define F(name, desc) name,
enum class TokenType { TOKEN_TYPES(F) };
#undef F

struct Token {
  TokenType type;
  const char *start;
  const char *message;
  int length;
  int line;
};

void init(const std::string source);
std::string getName(Token t);
std::string getName(TokenType t);
Token scanToken();
Token errorToken(const char *msg);

} // namespace Scanner

#endif // SCANNER_H_
