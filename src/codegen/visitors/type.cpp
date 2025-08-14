#include "type.h"

namespace codegen {
void TypeVisitor::visitInteger(Integer x) {}
void TypeVisitor::visitChar(Char x) {}
void TypeVisitor::visitDouble(Double x) {}
void TypeVisitor::visitIdent(Ident x) {}
void TypeVisitor::visitInt(Int *p) {
  type = llvm::Type::getInt32Ty(this->env->getContext());
  env->getST().setCurrentArrType(nullptr);
}
void TypeVisitor::visitDoub(Doub *p) {
  type = llvm::Type::getDoubleTy(env->getContext());
  env->getST().setCurrentArrType(nullptr);
}
void TypeVisitor::visitBool(Bool *p) {
  type = llvm::Type::getInt1Ty(env->getContext());
  env->getST().setCurrentArrType(nullptr);
}
void TypeVisitor::visitVoid(Void *p) {
  type = llvm::Type::getVoidTy(env->getContext());
  env->getST().setCurrentArrType(nullptr);
}
void TypeVisitor::visitArgument(Argument *p) { p->type_->accept(this); }
void TypeVisitor::visitArr(Arr *p) {
  p->type_->accept(this);
  env->getST().setCurrentArrType(type);
  type = env->createArray(type); // FIXME: rename?
}
void TypeVisitor::visitStruct(Struct *p) {
  auto typedefName = p->ident_;
  type = env->getStructFromTypedef(typedefName)->getPointerTo();
  env->getST().setLastStructName(
      env->getST().getStructNameFromTypedef(typedefName));
}

} // namespace codegen
