#include "program.h"
#include "shared/fail.h"
#include "typechecker/converter.h"
#include "typechecker/visitors/statement.h"
#include "typechecker/visitors/type.h"
#include <memory>

namespace typechecker {
/**
 * Main entry of the typechecker. It is done in two phases, first it finds
 function signatures, then it typechecks the function bodies. This is done to
 ensure that functions can be defined in any order.
 */
void ProgramVisitor::visitProgram(Program *p) {
  for (auto &def : *p->listtopdef_) {
    if (auto *typeDef = dynamic_cast<Typedef *>(def)) {
      typeDef->accept(this);
    }
  }
  typedefsDone = true;

  for (auto &def : *p->listtopdef_) {
    if (auto *structDef = dynamic_cast<StructDef *>(def)) {
      structDef->accept(this);
    }
  }
  structsDone = true;

  for (auto &def : *p->listtopdef_) {
    def->accept(this);
  }

  definitionsDone = true;
  for (auto &def : *p->listtopdef_) {
    def->accept(this);
  }
}

/**
 * Used mainly by visitProgram. It executes the two phases explicitly.
 */
void ProgramVisitor::visitFnDef(FnDef *p) {
  symbolTable.get()->setCurrentFunction(p->ident_);
  if (!definitionsDone) {
    defintionPhase(p);
    return;
  }

  typecheckingPhase(p);
}

/**
 * Visits a function definition, used in visitProgram mainly.
 */
void ProgramVisitor::visitTopDef(TopDef *p) { p->accept(this); }

/**
 * The definition phase identifies the functions in the Javalette file. Its main
 * purpose is to populate the symbol table to allow functions to be defined in
 * any order.
 */
void ProgramVisitor::defintionPhase(FnDef *p) {
  std::unique_ptr<TypeVisitor> typeVisitor = std::make_unique<TypeVisitor>(symbolTable);

  symbolTable.get()->setCurrentFunction(p->ident_);

  std::vector<std::pair<std::string, std::string>> arguments;
  for (auto arg : *p->listarg_) {
    Argument *argument = static_cast<Argument *>(arg);
    argument->type_->accept(typeVisitor.get());
    arguments.push_back({argument->ident_, typeVisitor->type});
  }
  p->type_->accept(typeVisitor.get());

  symbolTable.get()->insertFunction(p->ident_, new Func(p->ident_, typeVisitor->type, arguments));
}

/**
 * The typechecking phase checks the function body for type errors.
 */
void ProgramVisitor::typecheckingPhase(FnDef *p) {
  symbolTable.get()->setCurrentFunction(p->ident_);
  symbolTable.get()->pushScope();
  std::unique_ptr<TypeVisitor> typeVisitor = std::make_unique<TypeVisitor>(symbolTable);
  for (int i = 0; i < p->listarg_->size(); i++) {
    auto *arg = static_cast<Argument *>(p->listarg_->at(i));
    arg->type_->accept(typeVisitor.get());
    auto type = typeVisitor->type;
    symbolTable->insert(arg->ident_, type);
  }

  std::unique_ptr<StatementVisitor> statementVisitor =
      std::make_unique<StatementVisitor>(symbolTable);
  p->blk_->accept(statementVisitor.get());
  p->type_->accept(typeVisitor.get());
  std::string returnType = typeVisitor->type;

  if (symbolTable.get()->ifElseReturns) {
    if ((!symbolTable.get()->ifReturns || !symbolTable.get()->elseReturns) &&
        !symbolTable.get()->returns) {
      fail::fail("If-else statement must return in both branches");
    }
  } else if (!symbolTable.get()->returns) {
    if (returnType != "void") {
      bool crash = false;
      if (!symbolTable.get()->blockReturns)
        crash = true;
      if (symbolTable.get()->isInsideLoop)
        crash = true;
      if (crash)
        fail::fail("Function " + symbolTable->lookupCurrentFunction()->name +
                   " must return a value");
    }
  }

  symbolTable.get()->popScope();
  symbolTable.get()->resetReturnFlow();
}

/**
 * Adds an argument to the current function scope.
 */
void ProgramVisitor::visitArgument(Argument *p) {
  std::unique_ptr<TypeVisitor> typeVisitor = std::make_unique<TypeVisitor>(symbolTable);
  p->type_->accept(typeVisitor.get());
  auto type = typeVisitor->type;
  if (!symbolTable->isDefinedType(type)) {
    type = symbolTable->lookupTypedefStruct(p->ident_);
  }
  symbolTable->update(p->ident_, type);
}

void ProgramVisitor::visitStructDef(StructDef *p) {
  if (structsDone) {
    return;
  }

  symbolTable->insertStruct(p->ident_);
  std::unique_ptr<TypeVisitor> typeVisitor = std::make_unique<TypeVisitor>(symbolTable);
  for (auto &field : *p->listfield_) {
    Member *member = static_cast<Member *>(field);
    member->type_->accept(typeVisitor.get());
    symbolTable->insertStructField(p->ident_, member->ident_, typeVisitor->type);
  }
}
void ProgramVisitor::visitTypedef(Typedef *p) {
  if (typedefsDone) {
    return;
  }

  symbolTable->insertTypedefStruct(p->ident_2, p->ident_1);
  symbolTable->insertDefinedType(p->ident_2);
}
} // namespace typechecker
