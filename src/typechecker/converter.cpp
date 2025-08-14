#include "converter.h"
#include "bnfc/Absyn.H"

namespace typechecker {
/**
 * Converts a MulOp node to a UnaryOperation enum.
 */
MultiplicationOp convert(MulOp *mulOp) {
  // cast
  if (dynamic_cast<Times *>(mulOp)) {
    return MultiplicationOp::Multiplication;
  } else if (dynamic_cast<Div *>(mulOp)) {
    return MultiplicationOp::Division;
  } else if (dynamic_cast<Mod *>(mulOp)) {
    return MultiplicationOp::Modulo;
  }
  return MultiplicationOp::Multiplication; // unreachable
}

/**
 * Converts a RelOp node to a RelationalOp enum.
 */
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

/**
 * Converts an AddOp node to an AdditiveOp enum.
 */
AdditiveOp convert(AddOp *addOp) {
  if (dynamic_cast<Plus *>(addOp)) {
    return AdditiveOp::Addition;
  } else if (dynamic_cast<Minus *>(addOp)) {
    return AdditiveOp::Subtraction;
  }
  return AdditiveOp::Addition; // unreachable
}

/**
 * Checks if two types are compatible.
 */
bool isTypeCompatible(std::string lhs, std::string rhs) { return lhs == rhs; }

std::string getArrayType(std::string type) { return type + "[]"; }

bool isArrayType(std::string type) { return type.find("[]") != std::string::npos; }

std::string getElementType(std::string type) { return type.substr(0, type.size() - 2); }

} // namespace typechecker
