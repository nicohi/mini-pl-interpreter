#include "interpreter.h"
#include "compiler.h"
#include "parser.h"
#include "scanner.h"
#include <iostream>
#include <map>
#include <stack>

namespace Interpreter {

std::map<std::string, int> intMap;
std::map<std::string, std::string> stringMap;
std::stack<std::string> intStack;
std::stack<std::string> stringStack;

void setVar(Scanner::TokenType type) {
  //
  //
  //
}
class InterpretWalker : public Parser::TreeWalker {
public:
  void visitOpnd(const Parser::Opnd *i) override {}
  void visitInt(const Parser::Int *i) override {}
  void visitString(const Parser::String *s) override {}
  void visitIdent(const Parser::Ident *i) override {}
  void visitExpr(const Parser::Expr *e) override {}
  void visitBinary(const Parser::Binary *b) override {}
  void visitUnary(const Parser::Unary *u) override {}
  void visitSingle(const Parser::Single *s) override {}
  void visitStmt(const Parser::Stmt *s) override {}
  void visitStmts(const Parser::Stmts *s) override {
    for (Parser::TreeNode *n : s->stmts) {
      n->accept(this);
    }
  }
  void visitVar(const Parser::Var *v) override {}
  void visitAssign(const Parser::Assign *a) override {}
  void visitFor(const Parser::For *f) override {}
  void visitRead(const Parser::Read *r) override {}
  void visitPrint(const Parser::Print *p) override {}
  void visitAssert(const Parser::Assert *a) override {}
};

InterpretResult interpret(const std::string source) {
  try {
    Compiler::runScanner(source);
    Compiler::runParser(source);
    InterpretWalker *iw = new InterpretWalker();
    Parser::getProgram()->accept(iw);
  } catch (int e) {
    return InterpretResult::COMPILE_ERROR;
  }
  return InterpretResult::OK;
}

} // namespace Interpreter
