#pragma once

#include "bnfc/Absyn.H"
#include "codegen/environment.h"
#include <memory>
#include <vector>
namespace codegen {
class ProgramVisitor : public Visitor {
private:
  std::vector<llvm::Type *> args;
  std::vector<std::string> argNames;
  void definitionPhase(FnDef *p);
  void compilationPhase(FnDef *p);
  void opaqueStructPhase(StructDef *p);
  void typedefPhase(Typedef *p);
  void setStructBodyPhase(StructDef *p);
  Prog *ast;
  std::shared_ptr<Environment> env;

public:
  ProgramVisitor(std::shared_ptr<Environment> env, Prog *ast) : env(env), ast(ast), args() {}
  ~ProgramVisitor() = default;
  void visitArgument(Argument *p);
  void visitProgram(Program *p);
  void visitTopDef(TopDef *p);
  void visitProg(Prog *p) { p->accept(this); }
  void visitStructDef(StructDef *p);
  void visitTypedef(Typedef *p);

  // Unused functions.
  void visitFnDef(FnDef *p) {}
  void visitArg(Arg *p) {}
  void visitBlock(Block *p) {}
  void visitEmpty(Empty *p) {}
  void visitBStmt(BStmt *p) {}
  void visitDecl(Decl *p) {}
  void visitNoInit(NoInit *p) {}
  void visitInit(Init *p) {}
  void visitAss(Ass *p) {}
  void visitIncr(Incr *p) {}
  void visitDecr(Decr *p) {}
  void visitRet(Ret *p) {}
  void visitVRet(VRet *p) {}
  void visitCond(Cond *p) {}
  void visitCondElse(CondElse *p) {}
  void visitWhile(While *p) {}
  void visitSExp(SExp *p) {}
  void visitBlk(Blk *p) {}
  void visitStmt(Stmt *p) {}
  void visitItem(Item *p) {}
  void visitType(Type *p) {}
  void visitExpr(Expr *p) {}
  void visitAddOp(AddOp *p) {}
  void visitMulOp(MulOp *p) {}
  void visitRelOp(RelOp *p) {}
  void visitInt(Int *p) {}
  void visitDoub(Doub *p) {}
  void visitBool(Bool *p) {}
  void visitVoid(Void *p) {}
  void visitFun(Fun *p) {}
  void visitEVar(EVar *p) {}
  void visitELitInt(ELitInt *p) {}
  void visitELitDoub(ELitDoub *p) {}
  void visitELitTrue(ELitTrue *p) {}
  void visitELitFalse(ELitFalse *p) {}
  void visitEApp(EApp *p) {}
  void visitEString(EString *p) {}
  void visitNeg(Neg *p) {}
  void visitNot(Not *p) {}
  void visitEMul(EMul *p) {}
  void visitEAdd(EAdd *p) {}
  void visitERel(ERel *p) {}
  void visitEAnd(EAnd *p) {}
  void visitEOr(EOr *p) {}
  void visitPlus(Plus *p) {}
  void visitMinus(Minus *p) {}
  void visitTimes(Times *p) {}
  void visitDiv(Div *p) {}
  void visitMod(Mod *p) {}
  void visitLTH(LTH *p) {}
  void visitLE(LE *p) {}
  void visitGTH(GTH *p) {}
  void visitGE(GE *p) {}
  void visitEQU(EQU *p) {}
  void visitNE(NE *p) {}
  void visitListTopDef(ListTopDef *p) {}
  void visitListArg(ListArg *p) {}
  void visitListStmt(ListStmt *p) {}
  void visitListItem(ListItem *p) {}
  void visitListType(ListType *p) {}
  void visitListExpr(ListExpr *p) {}
  void visitInteger(Integer x) {}
  void visitChar(Char x) {}
  void visitDouble(Double x) {}
  void visitString(String x) {}
  void visitIdent(Ident x) {}

  void visitArr(Arr *p) {}
  void visitEArrGet(EArrGet *p) {}
  void visitEArr(EArr *p) {}
  void visitEArrDot(EArrDot *p) {}
  void visitNArr(NArr *p) {}
  void visitInitArr(InitArr *p) {}
  void visitForEach(ForEach *p) {}
  void visitListSize(ListSize *p) {}
  void visitSize(Size *p) {}

  void visitMember(Member *p) {}
  void visitNewPtr(NewPtr *p) {}
  void visitDeref(Deref *p) {}
  void visitNull(Null *p) {}
  void visitFieldAss(FieldAss *p) {}
  void visitStruct(Struct *p) {}
  void visitListField(ListField *p) {}
  void visitField(Field *p) {}
};
} // namespace codegen
