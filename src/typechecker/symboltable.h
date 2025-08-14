#pragma once

#include "bnfc/Absyn.H"
#include "typechecker/converter.h"
#include <stack>
#include <string>
#include <unordered_map>
namespace typechecker {

class SymbolTable {
  std::unordered_map<std::string, Func *> functions;
  std::stack<std::unordered_map<std::string, std::string>> stack;
  std::string currentFunction;
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>> structs;
  std::unordered_map<std::string, std::string> typedefs;
  std::vector<std::string> definedTypes;

public:
  std::string currentTypedef;
  std::string currentStruct;
  bool returns;
  bool ifElseReturns;
  bool ifReturns;
  bool elseReturns;
  bool whileReturns;
  bool blockReturns;
  bool isInsideLoop;
  bool isInsideIfElse;
  bool insideIf;
  bool insideElse;
  bool insideBlock;

  SymbolTable()
      : stack(std::stack<std::unordered_map<std::string, std::string>>()),
        functions(std::unordered_map<std::string, Func *>()), currentFunction(""),
        currentTypedef(""), currentStruct(""), returns(false), ifElseReturns(false),
        ifReturns(false), elseReturns(false), whileReturns(false), isInsideLoop(false),
        isInsideIfElse(false), insideIf(false), insideElse(false), blockReturns(false),
        insideBlock(false),
        structs(std::unordered_map<std::string, std::unordered_map<std::string, std::string>>()),
        typedefs(std::unordered_map<std::string, std::string>()),
        definedTypes(std::vector<std::string>()) {}
  void pushScope();
  std::unordered_map<std::string, std::string> popScope();
  void insert(std::string name, std::string val);
  void update(std::string name, std::string val);
  std::string lookup(std::string name);
  bool valueExists(std::string name);
  void insertFunction(const std::string &name, Func *fun);
  Func *lookupFunction(std::string name);
  Func *lookupCurrentFunction();
  void print();
  void setCurrentFunction(std::string name);
  void resetReturnFlow();
  void checkReturnFlow();

  void insertStruct(std::string structName);
  void insertStructField(std::string structName, std::string fieldName, std::string type);
  std::string lookupStructField(std::string structName, std::string fieldName);
  void insertTypedefStruct(std::string name, std::string structName);
  std::string lookupTypedefStruct(std::string name);
  void insertDefinedType(std::string name);
  bool isDefinedType(std::string name);
  bool isStruct(std::string name);
  bool isTypedef(std::string name);
  std::string getCurrentStruct();
};
} // namespace typechecker
