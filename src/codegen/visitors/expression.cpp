#include "expression.h"
#include "bnfc/Absyn.H"
#include "codegen/converter.h"
#include "codegen/visitors/type.h"
#include "shared/fail.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include <memory>

namespace codegen {

/**
 * Finds the pointer in the symbol table and loads it in
 * LLVM.
 */
void ExpressionVisitor::visitEVar(EVar *p) {
  auto var = env->getST().lookup(p->ident_);
  auto type = var->getType();
  value = env->getBuilder().CreateLoad(type, env->getST().lookupPointer(p->ident_));
  if (env->getST().isArray(p->ident_)) {
    env->getST().setCurrentArrType(env->getST().lookupArrayType(p->ident_));
  }
}

/**
 * Loads a constant integer based on the
 * integer contained in the parameter.
 */
void ExpressionVisitor::visitELitInt(ELitInt *p) { value = env->createInt(p->integer_); }

/**
 * Loads a double based on the double contained in the parameter.
 */
void ExpressionVisitor::visitELitDoub(ELitDoub *p) { value = env->createDouble(p->double_); }

/**
 * Loads a true boolean literal.
 */
void ExpressionVisitor::visitELitTrue(ELitTrue *p) { value = env->createBool(true); }

/**
 * Loads a false boolean literal.
 */
void ExpressionVisitor::visitELitFalse(ELitFalse *p) { value = env->createBool(false); }

/**
 * Makes a function call, i.e., foo(int a, double b);
 * The method first visits all expressions in the parameter list, pushes them to
 * an arg list and makes the call with the evaluated expressions.
 */
void ExpressionVisitor::visitEApp(EApp *p) {
  std::vector<llvm::Value *> args;
  for (auto &expr : *p->listexpr_) {
    expr->accept(this);
    args.push_back(value);
  }
  value = env->getBuilder().CreateCall(env->getModule().getFunction(p->ident_), args);
}

/**
 * Emits a string literal as a global string in LLVM.
 */
void ExpressionVisitor::visitEString(EString *p) {
  value = env->getBuilder().CreateGlobalStringPtr(p->string_);
}

/**
 * Negates the numeric result from the expression contained in
 * the parameter. Emits a Mul or FMul depending on the expression type.
 */
void ExpressionVisitor::visitNeg(Neg *p) {
  p->expr_->accept(this);
  if (value->getType()->isDoubleTy()) {
    value = env->getBuilder().CreateFMul(value, env->createDouble(-1.0));
  } else {
    value = env->getBuilder().CreateMul(value, env->createInt(-1));
  }
}

/**
 * Negates the boolean result from the expression contained in the parameter.
 * Always emits a Not instruction.
 */
void ExpressionVisitor::visitNot(Not *p) {
  p->expr_->accept(this);
  value = llvm::BinaryOperator::CreateNot(value, "", env->getBuilder().GetInsertBlock());
}

/**
 * Performs a multiplicative operation on two numeric expressions. Possible
 * operations are *, /, and %. Does necessary casting to ensure compatible
 * types, i.e., if one of the expressions is of the type double, the other is
 * cast to a double if it not already is.
 *
 * If any of the expressions are pointers, they are first dereferenced.
 */
void ExpressionVisitor::visitEMul(EMul *p) {
  p->expr_1->accept(this);
  llvm::Value *lhs = value;
  p->expr_2->accept(this);
  llvm::Value *rhs = value;

  value = env->createMul(lhs, rhs, convert(p->mulop_));
}

/**
 * Performs an additive operation on two numeric expressions. Possible
 * operations are + and -. Casts integers to doubles if needed. Any pointers are
 * dereferenced.
 */
void ExpressionVisitor::visitEAdd(EAdd *p) {
  p->expr_1->accept(this);
  llvm::Value *lhs = value;
  p->expr_2->accept(this);
  llvm::Value *rhs = value;

  value = env->createAdd(lhs, rhs, convert(p->addop_));
}

/**
 * Compares two numeric expressions.
 */
void ExpressionVisitor::visitERel(ERel *p) {
  p->expr_1->accept(this);
  llvm::Value *lhs = value;
  p->expr_2->accept(this);
  llvm::Value *rhs = value;

  value = env->createRelational(lhs, rhs, convert(p->relop_));
}

/**
 * Performs a logical AND between two boolean expressions. It is lazily
 * evaluated, i.e., if the first expression is false, the second one is ignored.
 */
void ExpressionVisitor::visitEAnd(EAnd *p) {
  llvm::Function *fn = env->getBuilder().GetInsertBlock()->getParent();
  llvm::BasicBlock *lazyBlock = llvm::BasicBlock::Create(env->getContext(), "lazy", fn);
  llvm::BasicBlock *contBlock = llvm::BasicBlock::Create(env->getContext(), "cont", fn);
  llvm::Value *result = env->getBuilder().CreateAlloca(llvm::Type::getInt1Ty(env->getContext()));

  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  p->expr_1->accept(ev.get());
  llvm::Value *lhs = ev->value;
  env->getBuilder().CreateStore(lhs, result);
  env->getBuilder().CreateCondBr(lhs, lazyBlock, contBlock);

  env->getBuilder().SetInsertPoint(lazyBlock);
  p->expr_2->accept(ev.get());
  llvm::Value *rhs = ev->value;
  env->getBuilder().CreateStore(rhs, result);
  env->getBuilder().CreateBr(contBlock);

  env->getBuilder().SetInsertPoint(contBlock);
  value = env->getBuilder().CreateLoad(llvm::Type::getInt1Ty(env->getContext()), result);
}

/**
 * Performs a logical OR between two boolean expressions. It is lazily
 * evaluated, i.e., if the first expression is true , the second one is ignored.
 */
void ExpressionVisitor::visitEOr(EOr *p) {
  llvm::Function *fn = env->getBuilder().GetInsertBlock()->getParent();
  llvm::BasicBlock *lazyBlock = llvm::BasicBlock::Create(env->getContext(), "lazy", fn);
  llvm::BasicBlock *contBlock = llvm::BasicBlock::Create(env->getContext(), "cont", fn);
  llvm::Value *result = env->getBuilder().CreateAlloca(llvm::Type::getInt1Ty(env->getContext()));

  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  p->expr_1->accept(ev.get());
  llvm::Value *lhs = ev->value;
  env->getBuilder().CreateStore(lhs, result);
  env->getBuilder().CreateCondBr(lhs, contBlock, lazyBlock);

  env->getBuilder().SetInsertPoint(lazyBlock);
  p->expr_2->accept(ev.get());
  llvm::Value *rhs = ev->value;
  env->getBuilder().CreateStore(rhs, result);
  env->getBuilder().CreateBr(contBlock);

  env->getBuilder().SetInsertPoint(contBlock);
  value = env->getBuilder().CreateLoad(llvm::Type::getInt1Ty(env->getContext()), result);
}

/**
 * a[0]
 */
void ExpressionVisitor::visitEArrGet(EArrGet *p) {
  p->expr_1->accept(this);
  llvm::Value *array = value;

  p->expr_2->accept(this);
  llvm::Value *index = value;

  value = env->getArrayElement(array, index, env->getST().getCurrentArrType());
}

/**
 * new int[10];
 * using calloc, we allocate the memory for the array and return the pointer.
 */
void ExpressionVisitor::visitEArr(EArr *p) {
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(env);
  p->type_->accept(tv.get());
  auto type = tv->type;
  env->getST().setCurrentArrType(type);
  p->size_->accept(this);
  auto size = value;

  value = env->allocateArray(type, size);
}

void ExpressionVisitor::visitNArr(NArr *p) { p->expr_->accept(this); }

/**
 * a.length
 */
void ExpressionVisitor::visitEArrDot(EArrDot *p) {
  p->expr_->accept(this);
  auto lenPtr = env->getArrayLength(value);
  auto alloca = env->getBuilder().CreateAlloca(env->createInt(1)->getType());
  env->getBuilder().CreateStore(lenPtr, alloca);
  value = env->getBuilder().CreateLoad(env->createInt(1)->getType(), alloca);
}

void ExpressionVisitor::visitListSize(ListSize *p) {
  for (auto &x : *p) {
    x->accept(this);
  }
}
void ExpressionVisitor::visitSize(Size *p) {}

/**
 * new A
 */
void ExpressionVisitor::visitNewPtr(NewPtr *p) {
  env->getST().setLastStructName(p->ident_);
  auto type = env->getStructByName(p->ident_);
  value = env->allocatePointer(type);
}

/**
 * a->b
 */
void ExpressionVisitor::visitDeref(Deref *p) {
  p->expr_1->accept(this);
  auto str = value;
  if (auto field = dynamic_cast<EVar *>(p->expr_2)) {
    auto fieldName = field->ident_;

    int fieldIndex = env->getST().getFieldIndex(env->getST().getCurrentStructName(), fieldName);
    value = env->getStructField(str, fieldIndex,
                                env->getStructByName(env->getST().getCurrentStructName()));
  } else if (auto arr = dynamic_cast<EArrGet *>(p->expr_2)) {
    // a->b[0]
    auto name = dynamic_cast<EVar *>(arr->expr_1)->ident_;
    auto fieldIndex = env->getST().getFieldIndex(env->getST().getCurrentStructName(), name);
    auto strType = env->getStructByName(env->getST().getCurrentStructName());
    auto array = env->getStructField(str, fieldIndex, strType);

    arr->expr_2->accept(this);
    auto arrIndex = value;

    value = env->getArrayElement(array, arrIndex, env->getST().getCurrentArrType());
  } else {
    fail::fail("Uncreachable");
  }
}

/**
 * struct A { int x; };
 * typedef struct A *Aptr;
 *
 * Aptr a;
 * (a)null;
 */
void ExpressionVisitor::visitNull(Null *p) {
  auto type = env->getStructFromTypedef(p->ident_);
  value = llvm::ConstantPointerNull::get(type->getPointerTo());
}
} // namespace codegen
