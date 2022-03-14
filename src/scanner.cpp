#include "scanner.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace Scanner {

#define F(name, desc) #name,
std::string TokenName[]{TOKEN_TYPES(F)};
#undef F

struct Scanner {
  char *src;
  const char *start;
  const char *current;
  int line;
};

static Scanner scanner;

void init(const std::string source) {
  scanner.src = (char *)std::malloc(source.size() + 1);
  std::memcpy(scanner.src, source.c_str(), source.size());
  scanner.start = scanner.src;
  scanner.current = scanner.src;
  scanner.line = 1;
}

std::string getName(Token t) { return TokenName[static_cast<int>(t.type)]; }
std::string getName(TokenType t) { return TokenName[static_cast<int>(t)]; }

static bool isEnd() { return *scanner.current == '\0'; }

static Token makeToken(TokenType type) {
  Token t;
  t.type = type;
  t.start = scanner.start;
  t.length = (int)(scanner.current - scanner.start);
  t.line = scanner.line;
  t.message = "";
  return t;
}

Token errorToken(const char *msg) {
  Token t = makeToken(TokenType::ERROR);
  t.message = msg;
  return t;
}

static char peek() { return *scanner.current; }

static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

static bool match(char expected) {
  if (isEnd())
    return false;
  if (*scanner.current != expected)
    return false;
  advance();
  return true;
}

static bool matchS(std::string e) {
  if (isEnd())
    return false;
  const char *expected = e.c_str();
  int count = e.length();
  // printf("Count:%d\n", count);
  for (int i = 0; i < count - 1; i++) {
    // printf("Current:%c Expected:%c\n", scanner.current[i], expected[i]);
    if (scanner.current[i] != expected[i])
      return false;
  }
  scanner.current += count;
  return true;
}

static void skipWhitespace() {
  for (;;) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      scanner.line++;
      advance();
      break;
    default:
      return;
    }
  }
}

static bool gotoChar(char c) {
  while (!isEnd()) {
    if (peek() == c) {
      advance();
      return true;
    }
    advance();
  }
  return false;
}

static Token string() {
  while (peek() != '"' && !isEnd()) {
    if (peek() == '\\') {
      if (scanner.current[1] == '\"') {
        advance();
      }
    }
    if (peek() == '\n')
      scanner.line++;
    advance();
  }
  if (isEnd())
    return errorToken("Unterminated string.");
  advance();
  return makeToken(TokenType::STRING_LIT);
}

static bool isDigit(char c) { return '0' <= c && c <= '9'; }

static bool isAlpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

static Token integer() {
  while (isDigit(peek()))
    advance();
  return makeToken(TokenType::INTEGER_LIT);
}

static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek()) || peek() == '_')
    advance();
  return makeToken(TokenType::IDENTIFIER);
}

Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;
  if (isEnd())
    return makeToken(TokenType::SCAN_EOF);
  char c = advance();
  switch (c) {
  case '/':
    if (peek() == '/') {
      gotoChar('\n');
      return makeToken(TokenType::COMMENT);
    }
    if (peek() == '*') {
      advance();
      for (;;) {
        gotoChar('*');
        if (match('/'))
          return makeToken(TokenType::COMMENT);
      }
      return makeToken(TokenType::COMMENT);
    }
    return makeToken(TokenType::SLASH);
  case '(':
    return makeToken(TokenType::LEFT_PAREN);
  case ')':
    return makeToken(TokenType::RIGHT_PAREN);
  case '-':
    return makeToken(TokenType::MINUS);
  case '+':
    return makeToken(TokenType::PLUS);
  case '*':
    return makeToken(TokenType::ASTERISK);
  case '=':
    return makeToken(TokenType::EQUAL);
  case '<':
    return makeToken(TokenType::LESS);
  case '&':
    return makeToken(TokenType::AND);
  case '!':
    return makeToken(TokenType::NOT);
  case ';':
    return makeToken(TokenType::SEMICOLON);
  case ':':
    return makeToken(match('=') ? TokenType::ASSIGN : TokenType::COLON);
  case '.':
    if (match('.'))
      return makeToken(TokenType::RANGE);
    break;
  case 'v':
    if (matchS("ar ")) {
      scanner.current--;
      return makeToken(TokenType::VAR);
    }
    break;
  case 'f':
    if (matchS("or ")) {
      scanner.current--;
      return makeToken(TokenType::FOR);
    }
    if (matchS("alse ")) {
      scanner.current--;
      return makeToken(TokenType::BOOLEAN_LIT);
    }
    break;
  case 'e':
    if (matchS("nd ")) {
      scanner.current--;
      return makeToken(TokenType::END);
    }
    break;
  case 'i':
    if (matchS("nt ")) {
      scanner.current--;
      return makeToken(TokenType::INT);
    }
    if (matchS("n ")) {
      scanner.current--;
      return makeToken(TokenType::IN);
    }
    break;
  case 'd':
    if (matchS("o ")) {
      scanner.current--;
      return makeToken(TokenType::DO);
    }
    break;
  case 'r':
    if (matchS("ead ")) {
      scanner.current--;
      return makeToken(TokenType::READ);
    }
    break;
  case 'p':
    if (matchS("rint ")) {
      scanner.current--;
      return makeToken(TokenType::PRINT);
    }
    break;
  case 's':
    if (matchS("tring ")) {
      scanner.current--;
      return makeToken(TokenType::STRING);
    }
    break;
  case 'b':
    if (matchS("ool ")) {
      scanner.current--;
      return makeToken(TokenType::BOOL);
    }
    break;
  case 'a':
    if (matchS("ssert ")) {
      scanner.current--;
      return makeToken(TokenType::ASSERT);
    }
    break;
  case 't':
    if (matchS("rue ")) {
      scanner.current--;
      return makeToken(TokenType::BOOLEAN_LIT);
    }
    break;
  case '"':
    return string();
  }
  if (isDigit(c))
    return integer();
  if (isAlpha(c))
    return identifier();
  return errorToken("Unexpected character.");
}

} // namespace Scanner
