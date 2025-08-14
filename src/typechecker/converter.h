#pragma once

#include "bnfc/Absyn.H"

namespace typechecker {
enum RelationalOp {  // relational operators
  Equal,             // ==
  NotEqual,          // !=
  LessThan,          // <
  LessThanOrEqual,   // <=
  GreaterThan,       // >
  GreaterThanOrEqual // >=
};
enum MultiplicationOp { // multiplicative operators
  Multiplication,       // *
  Division,             // /
  Modulo                // %
};
enum AdditiveOp { // additive operators
  Addition,       // +
  Subtraction     // -
};

class Func {
public:
  std::string name;
  std::string returnType;
  std::vector<std::pair<std::string, std::string>> arguments;
  Func(std::string name, std::string returnType,
       std::vector<std::pair<std::string, std::string>> arguments)
      : name(name), returnType(returnType), arguments(arguments) {}
};

MultiplicationOp convert(MulOp *mulOp);
RelationalOp convert(RelOp *relOp);
AdditiveOp convert(AddOp *addOp);
bool isTypeCompatible(std::string type1, std::string type2);
std::string getArrayType(std::string type);
bool isArrayType(std::string type);
std::string getElementType(std::string type);

} // namespace typechecker
