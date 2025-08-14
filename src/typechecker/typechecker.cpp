#include "typechecker.h"
#include "typechecker/converter.h"
#include "visitors/program.h"
#include <memory>

namespace typechecker {
void typecheck(Prog *ast) {
  std::shared_ptr<SymbolTable> symbolTable = std::make_shared<SymbolTable>();
  Func *printInt = new Func("printInt", "void", {{"x", "int"}});
  symbolTable->insertFunction("printInt", printInt);

  Func *printDouble = new Func("printDouble", "void", {{"x", "double"}});
  symbolTable->insertFunction("printDouble", printDouble);

  Func *printString = new Func("printString", "void", {{"x", "string"}});
  symbolTable->insertFunction("printString", printString);

  Func *readInt = new Func("readInt", "int", {});
  symbolTable->insertFunction("readInt", readInt);

  Func *readDouble = new Func("readDouble", "double", {});
  symbolTable->insertFunction("readDouble", readDouble);

  std::vector<std::string> primitiveTypes = {"int",   "double",   "string",   "boolean",
                                             "int[]", "double[]", "string[]", "boolean[]"};
  for (auto type : primitiveTypes) {
    symbolTable->insertDefinedType(type);
  }

  std::unique_ptr<ProgramVisitor> visitor = std::make_unique<ProgramVisitor>(symbolTable, ast);
  ast->accept(visitor.get());
}
} // namespace typechecker
