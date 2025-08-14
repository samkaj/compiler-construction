#include "converter.h"
#include "bnfc/Absyn.H"

namespace codegen {
MultiplicationOp convert(MulOp *mulOp) {
  if (dynamic_cast<Times *>(mulOp)) {
    return MultiplicationOp::Multiplication;
  } else if (dynamic_cast<Div *>(mulOp)) {
    return MultiplicationOp::Division;
  } else if (dynamic_cast<Mod *>(mulOp)) {
    return MultiplicationOp::Modulo;
  }
  return MultiplicationOp::Multiplication; // unreachable
}

RelationalOp convert(RelOp *relOp) {
  if (dynamic_cast<LTH *>(relOp)) {
    return RelationalOp::LessThan;
  } else if (dynamic_cast<LE *>(relOp)) {
    return RelationalOp::LessThanOrEqual;
  } else if (dynamic_cast<GTH *>(relOp)) {
    return RelationalOp::GreaterThan;
  } else if (dynamic_cast<GE *>(relOp)) {
    return RelationalOp::GreaterThanOrEqual;
  } else if (dynamic_cast<EQU *>(relOp)) {
    return RelationalOp::Equal;
  } else if (dynamic_cast<NE *>(relOp)) {
    return RelationalOp::NotEqual;
  }
  return RelationalOp::Equal; // unreachable
}

AdditiveOp convert(AddOp *addOp) {
  if (dynamic_cast<Plus *>(addOp)) {
    return AdditiveOp::Addition;
  } else if (dynamic_cast<Minus *>(addOp)) {
    return AdditiveOp::Subtraction;
  }
  return AdditiveOp::Addition; // unreachable
}
} // namespace codegen
