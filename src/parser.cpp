#include "parser.h"
#include "scanner.h"
#include <cstdio>
#include <iostream>
#include <list>
#include <map>

namespace Parser {

struct ParserState {
  Scanner::Token current;
  Scanner::Token previous;
  bool hadError = false;
  bool panicMode = false;
};

ParserState parser;

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

class Opnd;
class Int;
class String;
class Ident;
class Expr;
class Binary;
class Unary;
class Single;
class Stmt;
class Stmts;
class Var;
class Assign;
class For;
class Read;
class Print;
class Assert;
class TreeWalker {
public:
  virtual void visitOpnd(const Opnd *i) = 0;
  virtual void visitInt(const Int *i) = 0;
  virtual void visitString(const String *s) = 0;
  virtual void visitIdent(const Ident *i) = 0;
  virtual void visitExpr(const Expr *e) = 0;
  virtual void visitBinary(const Binary *b) = 0;
  virtual void visitUnary(const Unary *u) = 0;
  virtual void visitSingle(const Single *s) = 0;
  virtual void visitStmt(const Stmt *s) = 0;
  virtual void visitStmts(const Stmts *s) = 0;
  virtual void visitVar(const Var *v) = 0;
  virtual void visitAssign(const Assign *a) = 0;
  virtual void visitFor(const For *f) = 0;
  virtual void visitRead(const Read *r) = 0;
  virtual void visitPrint(const Print *p) = 0;
  virtual void visitAssert(const Assert *a) = 0;
};

class TreeNode {
public:
  virtual void accept(TreeWalker *t) = 0;
  std::string hello() { return "HELLO\n"; }
};

class Opnd : public TreeNode {
public:
  void accept(TreeWalker *t) override { t->visitOpnd(this); };
};
class Int : public Opnd {
public:
  Scanner::Token value;
  Int(Scanner::Token v) { this->value = v; }
  void accept(TreeWalker *t) override { t->visitInt(this); };
};
class String : public Opnd {
public:
  Scanner::Token value;
  String(Scanner::Token v) { this->value = v; }
  void accept(TreeWalker *t) override { t->visitString(this); };
};
class Ident : public Opnd {
public:
  Scanner::Token ident;
  Ident(Scanner::Token v) { this->ident = v; }
  void accept(TreeWalker *t) override { t->visitIdent(this); };
};
class Expr : public Opnd {
public:
  Opnd *left;
  Scanner::Token op;
  Opnd *right;
  void accept(TreeWalker *t) override { t->visitExpr(this); };
};
class Binary : public Expr {
public:
  Binary(Parser::Opnd *left, Scanner::Token op, Parser::Opnd *right) {
    this->left = left;
    this->op = op;
    this->right = right;
  }
  void accept(TreeWalker *t) override { t->visitBinary(this); };
};
class Unary : public Expr {
public:
  Unary(Scanner::Token op, Parser::Opnd *right) {
    this->op = op;
    this->right = right;
  }
  void accept(TreeWalker *t) override { t->visitUnary(this); };
};
class Single : public Expr {
public:
  Single(Parser::Opnd *right) { this->right = right; }
  void accept(TreeWalker *t) override { t->visitSingle(this); };
};
class Stmt : public TreeNode {
public:
  std::string info;
  Stmt() { info = "dummy statement"; }
  void accept(TreeWalker *t) override { t->visitStmt(this); };
};
class Stmts : public TreeNode {
public:
  std::list<TreeNode *> stmts;
  void append(Stmt *s) { stmts.push_back(s); }
  void accept(TreeWalker *t) override { t->visitStmts(this); };
};
class Var : public Stmt {
public:
  Scanner::Token ident;
  Scanner::Token type;
  Expr *expr;
  Var() { info = "Var"; }
  void accept(TreeWalker *t) override { t->visitVar(this); };
};
class Assign : public Stmt {
public:
  Scanner::Token ident;
  Expr *expr;
  Assign(Scanner::Token id, Parser::Expr *e) {
    this->ident = id;
    this->expr = e;
    info = "Assign";
  }
  void accept(TreeWalker *t) override { t->visitAssign(this); };
};
class For : public Stmt {
public:
  Scanner::Token ident;
  Expr *from;
  Expr *to;
  Stmts *body;
  For(Scanner::Token id, Parser::Expr *f, Parser::Expr *t, Parser::Stmts *b) {
    this->ident = id;
    this->from = f;
    this->to = t;
    this->body = b;
    info = "For";
  }
  void accept(TreeWalker *t) override { t->visitFor(this); };
};
class Read : public Stmt {
public:
  Scanner::Token ident;
  Read(Scanner::Token i) {
    this->ident = i;
    info = "Read";
  }
  void accept(TreeWalker *t) override { t->visitRead(this); };
};
class Print : public Stmt {
public:
  Expr *expr;
  Print() { info = "Print"; }
  void accept(TreeWalker *t) override { t->visitPrint(this); };
};
class Assert : public Stmt {
public:
  Expr *expr;
  Assert(Parser::Expr *e) {
    this->expr = e;
    info = "Assert";
  }
  void accept(TreeWalker *t) override { t->visitAssert(this); };
};

Stmts program;

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
    parser.panicMode = false;
  }
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
    return new Binary(left, parser.previous, operand());
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
  if (isCurrent(Scanner::TokenType::COMMENT)) {
    advance();
    return statement();
  } else if (isCurrent(Scanner::TokenType::VAR)) {
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
    if (isCurrent(Scanner::TokenType::SCAN_EOF) ||
        isCurrent(Scanner::TokenType::END))
      return s;
    // std::cout << "Parsing statement at: " << Scanner::getName(parser.current)
    // << std::endl;
    s->append(statement());
  }
  return s;
}

class PrintWalker : public TreeWalker {
public:
  void printToken(Scanner::Token t) { printf("%.*s", t.length, t.start); }
  void visitOpnd(const Opnd *i) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "opnd" << std::endl;
  }
  void visitInt(const Int *i) override { printToken(i->value); }
  void visitString(const String *s) override { printToken(s->value); }
  void visitIdent(const Ident *i) override { printToken(i->ident); }
  void visitExpr(const Expr *e) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "DUMMYEXPR";
  }
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
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "stmts" << std::endl;
  }
  void visitVar(const Var *v) override {
    std::cout << "var ident:";
    printToken(v->ident);
    std::cout << " ";
    std::cout << "type:" << Scanner::getName(v->type) << " ";
    if (v->expr) {
      std::cout << "expr:";
      v->expr->accept(this);
    }
  }
  void visitAssign(const Assign *a) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "assign" << std::endl;
  }
  void visitFor(const For *f) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "for" << std::endl;
  }
  void visitRead(const Read *r) override {
    std::cout << "read expr:";
    printToken(r->ident);
  }
  void visitPrint(const Print *p) override {
    std::cout << "print expr:";
    p->expr->accept(this);
  }
  void visitAssert(const Assert *a) override {
    std::cout << "assert expr=";
    a->expr->accept(this);
  }
};

void pprint(Stmts *ss) {
  PrintWalker pw = PrintWalker();
  // std::cout << "FIRST STMT:" << ss->stmts.front() << "\n";
  // std::cout << "LAST STMT:" << ss->stmts.back() << "\n";
  for (TreeNode *s : ss->stmts) {
    // std::cout << "ATTEMPTING ACCEPT:" << s << "\n";
    s->accept(&pw);
    std::cout << std::endl;
  }

  //   Assign *a = new Assign(parser.previous, new Expr());
  //   Print *p = new Print();
  //   std::cout << "TEST: ";
  //   a->accept(&pw);
  //   std::cout << std::endl;
  //   std::cout << "TEST: ";
  //   p->accept(&pw);
  //   std::cout << std::endl;
  //   std::list<TreeNode *> test = {a, p};
  //   for (TreeNode *s : test) {
  //     s->accept(&pw);
  //     std::cout << std::endl;
  //   }
}

bool parse(const std::string source) {
  Scanner::init(source);
  parser.hadError = false;
  parser.panicMode = false;
  advance();
  Stmts *ss = statements();
  consume(Scanner::TokenType::SCAN_EOF, "");
  pprint(ss);
  return !parser.hadError;
}

} // namespace Parser
