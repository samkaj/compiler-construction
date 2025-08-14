#include "symboltable.h"
#include "shared/fail.h"
#include <iostream>

namespace typechecker {
void SymbolTable::pushScope() { stack.push(std::unordered_map<std::string, std::string>()); }

std::unordered_map<std::string, std::string> SymbolTable::popScope() {
  if (stack.empty()) {
    fail::fail("tried to pop from empty scope");
  }
  std::unordered_map<std::string, std::string> top = stack.top();
  stack.pop();

  return top;
}

/**
 * Inserts the variable into the symbol table. Throws an exception if the
 * variable already exists in the current scope.
 */
void SymbolTable::insert(std::string name, std::string val) {
  if (val == "void") {
    fail::fail("cannot insert void type");
  }

  if (stack.empty()) {
    fail::fail("cannot insert into empty scope");
  }

  if (stack.top().find(name) != stack.top().end()) {
    std::string message = "variable already exists: " + name;
    fail::fail(message.c_str());
  }

  stack.top()[name] = val;
}

/**
 * Updates the value of a variable in the symbol table. Works like insert, but
 * allows for updating values.
 */
void SymbolTable::update(std::string name, std::string val) {
  if (stack.empty()) {
    fail::fail("cannot insert into empty scope");
  }

  stack.top()[name] = val;
}

/**
 * Looks up a variable in the symbol table. Throws an exception if the variable
 * does not exist in any scope. It handles shadowing automatically by looking
 * from the top of the stack.
 */
std::string SymbolTable::lookup(std::string name) {
  if (stack.empty()) {
    fail::fail("tried to lookup without a scope");
  }

  std::stack<std::unordered_map<std::string, std::string>> temp = stack;
  while (!temp.empty()) {
    auto &scope = temp.top();
    auto scopeIterator = scope.find(name);
    if (scopeIterator != scope.end()) {
      return scopeIterator->second;
    }
    temp.pop();
  }

  std::string message = "variable not found: " + name;
  fail::fail(message.c_str());
  return nullptr;
}

bool SymbolTable::valueExists(std::string name) {
  return stack.top().find(name) != stack.top().end();
}

void SymbolTable::insertFunction(const std::string &name, Func *fun) {
  if (functions.find(name) != functions.end()) {
    std::string message = "function already exists: " + name;
    fail::fail(message.c_str());
  }

  functions[name] = fun;
}

Func *SymbolTable::lookupFunction(std::string name) {
  if (functions.find(name) == functions.end()) {
    std::string message = "function not found: " + name;
    fail::fail(message.c_str());
  }

  return functions[name];
}

Func *SymbolTable::lookupCurrentFunction() {
  if (currentFunction.empty()) {
    fail::fail("no current function");
  }

  return lookupFunction(currentFunction);
}

void SymbolTable::setCurrentFunction(std::string name) { currentFunction = name; }

/**
 * Debug function to print the symbol table.
 */
void SymbolTable::print() {
  std::stack<std::unordered_map<std::string, std::string>> temp = stack;
  while (!temp.empty()) {
    auto &scope = temp.top();
    for (auto &pair : scope) {
      std::cout << pair.first << " : " << pair.second << std::endl;
    }
    temp.pop();
  }

  for (auto &pair : functions) {
    std::cout << pair.first << " : " << pair.second->returnType << " (";
    for (auto &arg : pair.second->arguments) {
      std::cout << arg.first << " : " << arg.second << ", ";
    }
    std::cout << ")" << std::endl;
  }
}

/**
 * Resets the return flow variables to their default values.
 */
void SymbolTable::resetReturnFlow() {
  returns = false;
  ifElseReturns = false;
  ifReturns = false;
  elseReturns = false;
}

/**
 * Sadly a bit coupled with the StatementVisitor, but it's the easiest way to
 * check the return flow. After every return, this function is called to update
 * variables tracking the return flow.
 */
void SymbolTable::checkReturnFlow() {
  if (stack.size() == 1) {
    returns = true;
  }

  if (insideBlock) {
    blockReturns = true;
  }

  if (insideIf) {
    ifReturns = true;
  }

  if (insideElse) {
    elseReturns = true;
  }

  if (isInsideIfElse) {
    ifElseReturns = true;
  }
}

void SymbolTable::insertStruct(std::string structName) {
  if (structs.find(structName) != structs.end()) {
    std::string message = "struct already exists: " + structName;
    fail::fail(message.c_str());
  }

  structs[structName] = std::unordered_map<std::string, std::string>();
}

void SymbolTable::insertStructField(std::string structName, std::string fieldName,
                                    std::string type) {
  if (structs.find(structName) == structs.end()) {
    std::string message = "insert: struct not found: " + structName;
    fail::fail(message.c_str());
  }

  if (structs[structName].find(fieldName) != structs[structName].end()) {
    std::string message = "field already exists: " + fieldName;
    fail::fail(message.c_str());
  }

  structs[structName][fieldName] = type;
}

std::string SymbolTable::lookupStructField(std::string structName, std::string fieldName) {
  if (structs.find(structName) == structs.end()) {
    std::string message = "lookup: struct not found: " + structName;
    fail::fail(message.c_str());
  }

  if (structs[structName].find(fieldName) == structs[structName].end()) {
    return "";
  }

  return structs[structName][fieldName];
}

void SymbolTable::insertTypedefStruct(std::string name, std::string structName) {
  if (typedefs.find(name) != typedefs.end()) {
    std::string message = "typedef already exists: " + name;
    fail::fail(message.c_str());
  }

  typedefs[name] = structName;
}

std::string SymbolTable::lookupTypedefStruct(std::string name) {
  if (typedefs.find(name) == typedefs.end()) {
    std::string message = "typedef not found: " + name;
    return "";
  }

  return typedefs[name];
}

void SymbolTable::insertDefinedType(std::string name) { definedTypes.push_back(name); }

bool SymbolTable::isDefinedType(std::string name) {
  for (auto &type : definedTypes) {
    if (type == name) {
      return true;
    }
  }

  for (auto &type : structs) {
    if (type.first == name) {
      return true;
    }
  }

  std::string message = "type not defined: " + name;
  fail::fail(message.c_str());
  return false;
}

bool SymbolTable::isStruct(std::string type) { return structs.find(type) != structs.end(); }

bool SymbolTable::isTypedef(std::string type) { return typedefs.find(type) != typedefs.end(); }

std::string SymbolTable::getCurrentStruct() { return lookupTypedefStruct(currentTypedef); }
} // namespace typechecker
