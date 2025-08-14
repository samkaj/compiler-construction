#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include <stack>
#include <string>
#include <unordered_map>
namespace codegen {
class SymbolTable {
  std::stack<std::unordered_map<std::string, llvm::Value *>> valueStack;
  std::stack<std::unordered_map<std::string, llvm::Value *>> pointerStack;
  std::stack<std::unordered_map<std::string, llvm::Type *>> arrayTypes;
  std::unordered_map<std::string, llvm::Function *> functions;
  std::unordered_map<std::string, std::unordered_map<std::string, int>> structs;
  std::unordered_map<std::string, std::string> typedefs;
  std::string lastStructName;
  llvm::Type *arrType;

public:
  SymbolTable()
      : valueStack(std::stack<std::unordered_map<std::string, llvm::Value *>>()),
        pointerStack(std::stack<std::unordered_map<std::string, llvm::Value *>>()),
        arrayTypes(std::stack<std::unordered_map<std::string, llvm::Type *>>()), functions(),
        structs(std::unordered_map<std::string, std::unordered_map<std::string, int>>()),
        typedefs(std::unordered_map<std::string, std::string>()), lastStructName(""),
        arrType(nullptr) {}
  void pushScope();
  std::unordered_map<std::string, llvm::Value *> popScope();
  void insert(std::string name, llvm::Value *val);
  llvm::Value *lookup(std::string name);
  void insertPointer(std::string name, llvm::Value *val);
  llvm::Value *lookupPointer(std::string name);
  bool valueExists(std::string name);
  void insertFunction(const std::string &name, llvm::Function *fun);
  llvm::Function *lookupFunction(std::string name);
  void insertArrayType(std::string name, llvm::Type *type);
  bool isArray(std::string name);
  llvm::Type *lookupArrayType(std::string name);
  void insertTypedef(std::string typedefName, std::string structName);
  void insertStruct(std::string structName, std::unordered_map<std::string, int> fieldIndices);
  int getFieldIndex(std::string structName, std::string fieldName);
  std::string getStructNameFromTypedef(std::string typedefName);
  std::string getCurrentStructName();
  void setLastStructName(std::string name);
  void setCurrentArrType(llvm::Type *type);
  llvm::Type *getCurrentArrType();
};
} // namespace codegen
