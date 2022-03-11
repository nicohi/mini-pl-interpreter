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
  virtual void accept(TreeWalker *t) const = 0;
};

class Opnd : public TreeNode {
public:
  void accept(TreeWalker *t) const override { t->visitOpnd(this); };
};
class Int : public Opnd {
public:
  Scanner::Token value;
  Int(Scanner::Token v) { this->value = v; }
  void accept(TreeWalker *t) const override { t->visitInt(this); };
};
class String : public Opnd {
public:
  Scanner::Token value;
  String(Scanner::Token v) { this->value = v; }
  void accept(TreeWalker *t) const override { t->visitString(this); };
};
class Ident : public Opnd {
public:
  Scanner::Token ident;
  Ident(Scanner::Token v) { this->ident = v; }
  void accept(TreeWalker *t) const override { t->visitIdent(this); };
};
class Expr : public Opnd {
public:
  Opnd *left;
  Scanner::Token op;
  Opnd *right;
  void accept(TreeWalker *t) const override { t->visitExpr(this); };
};
class Binary : public Expr {
public:
  Binary(Parser::Opnd left, Scanner::Token op, Parser::Opnd right) {
    this->left = &left;
    this->op = op;
    this->right = &right;
  }
  void accept(TreeWalker *t) const override { t->visitBinary(this); };
};
class Unary : public Expr {
public:
  Unary(Scanner::Token op, Parser::Opnd right) {
    this->op = op;
    this->right = &right;
  }
  void accept(TreeWalker *t) const override { t->visitUnary(this); };
};
class Single : public Expr {
public:
  Single(Parser::Opnd right) { this->right = &right; }
  void accept(TreeWalker *t) const override { t->visitSingle(this); };
};
class Stmt : public TreeNode {
public:
  std::string info;
  Stmt() { info = "dummy statement"; }
  // void accept(TreeWalker *t) { t->visitStmt(this); };
  void accept(TreeWalker *t);
};
class Stmts : public TreeNode {
public:
  std::list<Stmt *> stmts;
  void append(Stmt *s) { stmts.push_back(s); }
  void accept(TreeWalker *t) const override { t->visitStmts(this); };
};
class Var : public Stmt {
public:
  Scanner::Token ident;
  Scanner::Token type;
  Expr expr;
  Var() { info = "Var"; }
  // void accept(TreeWalker *t) const override { t->visitVar(this); };
  void accept(TreeWalker *t);
};
class Assign : public Stmt {
public:
  Scanner::Token ident;
  Expr expr;
  Assign(Scanner::Token id, Parser::Expr e) {
    this->ident = id;
    this->expr = e;
    info = "Assign";
  }
  void accept(TreeWalker *t) const override { t->visitAssign(this); };
};
class For : public Stmt {
public:
  Scanner::Token ident;
  Expr from;
  Expr to;
  Stmts body;
  For(Scanner::Token id, Parser::Expr f, Parser::Expr t, Parser::Stmts b) {
    this->ident = id;
    this->from = f;
    this->to = t;
    this->body = b;
    info = "For";
  }
  void accept(TreeWalker *t) const override { t->visitFor(this); };
};
class Read : public Stmt {
public:
  Scanner::Token ident;
  Read(Scanner::Token i) {
    this->ident = i;
    info = "Read";
  }
  void accept(TreeWalker *t) const override { t->visitRead(this); };
};
class Print : public Stmt {
public:
  Expr expr;
  Print() { info = "Print"; }
  void accept(TreeWalker *t) const override { t->visitPrint(this); };
};
class Assert : public Stmt {
public:
  Expr expr;
  Assert(Parser::Expr e) {
    this->expr = e;
    info = "Assert";
  }
  void accept(TreeWalker *t) const override { t->visitAssert(this); };
};

void Stmt::accept(TreeWalker *t) { t->visitStmt(this); }
void Var::accept(TreeWalker *t) { t->visitStmt(this); }

class TreeNodeFactory {
public:
  TreeNode *CreateInstance();
};
template <class T> class Factory : public TreeNodeFactory {
public:
  TreeNode *CreateInstance() { return new T(); }
};
std::map<std::string, TreeNodeFactory *> fs;
static void init() {
  fs["Opnd"] = new Factory<Opnd>();
  fs["Int"] = new Factory<Int>();
  fs["String"] = new Factory<String>();
  fs["Ident"] = new Factory<Ident>();
  fs["Expr"] = new Factory<Expr>();
  fs["Binary"] = new Factory<Binary>();
  fs["Unary"] = new Factory<Unary>();
  fs["Single"] = new Factory<Single>();
  fs["Stmt"] = new Factory<Stmt>();
  fs["Stmts"] = new Factory<Stmts>();
  fs["Var"] = new Factory<Var>();
  fs["Assign"] = new Factory<Assign>();
  fs["For"] = new Factory<For>();
  fs["Read"] = new Factory<Read>();
  fs["Print"] = new Factory<Print>();
  fs["Assert"] = new Factory<Assert>();
  parser.hadError = false;
  parser.panicMode = false;
}

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
static Expr expression();

static Opnd operand() {
  if (isCurrent(Scanner::TokenType::INTEGER_LIT)) {
    advance();
    return Int(parser.previous);
  }
  if (isCurrent(Scanner::TokenType::STRING_LIT)) {
    advance();
    return String(parser.previous);
  }
  if (isCurrent(Scanner::TokenType::IDENTIFIER)) {
    advance();
    return Ident(parser.previous);
  }
  consume(Scanner::TokenType::LEFT_PAREN,
          "Expected literal, identifier, or '('");
  Expr e = expression();
  consume(Scanner::TokenType::RIGHT_PAREN, "Expected ')'");
  return e;
}

static Expr expression() {
  if (isUnaryOp()) {
    advance();
    return Unary(parser.previous, operand());
  }
  Opnd left = operand();
  if (isBinaryOp()) {
    advance();
    return Binary(left, parser.previous, operand());
  } else
    return Single(left);
}

static Var var() {
  advance();
  Var v = Var();
  consume(Scanner::TokenType::IDENTIFIER, "Expected an identifier after 'var'");
  v.ident = parser.previous;
  consume(Scanner::TokenType::COLON, "Expected an ':' after identifier");
  if (isType()) {
    v.type = parser.current;
    advance();
  } else {
    errorAt(parser.current, "Expected type after ':'");
  }
  if (isCurrent(Scanner::TokenType::ASSIGN)) {
    advance();
    v.expr = expression();
  }
  return v;
}

static Assign assign() {
  advance();
  Scanner::Token id = parser.previous;
  consume(Scanner::TokenType::ASSIGN, "Expected ':=' after identifier");
  Expr e = expression();
  return Assign(id, e);
}

static Print print() {
  advance();
  Print p = Print();
  p.expr = expression();
  return p;
}

static Read read() {
  advance();
  consume(Scanner::TokenType::IDENTIFIER, "Expected identifier after read");
  return Read(parser.previous);
}

static Assert assert() {
  advance();
  consume(Scanner::TokenType::LEFT_PAREN, "Expected '(' after assert");
  Expr e = expression();
  consume(Scanner::TokenType::RIGHT_PAREN,
          "Expected ')' after assert expression");
  return Assert(e);
}

static Stmts statements();
static For forLoop() {
  advance();
  consume(Scanner::TokenType::IDENTIFIER, "Expected identifier after for");
  Scanner::Token id = parser.previous;
  consume(Scanner::TokenType::IN, "Expected 'in' after identifier");
  Expr from = expression();
  consume(Scanner::TokenType::RANGE, "Expected '..' after expression");
  Expr to = expression();
  consume(Scanner::TokenType::DO, "Expected 'do' after expression");
  Stmts body = statements();
  consume(Scanner::TokenType::END, "Expected 'end' after loop body");
  consume(Scanner::TokenType::FOR, "Expected 'for' after end");
  return For(id, from, to, body);
}

static Stmt *statement() {
  Stmt *s = new Stmt();
  if (isCurrent(Scanner::TokenType::COMMENT)) {
    advance();
    return statement();
  } else if (isCurrent(Scanner::TokenType::VAR)) {
    Var v = var();
    s = &v;
    // s = &var();
  } else if (isCurrent(Scanner::TokenType::IDENTIFIER)) {
    Assign v = assign();
    s = &v;
    // s = &assign();
  } else if (isCurrent(Scanner::TokenType::FOR)) {
    For v = forLoop();
    s = &v;
    // s = &forLoop();
  } else if (isCurrent(Scanner::TokenType::READ)) {
    Read v = read();
    s = &v;
    // s = &read();
  } else if (isCurrent(Scanner::TokenType::PRINT)) {
    Print v = print();
    s = &v;
    // s = &print();
  } else if (isCurrent(Scanner::TokenType::ASSERT)) {
    Assert v = assert();
    s = &v;
    // s = &assert();
  } else
    exitPanic();
  consume(Scanner::TokenType::SEMICOLON, "Expected ';' at end of statement");
  return s;
}
static Stmts statements() {
  Stmts s;
  for (;;) {
    if (isCurrent(Scanner::TokenType::SCAN_EOF) ||
        isCurrent(Scanner::TokenType::END))
      return s;
    // std::cout << "Parsing statement at: " << Scanner::getName(parser.current)
    // << std::endl;
    s.append(statement());
  }
  return s;
}

class PrintWalker : public TreeWalker {
public:
  void visitOpnd(const Opnd *i) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "opnd" << std::endl;
  }
  void visitInt(const Int *i) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "int" << std::endl;
  }
  void visitString(const String *s) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "string" << std::endl;
  }
  void visitIdent(const Ident *i) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "ident" << std::endl;
  }
  void visitExpr(const Expr *e) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "expr" << std::endl;
  }
  void visitBinary(const Binary *b) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "binary_expr" << std::endl;
  }
  void visitUnary(const Unary *u) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "unary_expr" << std::endl;
  }
  void visitSingle(const Single *s) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "single_expr" << std::endl;
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
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "var" << std::endl;
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
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "read" << std::endl;
  }
  void visitPrint(const Print *p) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "print" << std::endl;
  }
  void visitAssert(const Assert *a) override {
    // std::cout << "NOT_IMPLEMENTED" << std::endl;
    std::cout << "assert" << std::endl;
  }
};

// TODO implement factory
// https://stackoverflow.com/questions/1883862/c-oop-list-of-classes-class-types-and-creating-instances-of-them
void pprint(Stmts ss) {
  PrintWalker pw = PrintWalker();
  for (Stmt *s : ss.stmts) {
    std::cout << s->info << std::endl;
    s->accept(&pw);
    std::cout << std::endl;
  }

  Assign a = Assign(parser.previous, Expr());
  Print p = Print();
  std::cout << "TEST: ";
  a.accept(&pw);
  std::cout << std::endl;
  std::cout << "TEST: ";
  p.accept(&pw);
  std::cout << std::endl;
}

bool parse(const std::string source) {
  Scanner::init(source);
  init();
  advance();
  Stmts ss = statements();
  consume(Scanner::TokenType::SCAN_EOF, "");
  pprint(ss);
  return !parser.hadError;
}

} // namespace Parser
