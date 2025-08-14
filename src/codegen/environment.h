#pragma once

#include "codegen/converter.h"
#include "codegen/symboltable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <memory>
namespace codegen {
class Environment {
private:
  std::unique_ptr<llvm::LLVMContext> context;
  std::unique_ptr<llvm::IRBuilder<>> builder;
  std::unique_ptr<llvm::Module> module;
  std::unique_ptr<SymbolTable> symbolTable;
  void prelude();

public:
  Environment()
      : context(std::make_unique<llvm::LLVMContext>()),
        builder(std::make_unique<llvm::IRBuilder<>>(*context)),
        module(std::make_unique<llvm::Module>("main", *context)),
        symbolTable(std::make_unique<SymbolTable>()) {
    prelude();
  }
  ~Environment() = default;
  llvm::IRBuilder<> &getBuilder();
  llvm::Module &getModule();
  llvm::LLVMContext &getContext();
  SymbolTable &getST();
  llvm::StructType *getArrayType(llvm::Type *type);
  llvm::Value *allocateArray(llvm::Type *type, llvm::Value *size);
  llvm::Type *createArray(llvm::Type *type);
  llvm::Value *getArrayLength(llvm::Value *array);
  llvm::Value *getArrayElement(llvm::Value *array, llvm::Value *index, llvm::Type *type);
  void setArrayElement(llvm::Value *array, llvm::Value *index, llvm::Value *value,
                       llvm::Type *type);
  llvm::Value *getNullValue(llvm::Type *type);
  llvm::Value *createInt(int value);
  llvm::Value *createDouble(double value);
  llvm::Value *createBool(bool value);
  llvm::Value *createString(const std::string &value);
  llvm::Value *createMul(llvm::Value *lhs, llvm::Value *rhs, MultiplicationOp op);
  llvm::Value *createAdd(llvm::Value *lhs, llvm::Value *rhs, AdditiveOp op);
  llvm::Value *createRelational(llvm::Value *lhs, llvm::Value *rhs, RelationalOp op);
  llvm::Value *getInt64(llvm::Value *int32);
  llvm::Type *getInt64Ty();
  llvm::Value *createInt64(int value);

  void defineStruct(std::string name);
  void setStructBody(std::string name, std::vector<llvm::Type *> fields);
  llvm::Type *getStructFromTypedef(std::string typedefName);
  llvm::Type *getStructByName(std::string structName);
  llvm::Value *getStructField(llvm::Value *str, int index, llvm::Type *structType);
  void setStructField(llvm::Value *str, int index, llvm::Type *structType, llvm::Value *newValue);
  int getFieldIndex(std::string structName, std::string fieldName);
  llvm::Value *allocatePointer(llvm::Type *structType);
};
} // namespace codegen
