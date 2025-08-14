#include "program.h"
#include "bnfc/Absyn.H"
#include "codegen/visitors/statement.h"
#include "codegen/visitors/type.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include <memory>

namespace codegen {
/**
 * Programs are the highest level components. The compilation is done in five
 * phases:
 * 1. Opaque struct phase: All structs are defined as opaque.
 * 2. Typedef phase: Links the pointers to their structs.
 * 3. Set struct body phase: Populates the struct bodies.
 * 4. Definition phase: Identifies the functions in the Javalette file.
 * 5. Compilation phase: Generates LLVM IR based on the functions contained in
 * the program.
 */
void ProgramVisitor::visitProgram(Program *p) {
  for (auto &def : *p->listtopdef_) {
    if (auto *structDef = dynamic_cast<StructDef *>(def)) {
      opaqueStructPhase(structDef);
    }
  }

  for (auto &def : *p->listtopdef_) {
    if (auto *typeDef = dynamic_cast<Typedef *>(def)) {
      typedefPhase(typeDef);
    }
  }

  for (auto &def : *p->listtopdef_) {
    if (auto *structDef = dynamic_cast<StructDef *>(def)) {
      setStructBodyPhase(structDef);
    }
  }

  for (auto &def : *p->listtopdef_) {
    if (auto *fnDef = dynamic_cast<FnDef *>(def)) {
      definitionPhase(fnDef);
    }
  }

  for (auto &def : *p->listtopdef_) {
    if (auto *fnDef = dynamic_cast<FnDef *>(def)) {
      compilationPhase(fnDef);
    }
  }
}

/**
 * Puts all structs in context. Since they may contain typedefs which are not
 * yet defined, they are opaque.
 */
void ProgramVisitor::opaqueStructPhase(StructDef *p) { env->defineStruct(p->ident_); }

/**
 * Links the pointers to their structs.
 */
void ProgramVisitor::typedefPhase(Typedef *p) {
  env->getST().insertTypedef(p->ident_2, p->ident_1);
}

/**
 * With typedefs in context, the struct bodies are populated.
 */
void ProgramVisitor::setStructBodyPhase(StructDef *p) {
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(env);
  std::vector<llvm::Type *> types{};
  std::unordered_map<std::string, int> indices;
  int i = 0;
  for (auto field : *p->listfield_) {
    auto member = dynamic_cast<Member *>(field);
    member->type_->accept(tv.get());
    if (tv->type->isStructTy()) {
      tv->type = tv->type->getPointerTo();
    }
    types.push_back(tv->type);
    indices[member->ident_] = i;
    i++;
  }
  env->setStructBody(p->ident_, types);
  env->getST().insertStruct(p->ident_, indices);
}

/**
 * The definition phase identifies the functions in the Javalette file. Its main
 * purpose is to populate the symbol table to allow functions to be defined in
 * any order.
 */
void ProgramVisitor::definitionPhase(FnDef *p) {
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(env);
  p->type_->accept(tv.get());
  llvm::Type *retType = tv.get()->type;
  if (retType->isStructTy()) {
    retType = retType->getPointerTo();
  }

  std::vector<llvm::Type *> args;
  for (auto &arg : *p->listarg_) {
    arg->accept(tv.get());
    if (tv->type->isStructTy()) {
      tv->type = tv->type->getPointerTo();
    }
    args.push_back(tv.get()->type);
  }

  llvm::FunctionType *funType = llvm::FunctionType::get(retType, args, false);
  llvm::Function *fun = llvm::Function::Create(funType, llvm::Function::ExternalLinkage, p->ident_,
                                               env.get()->getModule());
  env.get()->getST().insertFunction(p->ident_, fun);
}

/**
 * The compilation phase generates LLVM IR based on the functions contained in
 * the program. Since every program is a sequence of statements, it uses a
 * statement visitor to do most of the heavy lifting, and is therefore quite
 * abstract.
 */
void ProgramVisitor::compilationPhase(FnDef *p) {
  env.get()->getST().pushScope();
  llvm::Function *fun = env.get()->getST().lookupFunction(p->ident_);
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(env.get()->getContext(), "", fun);
  env.get()->getBuilder().SetInsertPoint(entry);
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(env);

  for (int i = 0; i < p->listarg_->size(); i++) {
    auto *arg = static_cast<Argument *>(p->listarg_->at(i));
    llvm::Argument *val = fun->getArg(i);

    arg->type_->accept(tv.get());
    auto type = tv->type;
    if (type->isStructTy() && type->getStructName() == "struct.array") {
      env->getST().insertArrayType(arg->ident_, env->getST().getCurrentArrType());
    }

    llvm::Value *ptr = env.get()->getBuilder().CreateAlloca(type, nullptr, arg->ident_);
    env.get()->getBuilder().CreateStore(val, ptr);
    env.get()->getST().insertPointer(arg->ident_, ptr);
    env.get()->getST().insert(arg->ident_, val);
  }

  std::unique_ptr<StatementVisitor> sv = std::make_unique<StatementVisitor>(env);
  p->blk_->accept(sv.get());

  if (env.get()->getBuilder().GetInsertBlock()->getTerminator() == nullptr) {
    if (fun->getReturnType()->isVoidTy()) {
      env.get()->getBuilder().CreateRetVoid();
    } else {
      env.get()->getBuilder().CreateUnreachable();
    }
  }

  args.clear();
  argNames.clear();
}

/**
 * Used for ensuring that the second compilation phase has access to the
 * arguments defined in the first phase.
 */
void ProgramVisitor::visitArgument(Argument *p) {
  std::unique_ptr<TypeVisitor> tv = std::make_unique<TypeVisitor>(env);
  p->type_->accept(tv.get());
  args.push_back(tv->type);
  argNames.push_back(p->ident_);
}

void ProgramVisitor::visitTopDef(TopDef *p) {}
void ProgramVisitor::visitStructDef(StructDef *p) {}
void ProgramVisitor::visitTypedef(Typedef *p) {}

} // namespace codegen
