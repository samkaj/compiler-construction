#include "fail.h"
#include <iostream>

namespace fail {
void fail(std::string message) {
  std::cerr << "ERROR:\nTYPE ERROR:\n\t" << message << std::endl;
  exit(1);
}
void panic(std::string message) {
  std::cerr << "ERROR:\nPANIC:\n\t" << message << std::endl;
  exit(1);
}
} // namespace fail
