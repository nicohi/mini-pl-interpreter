#include "interpreter.h"
#include "compiler.h"
#include "parser.h"
#include "scanner.h"
#include <functional>
#include <iostream>
#include <map>
#include <stack>
#include <string>

namespace Interpreter {

void error(std::string msg) {
  std::cerr << msg << std::endl;
  throw;
}

class Variable {
public:
  Scanner::TokenType type;
  bool constant;
  int int_value;
  bool bool_value;
  std::string string_value;
  void update(int i) {
    if (constant)
      error("Tried to write to constant variable");
    int_value = i;
    string_value = std::to_string(i);
  }
  void update(bool b) {
    if (constant)
      error("Tried to write to constant variable");
    bool_value = b;
    string_value = b ? "true" : "false";
  }
  void update(std::string s) {
    if (constant)
      error("Tried to write to constant variable");
    if (type == Scanner::TokenType::INT)
      update(std::stoi(s));
    if (type == Scanner::TokenType::BOOL) {
      update(s[0] == 't');
    } else
      string_value = s;
  }
  void set(Scanner::TokenType t, std::string s) {
    constant = false;
    if (t == Scanner::TokenType::INT || t == Scanner::TokenType::INTEGER_LIT) {
      type = Scanner::TokenType::INT;
      this->update(std::stoi(s));
    } else if (t == Scanner::TokenType::BOOL ||
               t == Scanner::TokenType::BOOLEAN_LIT) {
      type = Scanner::TokenType::BOOL;
      this->update(s[0] == 't');
    } else if (t == Scanner::TokenType::STRING) {
      type = Scanner::TokenType::STRING;
      this->update(s);
    } else if (t == Scanner::TokenType::STRING_LIT) {
      // strip ""
      type = Scanner::TokenType::STRING;
      this->update(s.substr(1, s.length() - 2));
    } else
      error("Can not set variable of type:" + Scanner::getName(t));
  }
  void set(Scanner::TokenType t) {
    constant = false;
    type = t;
    if (t == Scanner::TokenType::INT || t == Scanner::TokenType::INTEGER_LIT) {
      this->update(0);
    } else if (t == Scanner::TokenType::BOOL ||
               t == Scanner::TokenType::BOOLEAN_LIT) {
      this->update(false);
    } else if (t == Scanner::TokenType::STRING ||
               t == Scanner::TokenType::STRING_LIT) {
      this->update("");
    } else
      error("Can not set variable of type:" + Scanner::getName(t));
  }
  int getInt() { return int_value; }
  bool getBool() { return bool_value; }
  std::string getString() { return string_value; }
};

std::map<std::string, Variable *> varMap;
std::stack<Variable *> varStack;

std::string toStr(Scanner::Token t) {
  std::string s = "";
  for (int i = 0; i < t.length; i++)
    s = s + t.start[i];
  return s;
}

std::map<std::string, std::function<Variable *(Variable *, Variable *)>>
    opMap{};
void init() {
  opMap.emplace("+", [](Variable *l, Variable *r) {
    Variable *n = new Variable();
    n->set(l->type);
    if (l->type == Scanner::TokenType::INT)
      n->update(l->getInt() + r->getInt());
    else
      n->update(l->getString() + r->getString());
    return n;
  });
  opMap.emplace("-", [](Variable *l, Variable *r) {
    Variable *n = new Variable();
    n->set(l->type);
    n->update(l->getInt() - r->getInt());
    return n;
  });
  opMap.emplace("*", [](Variable *l, Variable *r) {
    Variable *n = new Variable();
    n->set(l->type);
    n->update(l->getInt() * r->getInt());
    return n;
  });
  opMap.emplace("/", [](Variable *l, Variable *r) {
    Variable *n = new Variable();
    n->set(l->type);
    n->update(l->getInt() / r->getInt());
    return n;
  });
  opMap.emplace("&", [](Variable *l, Variable *r) {
    Variable *n = new Variable();
    n->set(l->type);
    n->update(l->getBool() && r->getBool());
    return n;
  });
  opMap.emplace("=", [](Variable *l, Variable *r) {
    Variable *n = new Variable();
    n->set(l->type);
    if (l->type == Scanner::TokenType::INTEGER_LIT)
      n->update(l->getInt() == r->getInt());
    else if (l->type == Scanner::TokenType::BOOLEAN_LIT)
      n->update(l->getBool() == r->getBool());
    else
      n->update(l->getString() == r->getString());
    return n;
  });
  opMap.emplace("<", [](Variable *l, Variable *r) {
    Variable *n = new Variable();
    n->set(l->type);
    if (l->type == Scanner::TokenType::INTEGER_LIT)
      n->update(l->getInt() < r->getInt());
    else if (l->type == Scanner::TokenType::BOOLEAN_LIT)
      n->update(l->getBool() < r->getBool());
    else
      n->update(l->getString() < r->getString());
    return n;
  });
  // Ignores left
  opMap.emplace("!", [](Variable *l, Variable *r) {
    Variable *n = new Variable();
    n->set(r->type);
    if (r->type == Scanner::TokenType::INTEGER_LIT)
      n->update(!r->getInt());
    else if (l->type == Scanner::TokenType::BOOLEAN_LIT)
      n->update(!r->getBool());
    return n;
  });
}

void printStack_(std::stack<Variable *> &st) {
  if (st.empty())
    return;
  Variable *x = st.top();
  std::cout << "\tINT:" << x->getInt() << " STR:" << x->getString() << "\n";
  st.pop();
  printStack_(st);
  st.push(x);
}

void printStack() {
  std::cout << "========================\n";
  std::cout << "Expr stack:\n";
  printStack_(varStack);
  std::cout << "========================\n";
}

void printVarMap() {
  std::cout << "========================\n";
  std::cout << "Variable map:\n";
  for (auto const &[id, var] : varMap) {
    std::cout << "\t"
              << "id:" << id << " val:" << var->getString() << std::endl;
  }
  std::cout << "========================\n";
}

void printDiag() {
  printVarMap();
  printStack();
}

std::string unEscape(std::string s) {
  std::string o = "";
  bool next = false;
  for (char c : s) {
    if (next) {
      next = false;
      switch (c) {
      case 'n': {
        c = '\n';
        break;
      }
      case 't': {
        c = '\t';
        break;
      }
      case '\\': {
        c = '\\';
        break;
      }
      case '"': {
        c = '\"';
        break;
      }
      default: {
      }
      }
    }
    if (c == '\\') {
      next = true;
      continue;
    }
    o = o + c;
  }
  return o;
}

class InterpretWalker : public Parser::TreeWalker {
public:
  void visitOpnd(const Parser::Opnd *i) override { error("NOT IMPLEMENTED"); }
  void visitInt(const Parser::Int *i) override {
    Variable *v = new Variable();
    v->set(Scanner::TokenType::INTEGER_LIT, toStr(i->value));
    varStack.push(v);
  }
  void visitBool(const Parser::Bool *b) override {
    Variable *v = new Variable();
    v->set(Scanner::TokenType::BOOLEAN_LIT, toStr(b->value));
    varStack.push(v);
  }
  void visitString(const Parser::String *s) override {
    Variable *v = new Variable();
    v->set(Scanner::TokenType::STRING_LIT, toStr(s->value));
    varStack.push(v);
  }
  void visitIdent(const Parser::Ident *i) override {
    std::string id = toStr(i->ident);
    if (!varMap.count(id))
      error("Variable '" + id + "' has not been initialized");
    varStack.push(varMap[id]);
  }
  void visitExpr(const Parser::Expr *e) override { error("NOT IMPLEMENTED"); }
  void visitBinary(const Parser::Binary *b) override {
    b->left->accept(this);
    Variable *l = varStack.top();
    varStack.pop();
    b->right->accept(this);
    Variable *r = varStack.top();
    varStack.pop();
    varStack.push(opMap[toStr(b->op)](l, r));
    // std::cout << "BINARY l:" << l->getInt() << " op:" << toStr(b->op)
    //           << " r:" << r->getInt() << " res:" <<
    //           varStack.top()->getInt()
    //           << std::endl;
    // printStack(varStack);
  }
  void visitUnary(const Parser::Unary *u) override {
    u->right->accept(this);
    Variable *r = varStack.top();
    varStack.pop();
    r = opMap[toStr(u->op)](r, r);
    varStack.push(r);
  }
  void visitSingle(const Parser::Single *s) override { s->right->accept(this); }
  void visitStmt(const Parser::Stmt *s) override {}
  void visitStmts(const Parser::Stmts *s) override {
    for (Parser::TreeNode *n : s->stmts) {
      n->accept(this);
    }
  }
  void visitVar(const Parser::Var *v) override {
    std::string id = toStr(v->ident);
    if (varMap.count(id))
      error("Variable '" + id + "' already initialized");
    if (v->expr) {
      v->expr->accept(this);
      varMap[id] = varStack.top();
      varStack.pop();
      // if (varMap[id]->type != v->type.type) error("Could not set variable
      // '"
      // + id + "' to expression of type " + Scanner::getName(v->type));
    } else {
      Variable *var = new Variable();
      var->set(v->type.type);
      varMap[id] = var;
    }
  }
  void visitAssign(const Parser::Assign *a) override {
    std::string id = toStr(a->ident);
    if (!varMap.count(id))
      error("Variable '" + id + "' has not been initialized");
    a->expr->accept(this);
    varMap[id] = varStack.top();
    varStack.pop();
  }
  void visitFor(const Parser::For *f) override {
    std::string id = toStr(f->ident);
    if (!varMap.count(id))
      error("Variable '" + id + "' has not been initialized");
    Variable *control = varMap[id];
    f->from->accept(this);
    f->to->accept(this);
    int to = varStack.top()->getInt();
    varStack.pop();
    int from = varStack.top()->getInt();
    varStack.pop();
    control->update(from);
    while (from <= to) {
      control->constant = true;
      f->body->accept(this);
      control->constant = false;
      from++;
      control->update(from);
    }
    // error("NOT IMPLEMENTED");
  }
  void visitRead(const Parser::Read *r) override {
    std::string id = toStr(r->ident);
    if (!varMap.count(id))
      error("Variable '" + id + "' has not been initialized");
    std::string s;
    std::cin >> s;

    varMap[id]->update(s);
  }
  void visitPrint(const Parser::Print *p) override {
    p->expr->accept(this);
    // std::cout << "<printing>";
    //   printStack(varStack);
    std::cout << unEscape(varStack.top()->getString());
    varStack.pop();
  }
  void visitAssert(const Parser::Assert *a) override {
    a->expr->accept(this);
    if (!varStack.top()->getBool())
      printDiag();
    varStack.pop();
  }
};

InterpretResult interpret(const std::string source) {
  try {
    // Compiler::runScanner(source);
    // Compiler::runParser(source);
    init();
    InterpretWalker *iw = new InterpretWalker();
    // iw->visitPrint(new Parser::Print());
    Parser::parseAndWalk(source, iw);
  } catch (int e) {
    return InterpretResult::COMPILE_ERROR;
  }
  return InterpretResult::OK;
}

} // namespace Interpreter
