#include "compiler.h"
#include "codegen/environment.h"
#include "visitors/program.h"
#include <memory>

namespace codegen {
void compile(Prog *ast) {
  std::shared_ptr<Environment> env = std::make_shared<Environment>();
  std::unique_ptr<ProgramVisitor> visitor = std::make_unique<ProgramVisitor>(env, ast);
  ast->accept(visitor.get());
  env->getModule().print(llvm::outs(), nullptr);
}
} // namespace codegen
