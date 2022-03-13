#ifndef PARSER_H_
#define PARSER_H_

#include "scanner.h"
#include <list>
#include <string>

namespace Parser {

class Opnd;
class Int;
class Bool;
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
  virtual void visitBool(const Bool *b) = 0;
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
class Bool : public Opnd {
public:
  Scanner::Token value;
  Bool(Scanner::Token v) { this->value = v; }
  void accept(TreeWalker *t) override { t->visitBool(this); };
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

bool parse(const std::string source);
void parseAndWalk(const std::string source, TreeWalker *tw);

Stmts *getProgram();

} // namespace Parser

#endif // COMPILER_H_
