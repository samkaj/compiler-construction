#include "environment.h"
#include "codegen/converter.h"
#include "shared/fail.h"
#include "llvm/IR/DerivedTypes.h"

namespace codegen {
llvm::IRBuilder<> &Environment::getBuilder() { return *builder; }
llvm::Module &Environment::getModule() { return *module; }
llvm::LLVMContext &Environment::getContext() { return *context; }
SymbolTable &Environment::getST() { return *symbolTable; }

void Environment::defineStruct(std::string name) { llvm::StructType::create(getContext(), name); }

void Environment::setStructBody(std::string name, std::vector<llvm::Type *> fields) {
  auto type = llvm::StructType::getTypeByName(getContext(), name);
  type->setBody(fields);
}

llvm::Type *Environment::getStructFromTypedef(std::string typedefName) {
  auto structName = getST().getStructNameFromTypedef(typedefName);
  return getStructByName(structName);
}

llvm::Type *Environment::getStructByName(std::string structName) {
  return llvm::StructType::getTypeByName(getContext(), structName);
}

llvm::Value *Environment::getStructField(llvm::Value *str, int index, llvm::Type *structType) {
  auto gep = builder->CreateStructGEP(structType, str, index);
  return builder->CreateLoad(structType->getStructElementType(index), gep);
}

void Environment::setStructField(llvm::Value *str, int index, llvm::Type *structType,
                                 llvm::Value *newValue) {
  auto gep = builder->CreateStructGEP(structType, str, index);
  builder->CreateStore(newValue, gep);
}

llvm::StructType *Environment::getArrayType(llvm::Type *type) { return nullptr; }

/**
 * Create a new array. It is a wrapper around the calloc function with a length
 * field
 */
llvm::Value *Environment::allocateArray(llvm::Type *type, llvm::Value *size) {
  // Allocate some memory for the array
  auto calloc = getST().lookupFunction("calloc");
  auto sizeOfType = createInt64(8); // FIXME? kinda hacky. but since the array is a pointer
                                    // and a i32, it should be 8 bytes right?
  auto size64 = builder->CreateSExt(size, llvm::Type::getInt64Ty(*context));
  auto ptrToArray = builder->CreateCall(calloc, {size64, sizeOfType}, "arr_ptr");

  auto arrStructType = createArray(type);

  // 16 bytes. 8 for the length, 8 for the pointer
  auto ptr = builder->CreateCall(calloc, {createInt64(1), createInt64(16)}, "arr_struct_ptr");

  auto length = builder->CreateStructGEP(arrStructType, ptr, 0);
  builder->CreateStore(size, length);

  auto arr = builder->CreateStructGEP(arrStructType, ptr, 1);
  builder->CreateStore(ptrToArray, arr);

  return ptr;
}

llvm::Value *Environment::allocatePointer(llvm::Type *structType) {
  auto calloc = getST().lookupFunction("calloc");
  auto dataLayout = getModule().getDataLayout();
  auto sizeOfStruct = dataLayout.getTypeAllocSize(structType);
  auto sizeOfType = createInt64(sizeOfStruct);
  auto size64 = createInt64(1);
  auto ptrToArray = builder->CreateCall(calloc, {size64, sizeOfType}, "ptr");
  for (int i = 0; i < structType->getStructNumElements(); i++) {
    auto gep = builder->CreateStructGEP(structType, ptrToArray, i);
    builder->CreateStore(getNullValue(structType->getStructElementType(i)), gep);
  }
  return ptrToArray;
}

llvm::Type *Environment::createArray(llvm::Type *type) {
  auto ty = llvm::StructType::getTypeByName(getContext(), "struct.array");
  if (!ty) {
    std::vector<llvm::Type *> fields = {llvm::Type::getInt32Ty(getContext()),
                                        llvm::Type::getInt8PtrTy(getContext())};
    ty = llvm::StructType::create(getContext(), fields, "struct.array");
  }
  return ty;
}

llvm::Value *Environment::getInt64(llvm::Value *int32) {
  return builder->CreateSExt(int32, llvm::Type::getInt64Ty(getContext()));
}

llvm::Value *Environment::getNullValue(llvm::Type *type) {
  if (type->isPointerTy()) {
    return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(type));
  }

  return llvm::Constant::getNullValue(type);
}

llvm::Type *Environment::getInt64Ty() { return llvm::Type::getInt64Ty(getContext()); }

/**
 * Get the length of an array
 */
llvm::Value *Environment::getArrayLength(llvm::Value *array) {
  auto zero = createInt(0);
  auto ty = zero->getType();
  auto arrPtr = builder->CreateGEP(createArray(ty), array, {zero, zero}, "arr_len");
  auto len = builder->CreateLoad(ty, arrPtr);
  return len;
}

/**
 * Get an element from an array
 */
llvm::Value *Environment::getArrayElement(llvm::Value *array, llvm::Value *index,
                                          llvm::Type *type) {
  auto gep = builder->CreateStructGEP(createArray(type), array, 1, "set_arr_ptr");
  auto arrPtr = builder->CreateLoad(gep->getType(), gep);
  auto element = builder->CreateGEP(type->getPointerTo(), arrPtr, {index}, "gep_arr_get");
  return builder->CreateLoad(type, element);
}

/**
 * Set an element in an array
 */
void Environment::setArrayElement(llvm::Value *array, llvm::Value *index, llvm::Value *value,
                                  llvm::Type *type) {
  auto gep = builder->CreateStructGEP(createArray(type), array, 1, "set_arr_ptr");
  auto arrPtr = builder->CreateLoad(gep->getType(), gep);
  auto element = builder->CreateGEP(type->getPointerTo(), arrPtr, {index}, "gep_arr_set");
  builder->CreateStore(value, element);
}

llvm::Value *Environment::createInt64(int value) {
  auto *val = llvm::ConstantInt::get(*context, llvm::APInt(64, value));
  return val;
}

llvm::Value *Environment::createInt(int value) {
  auto *val = llvm::ConstantInt::get(*context, llvm::APInt(32, value));
  return val;
}
llvm::Value *Environment::createDouble(double value) {
  auto *val = llvm::ConstantFP::get(*context, llvm::APFloat(value));
  return val;
}
llvm::Value *Environment::createBool(bool value) {
  auto *val = llvm::ConstantInt::get(*context, llvm::APInt(1, value));
  return val;
}
llvm::Value *Environment::createString(const std::string &value) {
  auto *val = builder->CreateGlobalStringPtr(value);
  return val;
}

llvm::Value *Environment::createMul(llvm::Value *lhs, llvm::Value *rhs, MultiplicationOp op) {
  if (lhs->getType()->isIntegerTy() && rhs->getType()->isDoubleTy()) {
    lhs = builder->CreateSIToFP(lhs, rhs->getType());
  } else if (lhs->getType()->isDoubleTy() && rhs->getType()->isIntegerTy()) {
    rhs = builder->CreateSIToFP(rhs, lhs->getType());
  }

  if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    if (op == MultiplicationOp::Multiplication) {
      return builder->CreateMul(lhs, rhs);
    } else if (op == MultiplicationOp::Division) {
      return builder->CreateSDiv(lhs, rhs);
    } else if (op == MultiplicationOp::Modulo) {
      return builder->CreateSRem(lhs, rhs);
    }
  } else if (lhs->getType()->isDoubleTy() && rhs->getType()->isDoubleTy()) {
    if (op == MultiplicationOp::Multiplication) {
      return builder->CreateFMul(lhs, rhs);
    } else if (op == MultiplicationOp::Division) {
      return builder->CreateFDiv(lhs, rhs);
    }
  }

  fail::panic("Invalid types for multiplication");
  return nullptr;
}
llvm::Value *Environment::createAdd(llvm::Value *lhs, llvm::Value *rhs, AdditiveOp op) {
  // if one is double and the other is int, convert int to double
  if (lhs->getType()->isIntegerTy() && rhs->getType()->isDoubleTy()) {
    lhs = builder->CreateSIToFP(lhs, rhs->getType());
  } else if (lhs->getType()->isDoubleTy() && rhs->getType()->isIntegerTy()) {
    rhs = builder->CreateSIToFP(rhs, lhs->getType());
  }

  if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    if (op == AdditiveOp::Addition) {
      return builder->CreateAdd(lhs, rhs);
    } else if (op == AdditiveOp::Subtraction) {
      return builder->CreateSub(lhs, rhs);
    }
  } else if (lhs->getType()->isDoubleTy() && rhs->getType()->isDoubleTy()) {
    if (op == AdditiveOp::Addition) {
      return builder->CreateFAdd(lhs, rhs);
    } else if (op == AdditiveOp::Subtraction) {
      return builder->CreateFSub(lhs, rhs);
    }
  }

  fail::panic("Invalid types for addition");
  return nullptr;
}
llvm::Value *Environment::createRelational(llvm::Value *lhs, llvm::Value *rhs, RelationalOp op) {
  // if one is double and the other is int, convert int to double
  if (lhs->getType()->isIntegerTy() && rhs->getType()->isDoubleTy()) {
    lhs = builder->CreateSIToFP(lhs, rhs->getType());
  } else if (lhs->getType()->isDoubleTy() && rhs->getType()->isIntegerTy()) {
    rhs = builder->CreateSIToFP(rhs, lhs->getType());
  }

  if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    if (op == RelationalOp::Equal) {
      return builder->CreateICmpEQ(lhs, rhs);
    } else if (op == RelationalOp::NotEqual) {
      return builder->CreateICmpNE(lhs, rhs);
    } else if (op == RelationalOp::LessThan) {
      return builder->CreateICmpSLT(lhs, rhs);
    } else if (op == RelationalOp::LessThanOrEqual) {
      return builder->CreateICmpSLE(lhs, rhs);
    } else if (op == RelationalOp::GreaterThan) {
      return builder->CreateICmpSGT(lhs, rhs);
    } else if (op == RelationalOp::GreaterThanOrEqual) {
      return builder->CreateICmpSGE(lhs, rhs);
    }
  } else if (lhs->getType()->isDoubleTy() && rhs->getType()->isDoubleTy() ||
             lhs->getType()->isFloatTy() && rhs->getType()->isFloatTy()) {
    if (op == RelationalOp::Equal) {
      return builder->CreateFCmpOEQ(lhs, rhs);
    } else if (op == RelationalOp::NotEqual) {
      return builder->CreateFCmpONE(lhs, rhs);
    } else if (op == RelationalOp::LessThan) {
      return builder->CreateFCmpOLT(lhs, rhs);
    } else if (op == RelationalOp::LessThanOrEqual) {
      return builder->CreateFCmpOLE(lhs, rhs);
    } else if (op == RelationalOp::GreaterThan) {
      return builder->CreateFCmpOGT(lhs, rhs);
    } else if (op == RelationalOp::GreaterThanOrEqual) {
      return builder->CreateFCmpOGE(lhs, rhs);
    }
  }

  if (lhs->getType()->isPointerTy() && rhs->getType()->isPointerTy()) {
    if (op == RelationalOp::Equal) {
      return builder->CreateICmpEQ(lhs, rhs);
    } else if (op == RelationalOp::NotEqual) {
      return builder->CreateICmpNE(lhs, rhs);
    }
  }

  fail::panic("Invalid types for relational operation");
  return nullptr;
}

void Environment::prelude() {
  std::vector<llvm::Type *> printIntArgs(1, llvm::Type::getInt32Ty(*context));
  llvm::FunctionType *printIntType =
      llvm::FunctionType::get(llvm::Type::getVoidTy(*context), printIntArgs, false);
  llvm::Function *printIntFn = llvm::Function::Create(printIntType, llvm::Function::ExternalLinkage,
                                                      "printInt", module.get());
  getST().insertFunction("printInt", printIntFn);

  // int printDouble(double x);
  std::vector<llvm::Type *> printDoubleArgs(1, llvm::Type::getDoubleTy(*context));
  llvm::FunctionType *printDoubleType =
      llvm::FunctionType::get(llvm::Type::getVoidTy(*context), printDoubleArgs, false);
  llvm::Function *printDoubleFn = llvm::Function::Create(
      printDoubleType, llvm::Function::ExternalLinkage, "printDouble", module.get());
  getST().insertFunction("printDouble", printDoubleFn);

  // int printString(char *x);
  std::vector<llvm::Type *> printStringArgs(1, llvm::Type::getInt8PtrTy(*context));
  llvm::FunctionType *printStringType =
      llvm::FunctionType::get(llvm::Type::getVoidTy(*context), printStringArgs, false);
  llvm::Function *printStringFn = llvm::Function::Create(
      printStringType, llvm::Function::ExternalLinkage, "printString", module.get());
  getST().insertFunction("printString", printStringFn);

  // int readInt();
  std::vector<llvm::Type *> readIntArgs;
  llvm::FunctionType *readIntType =
      llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), readIntArgs, false);
  llvm::Function *readIntFn =
      llvm::Function::Create(readIntType, llvm::Function::ExternalLinkage, "readInt", module.get());
  getST().insertFunction("readInt", readIntFn);

  // double readDouble();
  std::vector<llvm::Type *> readDoubleArgs;
  llvm::FunctionType *readDoubleType =
      llvm::FunctionType::get(llvm::Type::getDoubleTy(*context), readDoubleArgs, false);
  llvm::Function *readDoubleFn = llvm::Function::Create(
      readDoubleType, llvm::Function::ExternalLinkage, "readDouble", module.get());
  getST().insertFunction("readDouble", readDoubleFn);

  // calloc
  std::vector<llvm::Type *> callocArgs(2);
  callocArgs[0] = llvm::Type::getInt64Ty(*context);
  callocArgs[1] = llvm::Type::getInt64Ty(*context);
  llvm::FunctionType *callocType =
      llvm::FunctionType::get(llvm::Type::getInt8PtrTy(*context), callocArgs, false);
  llvm::Function *callocFn =
      llvm::Function::Create(callocType, llvm::Function::ExternalLinkage, "calloc", module.get());
  getST().insertFunction("calloc", callocFn);
}
} // namespace codegen
