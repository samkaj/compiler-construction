#include "typechecker/visitors/statement.h"
#include "bnfc/Absyn.H"
#include "shared/fail.h"
#include "typechecker/converter.h"
#include "typechecker/visitors/expression.h"
#include "typechecker/visitors/type.h"
#include <memory>

namespace typechecker {
/**
 * Typechecks the statements in a block. A block is a list of statements.
 */
void StatementVisitor::visitBlock(Block *p) {
  for (auto &stmt : *p->liststmt_) {
    stmt->accept(this);
  }
}

/**
 * Does nothing.
 */
void StatementVisitor::visitEmpty(Empty *p) {
  // Do nothing
}

/**
 * Visits a block statement. A block statement is a block surrounded by curly
 * braces.
 */
void StatementVisitor::visitBStmt(BStmt *p) {
  symbolTable->pushScope();
  symbolTable->insideBlock = true;
  p->blk_->accept(this);
  symbolTable->insideBlock = false;
  symbolTable->popScope();
}

/**
 * Declares a variable, either with or without initialization.
 */
void StatementVisitor::visitDecl(Decl *p) {
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(symbolTable);
  p->type_->accept(tv.get());
  type = tv->type;
  symbolTable->currentTypedef = type;
  for (auto &item : *p->listitem_) {
    item->accept(this);
  }
}

/**
 * Declares a variable without initialization, it is added to the symbol table.
 */
void StatementVisitor::visitNoInit(NoInit *p) {
  if (!symbolTable->isDefinedType(type)) {
    fail::fail("Type " + type + " is not defined");
  }

  symbolTable->currentTypedef = type;

  symbolTable->insert(p->ident_, type);
}

/**
 * Declares a variable with initialization, it is added to the symbol table. If
 * it is already declared, an exception is thrown. It also fails if the expected
 * type does not match the received one.
 */
void StatementVisitor::visitInit(Init *p) {
  if (symbolTable->valueExists(p->ident_)) {
    fail::fail("Variable already declared");
  }

  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);
  p->expr_->accept(ev.get());
  std::string exprType = ev->type;
  if (exprType != type) {
    if (symbolTable->lookupTypedefStruct(type) != exprType) {
      fail::fail("Type mismatch in variable initialization, got " + exprType + " expected " + type);
    }
  }

  symbolTable->update(p->ident_, type);
}

/**
 * Visits an assignment statement. It fails if the expected type does not match
 * the received one.
 */
void StatementVisitor::visitAss(Ass *p) {
  std::string symbolType = symbolTable->lookup(p->ident_);
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);
  p->expr_->accept(ev.get());
  if (ev->type == symbolType) {
    return;
  }

  if (symbolTable->lookupTypedefStruct(symbolType) == ev->type) {
    type = ev->type;
    return;
  }

  fail::fail("Type mismatch in variable assignment, got " + symbolType + " expected " + ev->type);
}

/**
 * Visits an increment. This can only be done on integers, and fails if this
 * does not hold.
 */
void StatementVisitor::visitIncr(Incr *p) {
  std::string symbolType = symbolTable->lookup(p->ident_);
  if (symbolType != "int") {
    fail::fail("Cannot increment non-integer values");
  }
  type = symbolType;
}

/**
 * Visits a decrement. This can only be done on integers, and fails if this
 * does not hold.
 */
void StatementVisitor::visitDecr(Decr *p) {
  std::string symbolType = symbolTable->lookup(p->ident_);
  if (symbolType != "int") {
    fail::fail("Cannot decrement non-integer values");
  }
  type = symbolType;
}

/**
 * Ensures that return statements match the function signature.
 */
void StatementVisitor::visitRet(Ret *p) {
  Func *fun = symbolTable->lookupCurrentFunction();
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(symbolTable);
  p->accept(tv.get());
  std::string exprType = tv->type;
  if (exprType != fun->returnType) {
    if (symbolTable->lookupTypedefStruct(fun->returnType) == exprType) {
      type = exprType;
    } else {
      fail::fail("Return type does not match function signature");
    }
  }

  if (exprType == "void") {
    fail::fail("Cannot return void from a non-void function");
  }

  type = "void";
  symbolTable->checkReturnFlow();
}

void StatementVisitor::visitVRet(VRet *p) {
  Func *fun = symbolTable->lookupCurrentFunction();
  if (fun->returnType != "void") {
    fail::fail("Non-void return type does not match function signature");
  }

  type = "void";
  symbolTable->checkReturnFlow();
}

/**
 * If statements are typechecked by checking that the condition is of type bool.
 */
void StatementVisitor::visitCond(Cond *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);
  p->expr_->accept(ev.get());
  std::string exprType = ev->type;
  type = exprType;
  if (exprType != "boolean") {
    fail::fail("Condition must be of type bool");
  }

  symbolTable.get()->insideIf = true;
  symbolTable.get()->pushScope();
  p->stmt_->accept(this);
  symbolTable.get()->popScope();
}

/**
 * If-else statements are typechecked by checking that the condition is of type
 * bool.
 */
void StatementVisitor::visitCondElse(CondElse *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);
  p->expr_->accept(ev.get());
  std::string exprType = ev->type;
  type = exprType;
  if (exprType != "boolean") {
    fail::fail("Condition must be of type bool");
  }

  symbolTable.get()->insideIf = true;
  symbolTable.get()->isInsideIfElse = true;
  symbolTable.get()->pushScope();
  p->stmt_1->accept(this);
  symbolTable.get()->popScope();
  symbolTable.get()->insideIf = false;
  symbolTable.get()->insideElse = true;
  symbolTable.get()->pushScope();
  p->stmt_2->accept(this);
  symbolTable.get()->popScope();
  symbolTable.get()->insideElse = false;
  symbolTable.get()->isInsideIfElse = false;
}

/**
 * While statements are typechecked by checking that the condition is of type
 bool.
 */
void StatementVisitor::visitWhile(While *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);
  p->expr_->accept(ev.get());
  std::string exprType = ev->type;
  type = exprType;
  if (exprType != "boolean") {
    fail::fail("Condition must be of type boolean, got " + exprType);
  }

  symbolTable.get()->pushScope();
  symbolTable.get()->isInsideLoop = true;
  p->stmt_->accept(this);
  symbolTable.get()->popScope();
}

/**
 * Statements which are only expressions followed by a semicolon are typechecked
 * here. If these are not of type void, an exception is thrown.
 */
void StatementVisitor::visitSExp(SExp *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);
  p->expr_->accept(ev.get());
  type = ev->type;
  if (ev->type != "void") {
    fail::fail("Expression must be of type void got " + ev->type);
  }
}

/**
 * foo[0] = 1;
 */
void StatementVisitor::visitInitArr(InitArr *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);

  p->expr_1->accept(ev.get());
  std::string symbolType = ev->type;
  if (!isArrayType(symbolType)) {
    fail::fail("Cannot assign to non-array type");
  }

  p->expr_2->accept(ev.get());
  if (ev->type != "int") {
    fail::fail("Array index must be an integer");
  }

  p->expr_3->accept(ev.get());
  if (getElementType(symbolType) != ev->type) {
    auto type = symbolTable->lookupTypedefStruct(getElementType(symbolType));
    if (type != ev->type) {
      fail::fail("Type mismatch in array assignment, got " + ev->type + " expected " + type);
    }
  }
}

/**
 * int[] foo;
 * for (int i : foo) {}
 */
void StatementVisitor::visitForEach(ForEach *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);
  p->expr_->accept(ev.get());
  std::string symbolType = ev->type;
  if (!isArrayType(symbolType)) {
    fail::fail("Cannot iterate over non-array type");
  }

  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(symbolTable);
  p->type_->accept(tv.get());
  if (tv->type != getElementType(symbolType)) {
    fail::fail("Type mismatch in for-each loop");
  }

  symbolTable->pushScope();
  symbolTable->insert(p->ident_, getElementType(symbolType));
  p->stmt_->accept(this);
  symbolTable->popScope();
}

void StatementVisitor::visitFieldAss(FieldAss *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);
  p->expr_1->accept(ev.get());
  std::string symbolType = ev->type;
  p->expr_2->accept(ev.get());
  if (ev->type != symbolType) {
    if (symbolTable->lookupTypedefStruct(symbolType) != ev->type) {
      fail::fail("Type mismatch in field assignment " + ev->type + " " + symbolType);
    }
  }
}

void StatementVisitor::visitListField(ListField *p) {}

void StatementVisitor::visitField(Field *p) {}
} // namespace typechecker
