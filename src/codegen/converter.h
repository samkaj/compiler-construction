#pragma once

#include "bnfc/Absyn.H"

namespace codegen {
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

MultiplicationOp convert(MulOp *mulOp);
RelationalOp convert(RelOp *relOp);
AdditiveOp convert(AddOp *addOp);
} // namespace codegen
