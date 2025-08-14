#include "symboltable.h"
#include "shared/fail.h"
#include <string>

namespace codegen {
void SymbolTable::pushScope() {
  valueStack.push(std::unordered_map<std::string, llvm::Value *>());
  pointerStack.push(std::unordered_map<std::string, llvm::Value *>());
  arrayTypes.push(std::unordered_map<std::string, llvm::Type *>());
}

std::unordered_map<std::string, llvm::Value *> SymbolTable::popScope() {
  if (valueStack.empty()) {
    fail::panic("tried to pop from empty scope");
  }
  std::unordered_map<std::string, llvm::Value *> top = valueStack.top();
  valueStack.pop();

  if (pointerStack.empty()) {
    fail::panic("tried to pop from empty scope");
  }
  pointerStack.pop();

  if (arrayTypes.empty()) {
    fail::panic("tried to pop from empty scope");
  }
  arrayTypes.pop();
  return top;
}

void SymbolTable::insert(std::string name, llvm::Value *val) {
  if (valueStack.empty()) {
    fail::panic("cannot insert into empty scope");
  }

  valueStack.top()[name] = val;
}

llvm::Value *SymbolTable::lookup(std::string name) {
  if (valueStack.empty()) {
    fail::panic("tried to lookup without a scope");
  }

  std::stack<std::unordered_map<std::string, llvm::Value *>> temp = valueStack;
  while (!temp.empty()) {
    auto &scope = temp.top();
    auto scopeIterator = scope.find(name);
    if (scopeIterator != scope.end()) {
      return scopeIterator->second;
    }
    temp.pop();
  }

  std::string message = "variable not found: " + name;
  fail::panic(message.c_str());
  return nullptr;
}

bool SymbolTable::valueExists(std::string name) {
  return valueStack.top().find(name) != valueStack.top().end();
}

void SymbolTable::insertFunction(const std::string &name, llvm::Function *fun) {
  functions[name] = fun;
}

llvm::Function *SymbolTable::lookupFunction(std::string name) { return functions[name]; }

void SymbolTable::insertPointer(std::string name, llvm::Value *val) {
  if (pointerStack.empty()) {
    fail::panic("cannot insert pointer into empty scope");
  }

  pointerStack.top()[name] = val;
}

llvm::Value *SymbolTable::lookupPointer(std::string name) {
  if (pointerStack.empty()) {
    fail::panic("tried to lookup without a scope");
  }

  std::stack<std::unordered_map<std::string, llvm::Value *>> temp = pointerStack;
  while (!temp.empty()) {
    auto &scope = temp.top();
    auto scopeIterator = scope.find(name);
    if (scopeIterator != scope.end()) {
      return scopeIterator->second;
    }
    temp.pop();
  }

  std::string message = "pointer not found: " + name;
  fail::panic(message.c_str());
  return nullptr;
}

void SymbolTable::insertArrayType(std::string array, llvm::Type *type) {
  arrayTypes.top()[array] = type;
}

bool SymbolTable::isArray(std::string name) {
  if (arrayTypes.empty()) {
    fail::panic("tried to lookup array type without a scope");
  }

  std::stack<std::unordered_map<std::string, llvm::Type *>> temp = arrayTypes;
  while (!temp.empty()) {
    auto &scope = temp.top();
    auto scopeIterator = scope.find(name);
    if (scopeIterator != scope.end()) {
      return true;
    }
    temp.pop();
  }

  return false;
}

llvm::Type *SymbolTable::lookupArrayType(std::string array) {
  if (arrayTypes.empty()) {
    fail::panic("tried to lookup array type without a scope");
  }

  std::stack<std::unordered_map<std::string, llvm::Type *>> temp = arrayTypes;
  while (!temp.empty()) {
    auto &scope = temp.top();
    auto scopeIterator = scope.find(array);
    if (scopeIterator != scope.end()) {
      return scopeIterator->second;
    }
    temp.pop();
  }

  std::string message = "array type not found: " + array;
  fail::panic(message.c_str());
  return nullptr;
}

void SymbolTable::insertTypedef(std::string typedefName, std::string structName) {
  typedefs[typedefName] = structName;
}

void SymbolTable::insertStruct(std::string structName,
                               std::unordered_map<std::string, int> fieldIndices) {
  structs[structName] = fieldIndices;
}

int SymbolTable::getFieldIndex(std::string structName, std::string fieldName) {
  return structs[structName][fieldName];
}

std::string SymbolTable::getStructNameFromTypedef(std::string typedefName) {
  return typedefs[typedefName];
}

std::string SymbolTable::getCurrentStructName() { return lastStructName; }
void SymbolTable::setLastStructName(std::string name) { lastStructName = name; }
void SymbolTable::setCurrentArrType(llvm::Type *type) { arrType = type; }
llvm::Type *SymbolTable::getCurrentArrType() { return arrType; }
} // namespace codegen
