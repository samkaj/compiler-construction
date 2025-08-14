#include "bnfc/Absyn.H"
#include "bnfc/Parser.H"
#include "codegen/compiler.h"
#include "typechecker/typechecker.h"
#include <iostream>
#include <memory>

int main(int argc, char **argv) {
  FILE *input = stdin;
  if (argc > 1) {
    input = fopen(argv[1], "r");
    if (!input) {
      std::cerr << "ERROR: Could not open file " << argv[1] << std::endl;
      return 1;
    }
  }

  std::unique_ptr<Prog> parse_tree(pProg(input));
  if (!parse_tree) {
    std::cerr << "ERROR: Parsing failed" << std::endl;
    return 1;
  }

  typechecker::typecheck(parse_tree.get());
  codegen::compile(parse_tree.get());

  if (input != stdin) {
    fclose(input);
  }

  std::cerr << "OK" << std::endl;
  return 0;
}
