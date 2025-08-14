#include "typechecker/visitors/type.h"
#include "bnfc/Absyn.H"
#include "typechecker/visitors/expression.h"
#include <memory>
namespace typechecker {
void TypeVisitor::visitInteger(Integer x) { type = "int"; }
void TypeVisitor::visitChar(Char x) {}
void TypeVisitor::visitDouble(Double x) { type = "double"; }
void TypeVisitor::visitIdent(Ident x) { type = symbolTable->lookup(x); }
void TypeVisitor::visitInt(Int *p) { type = "int"; }
void TypeVisitor::visitDoub(Doub *p) { type = "double"; }
void TypeVisitor::visitBool(Bool *p) { type = "boolean"; }
void TypeVisitor::visitVoid(Void *p) { type = "void"; }
void TypeVisitor::visitRet(Ret *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(symbolTable);
  p->expr_->accept(ev.get());
  type = ev.get()->type;
}
void TypeVisitor::visitVRet(VRet *p) { type = "void"; }
void TypeVisitor::visitArr(Arr *p) {
  p->type_->accept(this);
  type = type + "[]";
}
void TypeVisitor::visitStruct(Struct *p) {
  type = p->ident_;
  symbolTable->currentTypedef = type;
}
} // namespace typechecker
