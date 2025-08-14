#include "typechecker/visitors/expression.h"
#include "bnfc/Absyn.H"
#include "shared/fail.h"
#include "typechecker/converter.h"
#include "typechecker/visitors/type.h"
#include <memory>

namespace typechecker {
/**
 * Inserts the variable into the symbol table.
 */
void ExpressionVisitor::visitEVar(EVar *p) {
  if (symbolTable->isTypedef(type)) {
    auto structName = symbolTable->lookupTypedefStruct(type);
    auto newType = symbolTable->lookupStructField(structName, p->ident_);
    if (!newType.empty()) {
      type = newType;
      return;
    }
  }
  type = symbolTable->lookup(p->ident_);
  symbolTable->currentTypedef = type;
}

/**
 * Sets the current type to integer.
 */
void ExpressionVisitor::visitELitInt(ELitInt *p) { type = "int"; }

/**
 * Sets the current type to double.
 */
void ExpressionVisitor::visitELitDoub(ELitDoub *p) { type = "double"; }

/**
 * Sets the current type to boolean.
 */
void ExpressionVisitor::visitELitTrue(ELitTrue *p) { type = "boolean"; }

/**
 * Sets the current type to boolean.
 */
void ExpressionVisitor::visitELitFalse(ELitFalse *p) { type = "boolean"; }

/**
 * Checks that the arguments of the function call are correct and sets the type
 to the return type of the function.
 */
void ExpressionVisitor::visitEApp(EApp *p) {
  Func *fun = symbolTable->lookupFunction(p->ident_);
  if (p->listexpr_->size() != fun->arguments.size()) {
    fail::fail("Argument count mismatch in function call");
  }

  for (int i = 0; i < p->listexpr_->size(); i++) {
    auto arg = p->listexpr_->at(i);
    arg->accept(this);
    if (type != fun->arguments[i].second) {
      fail::fail("Argument type mismatch in function call");
    }
  }

  type = fun->returnType;
}

/**
 * Sets the current type to string.
 */
void ExpressionVisitor::visitEString(EString *p) { type = "string"; }

/**
 * Checks that the negation is done on a numerical value.
 */
void ExpressionVisitor::visitNeg(Neg *p) {
  p->expr_->accept(this);
  if (type != "int" && type != "double") {
    fail::fail("Cannot negate non-numerical values");
  }
}

/**
 * Checks that the negation is done on a boolean value.
 */
void ExpressionVisitor::visitNot(Not *p) {
  p->expr_->accept(this);
  if (type != "boolean") {
    fail::fail("Cannot negate non-boolean values");
  }
}

/**
 * Checks that the multiplication is done on numerical values.
 */
void ExpressionVisitor::visitEMul(EMul *p) {
  p->expr_1->accept(this);
  std::string type1 = type;
  p->expr_2->accept(this);
  std::string type2 = type;
  if (type1 != "int" && type1 != "double") {
    fail::fail("Cannot multiply non-numerical values");
  }

  if (type2 != "int" && type2 != "double") {
    fail::fail("Cannot multiply non-numerical values");
  }

  if (type1 != type2) {
    fail::fail("Cannot multiply different types");
  }

  type = type1;
  p->mulop_->accept(this);
}

/**
 * Visits an expression.
 */
void ExpressionVisitor::visitExpr(Expr *p) { p->accept(this); }

/**
 * Checks that the addition is done on numerical values.
 */
void ExpressionVisitor::visitEAdd(EAdd *p) {
  p->expr_1->accept(this);
  std::string type1 = type;
  p->expr_2->accept(this);
  std::string type2 = type;
  if (type1 != "int" && type1 != "double") {
    fail::fail("Cannot add non-numerical values");
  }

  if (type2 != "int" && type2 != "double") {
    fail::fail("Cannot add non-numerical values");
  }

  if (type1 != type2) {
    fail::fail("Cannot add different types");
  }

  type = type1;
}

/**
 * Checks that the relational operator is used on compatible types.
 */
void ExpressionVisitor::visitERel(ERel *p) {
  p->expr_1->accept(this);
  std::string type1 = type;
  p->expr_2->accept(this);
  std::string type2 = type;
  if (!isTypeCompatible(type1, type2)) {
    fail::fail("Cannot compare non-comparable types");
  }

  type = "boolean";
}

/**
 * Checks that the AND operator is used on boolean values.
 */
void ExpressionVisitor::visitEAnd(EAnd *p) {
  p->expr_1->accept(this);
  std::string type1 = type;
  p->expr_2->accept(this);
  std::string type2 = type;
  if (type1 != "boolean") {
    fail::fail("Cannot AND non-boolean values");
  }

  if (type2 != "boolean") {
    fail::fail("Cannot AND non-boolean values");
  }

  type = "boolean";
}

/**
 * Checks that the OR operator is used on boolean values.
 */
void ExpressionVisitor::visitEOr(EOr *p) {
  p->expr_1->accept(this);
  std::string type1 = type;
  if (type1 != "boolean") {
    fail::fail("Cannot OR non-boolean values");
  }

  p->expr_2->accept(this);
  std::string type2 = type;
  if (type2 != "boolean") {
    fail::fail("Cannot OR non-boolean values");
  }

  type = "boolean";
}

/**
 * Sets the current type to the type of the identifier.
 */
void ExpressionVisitor::visitIdent(Ident x) {
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(symbolTable);
  tv->visitIdent(x);
  type = tv->type;
}

/**
 * Checks that the modulo operator is used on integer values.
 */
void ExpressionVisitor::visitMod(Mod *p) {
  if (type != "int") {
    fail::fail("Cannot use modulo on non-integer values");
  }
}

/**
 * a[0]
 */
void ExpressionVisitor::visitEArrGet(EArrGet *p) {
  p->expr_1->accept(this);
  if (!isArrayType(type)) {
    fail::fail("Cannot get element from non-array type");
  }
  std::string elementType = getElementType(type);

  p->expr_2->accept(this);
  if (type != "int") {
    fail::fail("Array index must be an integer");
  }

  type = elementType;
}

/**
 * new int[10]
 */
void ExpressionVisitor::visitEArr(EArr *p) {
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(symbolTable);
  p->type_->accept(tv.get());
  std::string arrType = tv->type;
  p->size_->accept(this);
  if (type != "int") {
    fail::fail("Array size must be an integer");
  }

  type = getArrayType(arrType);
}

void ExpressionVisitor::visitNArr(NArr *p) { p->expr_->accept(this); }

/**
 * a.length
 */
void ExpressionVisitor::visitEArrDot(EArrDot *p) {
  p->expr_->accept(this);
  if (!isArrayType(type)) {
    fail::fail("Cannot get length of non-array type");
  }

  if (p->ident_ != "length") {
    std::string msg = "Unsupported array operation: " + p->ident_;
    fail::fail(msg);
  }

  type = "int";
}

/**
 * prolly unused
 */
void ExpressionVisitor::visitListSize(ListSize *p) {
  for (auto &x : *p) {
    x->accept(this);
  }
}

/**
 * [size]
 */
void ExpressionVisitor::visitSize(Size *p) {}

void ExpressionVisitor::visitNewPtr(NewPtr *p) { type = p->ident_; }

void ExpressionVisitor::visitDeref(Deref *p) {
  p->expr_1->accept(this);
  p->expr_2->accept(this);
}

void ExpressionVisitor::visitNull(Null *p) {
  if (symbolTable->isTypedef(p->ident_)) {
    type = p->ident_;
    symbolTable->currentTypedef = p->ident_;
    return;
  }

  fail::fail("Pointer " + p->ident_ + " is not defined");
}
} // namespace typechecker
