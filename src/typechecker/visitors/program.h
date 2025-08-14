#pragma once

#include "bnfc/Absyn.H"
#include "typechecker/symboltable.h"
#include <memory>
namespace typechecker {
class ProgramVisitor : public Visitor {
private:
  std::shared_ptr<SymbolTable> symbolTable;
  Prog *ast;
  bool definitionsDone;
  bool structsDone;
  bool typedefsDone;

public:
  ProgramVisitor(std::shared_ptr<SymbolTable> symbolTable, Prog *ast)
      : symbolTable(symbolTable), ast(ast), definitionsDone(false), structsDone(false),
        typedefsDone(false) {}
  ~ProgramVisitor() = default;
  void visitFnDef(FnDef *p);
  void visitArgument(Argument *p);
  void visitProgram(Program *p);
  void visitTopDef(TopDef *p);
  void defintionPhase(FnDef *p);
  void typecheckingPhase(FnDef *p);
  void visitStructDef(StructDef *p);
  void visitTypedef(Typedef *p);

  // Unused functions.
  void visitProg(Prog *p) {}
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
} // namespace typechecker
