#include "codegen/visitors/statement.h"
#include "bnfc/Absyn.H"
#include "codegen/visitors/expression.h"
#include "codegen/visitors/type.h"
#include "shared/fail.h"
#include <memory>

namespace codegen {

/**
 * Executes the sequence of statements contained in the block. The statements
 * operate in a fresh scope and share it unless they themselves are block
 * statements.
 */
void StatementVisitor::visitBlock(Block *p) {
  env->getST().pushScope();
  for (auto &stmt : *p->liststmt_) {
    stmt->accept(this);
  }
  env->getST().popScope();
}

/**
 * Does nothing, it is empty.
 */
void StatementVisitor::visitEmpty(Empty *p) {}

/**
 * Executes a block statement. Scope is handled in visitBlock.
 */
void StatementVisitor::visitBStmt(BStmt *p) { p->blk_->accept(this); }

/**
 * Determines the type of one or more declarations which is used when visiting
 * the declarations.
 */
void StatementVisitor::visitDecl(Decl *p) {
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(env);
  env->getST().setCurrentArrType(nullptr);
  p->type_->accept(tv.get());
  type = tv->type;
  for (auto &item : *p->listitem_) {
    item->accept(this);
  }
}

/**
 * Initializes a variable to the symbol table and allocates it with the null
 * value determined by LLVM.
 */
void StatementVisitor::visitNoInit(NoInit *p) {
  if (env->getST().getCurrentArrType() != nullptr) {
    auto arr = env->getBuilder().CreateAlloca(env->createArray(type));
    auto type = env->getST().getCurrentArrType();
    auto size = env->createInt(0);
    auto val = env->allocateArray(type, size);
    env->getBuilder().CreateStore(val, arr);
    env->getST().insert(p->ident_, arr);
    env->getST().insertPointer(p->ident_, arr);
    env->getST().insertArrayType(p->ident_, type);
    return;
  }

  auto alloca = env->getBuilder().CreateAlloca(type);
  auto defaultValue = env->getNullValue(type);
  env->getBuilder().CreateStore(defaultValue, alloca);
  env->getST().insert(p->ident_, defaultValue);
  env->getST().insertPointer(p->ident_, alloca);
}

/**
 * Initializes a variable based on the contained expression before being added
 * to the symbol table.
 */
void StatementVisitor::visitInit(Init *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  if (env->getST().getCurrentArrType() != nullptr) {
    env->getST().insertArrayType(p->ident_, env->getST().getCurrentArrType());
  }
  p->expr_->accept(ev.get());
  llvm::Value *alloca = env->getBuilder().CreateAlloca(type);
  llvm::Value *value = ev->value;
  env->getBuilder().CreateStore(value, alloca);
  env->getST().insert(p->ident_, value);
  env->getST().insertPointer(p->ident_, alloca);
}

/**
 * Assigns a value to an already declared variable. The symbol table is also
 * updated.
 */
void StatementVisitor::visitAss(Ass *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  p->expr_->accept(ev.get());
  llvm::Value *value = ev->value;
  llvm::Value *valuePtr = env->getST().lookupPointer(p->ident_);
  env->getBuilder().CreateStore(value, valuePtr);
  env->getST().insertPointer(p->ident_, valuePtr);
  env->getST().insert(p->ident_, value);
}

/**
 * Increments an integer.
 */
void StatementVisitor::visitIncr(Incr *p) {
  llvm::Value *intPtr = env->getST().lookupPointer(p->ident_);
  llvm::Type *type = env->getST().lookup(p->ident_)->getType();
  llvm::Value *value = env->getBuilder().CreateLoad(type, intPtr);
  llvm::Value *incr = env->getBuilder().CreateAdd(value, env->createInt(1));
  env->getBuilder().CreateStore(incr, intPtr);
}

/**
 * Decrements an integer.
 */
void StatementVisitor::visitDecr(Decr *p) {
  llvm::Value *intPtr = env->getST().lookupPointer(p->ident_);
  llvm::Type *type = env->getST().lookup(p->ident_)->getType();
  llvm::Value *value = env->getBuilder().CreateLoad(type, intPtr);
  llvm::Value *decr = env->getBuilder().CreateSub(value, env->createInt(1));
  env->getBuilder().CreateStore(decr, intPtr);
}

/**
 * Returns the result from the contained expression.
 */
void StatementVisitor::visitRet(Ret *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  p->expr_->accept(ev.get());
  llvm::Value *value = ev->value;
  env->getBuilder().CreateRet(value);
}

/**
 * Returns void.
 */
void StatementVisitor::visitVRet(VRet *p) { env->getBuilder().CreateRetVoid(); }

/**
 * Statements can be expressions followed by a semicolon, simply evaluates the
 * expression.
 */
void StatementVisitor::visitSExp(SExp *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  p->expr_->accept(ev.get());
}

/**
 * Performs an if-statement. It creates two labels: "cont" and "true_body":
 *   - if the contained expression evaluates to true, it jumps to "true_body"
 *     and executes the statement,
 *   - otherwise it continues executing the rest of the program
 */
void StatementVisitor::visitCond(Cond *p) {
  llvm::Function *fn = env->getBuilder().GetInsertBlock()->getParent();
  llvm::BasicBlock *contBlock = llvm::BasicBlock::Create(env->getContext(), "cont", fn);
  llvm::BasicBlock *trueBody = llvm::BasicBlock::Create(env->getContext(), "true_body", fn);
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  p->expr_->accept(ev.get());
  llvm::Value *cond = ev->value;
  env->getBuilder().CreateCondBr(cond, trueBody, contBlock);

  env->getBuilder().SetInsertPoint(trueBody);
  p->stmt_->accept(this);

  if (env->getBuilder().GetInsertBlock()->getTerminator() == nullptr) {
    env->getBuilder().CreateBr(contBlock);
  }

  env->getBuilder().SetInsertPoint(contBlock);
}

/**
 * Performs an if-else-statement. It creates three labels: "cont", "true_body"
 * and "false_body":
 *   - if the contained expression evaluates to true, it jumps to "true_body"
 *     and executes the first statement, and jumps to "cont"
 *   - otherwise it executes the second statement and jumps to "cont"
 */
void StatementVisitor::visitCondElse(CondElse *p) {
  llvm::Function *fn = env->getBuilder().GetInsertBlock()->getParent();
  llvm::BasicBlock *trueBody = llvm::BasicBlock::Create(env->getContext(), "true_body", fn);
  llvm::BasicBlock *elseBody = llvm::BasicBlock::Create(env->getContext(), "false_body", fn);
  llvm::BasicBlock *contBlock = llvm::BasicBlock::Create(env->getContext(), "cont", fn);
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  p->expr_->accept(ev.get());
  llvm::Value *condition = ev->value;
  env->getBuilder().CreateCondBr(condition, trueBody, elseBody);

  env->getBuilder().SetInsertPoint(trueBody);
  p->stmt_1->accept(this);

  if (env->getBuilder().GetInsertBlock()->getTerminator() == nullptr) {
    env->getBuilder().CreateBr(contBlock);
  }

  env->getBuilder().SetInsertPoint(elseBody);
  p->stmt_2->accept(this);
  if (env->getBuilder().GetInsertBlock()->getTerminator() == nullptr) {
    env->getBuilder().CreateBr(contBlock);
  }

  env->getBuilder().SetInsertPoint(contBlock);
}

/**
 * Performs a while-statement. It creates three labels: "while_check",
 * "true_body" and "cont":
 *   - if the expression in the while expression evaluates to true, it jumps to
 * "true_body", otherwise "cont"
 *   - "true_body" executes the statement and jumps back to "while_check" and
 * does so until termination
 */
void StatementVisitor::visitWhile(While *p) {
  llvm::Function *fn = env->getBuilder().GetInsertBlock()->getParent();
  llvm::BasicBlock *checkBlock = llvm::BasicBlock::Create(env->getContext(), "while_check", fn);
  llvm::BasicBlock *bodyBlock = llvm::BasicBlock::Create(env->getContext(), "true_body", fn);
  llvm::BasicBlock *contBlock = llvm::BasicBlock::Create(env->getContext(), "cont", fn);

  env->getBuilder().CreateBr(checkBlock);
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  env->getBuilder().SetInsertPoint(checkBlock);
  p->expr_->accept(ev.get());
  llvm::Value *condition = ev->value;
  env->getBuilder().CreateCondBr(condition, bodyBlock, contBlock);

  env->getBuilder().SetInsertPoint(bodyBlock);
  p->stmt_->accept(this);
  env->getBuilder().CreateBr(checkBlock);

  if (env->getBuilder().GetInsertBlock()->getTerminator() == nullptr) {
    env->getBuilder().CreateBr(contBlock);
  }

  env->getBuilder().SetInsertPoint(contBlock);
}

/**
 * For each. It desugars to a while loop, basically.
 */
void StatementVisitor::visitForEach(ForEach *p) {
  llvm::Function *fn = env->getBuilder().GetInsertBlock()->getParent();
  llvm::BasicBlock *checkBlock = llvm::BasicBlock::Create(env->getContext(), "foreach", fn);
  llvm::BasicBlock *contBlock = llvm::BasicBlock::Create(env->getContext(), "cont", fn);

  llvm::Value *zero = env->createInt(0);
  llvm::Value *iterator = env->getBuilder().CreateAlloca(zero->getType());
  env->getBuilder().CreateStore(zero, iterator);

  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  p->expr_->accept(ev.get());
  llvm::Value *array = ev->value;

  auto length = env->getArrayLength(array);

  env->getBuilder().CreateBr(checkBlock);
  env->getBuilder().SetInsertPoint(checkBlock);
  auto currentIndex = env->getBuilder().CreateLoad(zero->getType(), iterator);
  auto x = env->getArrayElement(array, currentIndex,
                                env->getST().getCurrentArrType()); // int x = arr[i];
  auto xPtr = env->getBuilder().CreateAlloca(env->getST().getCurrentArrType(), 0, p->ident_);
  env->getBuilder().CreateStore(x, xPtr);
  env->getST().insert(p->ident_, x);
  env->getST().insertPointer(p->ident_, xPtr);

  p->stmt_->accept(this);

  // i++
  auto loaded = env->getBuilder().CreateLoad(zero->getType(), iterator);
  auto loadedIncremented = env->getBuilder().CreateAdd(loaded, env->createInt(1));
  env->getBuilder().CreateStore(loadedIncremented, iterator);

  auto cond = env->getBuilder().CreateICmpUGE(loadedIncremented, length);
  env->getBuilder().CreateCondBr(cond, contBlock, checkBlock);

  env->getBuilder().SetInsertPoint(contBlock);
}

/**
 * a[0] = 1;
 */
void StatementVisitor::visitInitArr(InitArr *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  p->expr_1->accept(ev.get());
  llvm::Value *array = ev->value;

  p->expr_2->accept(ev.get());
  llvm::Value *index = ev->value;

  p->expr_3->accept(ev.get());
  llvm::Value *newValue = ev->value;

  env->setArrayElement(array, index, newValue, env->getST().getCurrentArrType());
}

/**
 * a->x = 1;
 * ^^^^   ^---- expr_2
 * |----------- expr_1
 */
void StatementVisitor::visitFieldAss(FieldAss *p) {
  std::unique_ptr<ExpressionVisitor> ev = std::make_unique<ExpressionVisitor>(env);
  auto deref = dynamic_cast<Deref *>(p->expr_1);
  deref->expr_1->accept(ev.get());
  auto str = ev->value;

  if (auto field = dynamic_cast<EVar *>(deref->expr_2)) {
    auto fieldName = field->ident_;
    int fieldIndex = env->getST().getFieldIndex(env->getST().getCurrentStructName(), fieldName);
    p->expr_2->accept(ev.get());
    auto newValue = ev->value;
    env->setStructField(str, fieldIndex, env->getStructByName(env->getST().getCurrentStructName()),
                        newValue);
  } else if (auto arr = dynamic_cast<EArrGet *>(deref->expr_2)) {
    // a->b[0]
    auto name = dynamic_cast<EVar *>(arr->expr_1)->ident_;
    auto fieldIndexToStruct = env->getST().getFieldIndex(env->getST().getCurrentStructName(), name);
    auto strType = env->getStructByName(env->getST().getCurrentStructName());
    arr->expr_2->accept(ev.get());
    auto arrIndex = ev->value;

    p->expr_2->accept(ev.get());
    auto newValue = ev->value;
    auto array = env->getStructField(str, fieldIndexToStruct, strType); // get the array
    env->setArrayElement(array, arrIndex, newValue, env->getST().getCurrentArrType());
  } else {
    fail::panic("Uncreachable");
  }
}

void StatementVisitor::visitListField(ListField *p) {}
void StatementVisitor::visitField(Field *p) {}
} // namespace codegen
