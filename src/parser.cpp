#include "parser.h"
#include "compiler.h"
#include <cstdio>
#include <iostream>
#include <map>

namespace Parser {

struct ParserState {
  Scanner::Token current;
  Scanner::Token previous;
  bool hadError = false;
  bool panicMode = false;
};

ParserState parser;

Stmts *program;

enum class Precedence {
  NONE,
  ASSIGN, // :=
  AND,    // &
  EQUAL,  // =
  TERM,   // + -
  FACTOR, // * /
  UNARY,  // - !
  PRIMARY
};

typedef void (*ParseFn)();

struct ParseRule {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};
std::map<Scanner::TokenType, ParseRule> rules;

static void printCurrent(std::string msg) {
  std::cout << msg << Scanner::getName(parser.current) << std::endl;
}

static bool isCurrent(Scanner::TokenType t) { return parser.current.type == t; }

static void errorAt(Scanner::Token t, std::string msg) {
  if (parser.panicMode)
    return;
  parser.panicMode = true;
  fprintf(stderr, "[line %d] Error", t.line);

  if (t.type == Scanner::TokenType::SCAN_EOF) {
    fprintf(stderr, " at end");
  } else if (t.type == Scanner::TokenType::ERROR) {
    fprintf(stderr, " %s", t.message);
  } else {
    fprintf(stderr, " at '%.*s'", t.length, t.start);
  }

  fprintf(stderr, ": %s\n", msg.c_str());
  parser.hadError = true;
}

static void advance() {
  parser.previous = parser.current;
  for (;;) {
    parser.current = Scanner::scanToken();
    if (!isCurrent(Scanner::TokenType::ERROR))
      break;
    errorAt(parser.current, "Scanner error");
  }
}

static void consume(Scanner::TokenType type, std::string msg) {
  if (parser.current.type == type) {
    advance();
    return;
  }
  errorAt(parser.current, msg);
}

static void exitPanic() {
  while (!isCurrent(Scanner::TokenType::SEMICOLON)) {
    if (isCurrent(Scanner::TokenType::SCAN_EOF))
      break;
    std::cout << "Skipping token:" << Scanner::getName(parser.current)
              << std::endl;
    advance();
  }
  parser.panicMode = false;
}

static bool isBinaryOp() {
  return isCurrent(Scanner::TokenType::PLUS) ||
         isCurrent(Scanner::TokenType::MINUS) ||
         isCurrent(Scanner::TokenType::ASTERISK) ||
         isCurrent(Scanner::TokenType::SLASH) ||
         isCurrent(Scanner::TokenType::LESS) ||
         isCurrent(Scanner::TokenType::EQUAL) ||
         isCurrent(Scanner::TokenType::AND);
}

static bool isUnaryOp() { return isCurrent(Scanner::TokenType::NOT); }

static bool isType() {
  return isCurrent(Scanner::TokenType::INT) ||
         isCurrent(Scanner::TokenType::STRING) ||
         isCurrent(Scanner::TokenType::BOOL);
}
static Expr *expression();

static Opnd *operand() {
  if (isCurrent(Scanner::TokenType::INTEGER_LIT)) {
    advance();
    return new Int(parser.previous);
  }
  if (isCurrent(Scanner::TokenType::STRING_LIT)) {
    advance();
    return new String(parser.previous);
  }
  if (isCurrent(Scanner::TokenType::BOOLEAN_LIT)) {
    advance();
    return new Bool(parser.previous);
  }
  if (isCurrent(Scanner::TokenType::IDENTIFIER)) {
    advance();
    return new Ident(parser.previous);
  }
  consume(Scanner::TokenType::LEFT_PAREN,
          "Expected literal, identifier, or '('");
  Expr *e = expression();
  consume(Scanner::TokenType::RIGHT_PAREN, "Expected ')'");
  return e;
}

static Expr *expression() {
  if (isUnaryOp()) {
    advance();
    return new Unary(parser.previous, operand());
  }
  Opnd *left = operand();
  if (isBinaryOp()) {
    advance();
    return new Binary(left, parser.previous, expression());
  } else
    return new Single(left);
}

static Var *var() {
  advance();
  Var *v = new Var();
  consume(Scanner::TokenType::IDENTIFIER, "Expected an identifier after 'var'");
  v->ident = parser.previous;
  consume(Scanner::TokenType::COLON, "Expected an ':' after identifier");
  if (isType()) {
    v->type = parser.current;
    advance();
  } else {
    errorAt(parser.current, "Expected type after ':'");
  }
  if (isCurrent(Scanner::TokenType::ASSIGN)) {
    advance();
    v->expr = expression();
  }
  return v;
}

static Assign *assign() {
  advance();
  Scanner::Token id = parser.previous;
  consume(Scanner::TokenType::ASSIGN, "Expected ':=' after identifier");
  Expr *e = expression();
  return new Assign(id, e);
}

static Print *print() {
  advance();
  Print *p = new Print();
  p->expr = expression();
  return p;
}

static Read *read() {
  advance();
  consume(Scanner::TokenType::IDENTIFIER, "Expected identifier after read");
  return new Read(parser.previous);
}

static Assert *assert() {
  advance();
  consume(Scanner::TokenType::LEFT_PAREN, "Expected '(' after assert");
  Expr *e = expression();
  consume(Scanner::TokenType::RIGHT_PAREN,
          "Expected ')' after assert expression");
  return new Assert(e);
}

static Stmts *statements();
static For *forLoop() {
  advance();
  consume(Scanner::TokenType::IDENTIFIER, "Expected identifier after for");
  Scanner::Token id = parser.previous;
  consume(Scanner::TokenType::IN, "Expected 'in' after identifier");
  Expr *from = expression();
  consume(Scanner::TokenType::RANGE, "Expected '..' after expression");
  Expr *to = expression();
  consume(Scanner::TokenType::DO, "Expected 'do' after expression");
  Stmts *body = statements();
  consume(Scanner::TokenType::END, "Expected 'end' after loop body");
  consume(Scanner::TokenType::FOR, "Expected 'for' after end");
  return new For(id, from, to, body);
}

static Stmt *statement() {
  Stmt *s = new Stmt();
  if (isCurrent(Scanner::TokenType::VAR)) {
    s = var();
  } else if (isCurrent(Scanner::TokenType::IDENTIFIER)) {
    s = assign();
  } else if (isCurrent(Scanner::TokenType::FOR)) {
    s = forLoop();
  } else if (isCurrent(Scanner::TokenType::READ)) {
    s = read();
  } else if (isCurrent(Scanner::TokenType::PRINT)) {
    s = print();
  } else if (isCurrent(Scanner::TokenType::ASSERT)) {
    s = assert();
  } else
    exitPanic();
  consume(Scanner::TokenType::SEMICOLON, "Expected ';' at end of statement");
  return s;
}
static Stmts *statements() {
  Stmts *s = new Stmts();
  for (;;) {
    if (isCurrent(Scanner::TokenType::COMMENT)) {
      advance();
      continue;
    }
    if (isCurrent(Scanner::TokenType::ERROR)) {
      errorAt(parser.current, parser.current.message);
      exitPanic();
      continue;
    }
    if (isCurrent(Scanner::TokenType::SCAN_EOF) ||
        isCurrent(Scanner::TokenType::END)) {
      return s;
    }
    // std::cout << "Parsing statement at: " << Scanner::getName(parser.current)
    // << std::endl;
    s->append(statement());
  }
  return s;
}

class PrintWalker : public TreeWalker {
public:
  void printToken(Scanner::Token t) { printf("%.*s", t.length, t.start); }
  void visitOpnd(const Opnd *i) override { std::cout << "DUMMYOPND"; }
  void visitInt(const Int *i) override { printToken(i->value); }
  void visitBool(const Bool *b) override { printToken(b->value); }
  void visitString(const String *s) override { printToken(s->value); }
  void visitIdent(const Ident *i) override { printToken(i->ident); }
  void visitExpr(const Expr *e) override { std::cout << "DUMMYEXPR"; }
  void visitBinary(const Binary *b) override {
    std::cout << "(";
    b->left->accept(this);
    std::cout << " ";
    printToken(b->op);
    std::cout << " ";
    b->right->accept(this);
    std::cout << ")";
  }
  void visitUnary(const Unary *u) override {
    std::cout << "(";
    printToken(u->op);
    std::cout << " ";
    u->right->accept(this);
    std::cout << ")";
  }
  void visitSingle(const Single *s) override {
    std::cout << "(";
    s->right->accept(this);
    std::cout << ")";
  }
  void visitStmt(const Stmt *s) override {
    std::cout << "(stmt ";
    std::cout << s->info;
    std::cout << ")";
  }
  void visitStmts(const Stmts *s) override {
    std::cout << "(stmts\n";
    for (TreeNode *n : s->stmts) {
      n->accept(this);
      std::cout << std::endl;
    }
    std::cout << ")\n";
  }
  void visitVar(const Var *v) override {
    std::cout << "(var ident:";
    printToken(v->ident);
    std::cout << " ";
    std::cout << "type:" << Scanner::getName(v->type) << " ";
    if (v->expr) {
      std::cout << "expr:";
      v->expr->accept(this);
    }
    std::cout << ")";
  }
  void visitAssign(const Assign *a) override {
    std::cout << "(";
    std::cout << "assign ident:";
    printToken(a->ident);
    std::cout << " expr:";
    a->expr->accept(this);
    std::cout << ")";
  }
  void visitFor(const For *f) override {
    std::cout << "(";
    std::cout << "for ";
    std::cout << "ident:";
    printToken(f->ident);
    std::cout << " from:";
    f->from->accept(this);
    std::cout << " to:";
    f->to->accept(this);
    std::cout << " body:\n";
    f->body->accept(this);
    std::cout << "end for";
    std::cout << ")";
  }
  void visitRead(const Read *r) override {
    std::cout << "(";
    std::cout << "read expr:";
    printToken(r->ident);
    std::cout << ")";
  }
  void visitPrint(const Print *p) override {
    std::cout << "(";
    std::cout << "print expr:";
    p->expr->accept(this);
    std::cout << ")";
  }
  void visitAssert(const Assert *a) override {
    std::cout << "(";
    std::cout << "assert expr:";
    a->expr->accept(this);
    std::cout << ")";
  }
};

void pprint(Stmts *ss) {
  PrintWalker *pw = new PrintWalker();
  ss->accept(pw);
}

bool parse(const std::string source) {
  Scanner::init(source);
  parser.hadError = false;
  parser.panicMode = false;
  advance();
  program = statements();
  consume(Scanner::TokenType::SCAN_EOF, "");
  pprint(program);
  return !parser.hadError;
}

void parseAndWalk(const std::string source, TreeWalker *tw) {
  Scanner::init(source);
  parser.hadError = false;
  parser.panicMode = false;
  advance();
  program = statements();
  consume(Scanner::TokenType::SCAN_EOF, "");
  if (!parser.hadError)
    program->accept(tw);
}

} // namespace Parser
