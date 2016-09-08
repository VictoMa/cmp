#ifndef COMMON_H_
#define COMMON_H_
//提供公共变量的声明
#include <stddef.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>
#include <map>


#include <llvm/Constants.h>
#include <llvm/Function.h>
#include <llvm/Module.h>
#include <llvm/IRBuilder.h>
#include <llvm/Type.h>



class TypeInfo;			//类信息类
class FunctionInfo;			//函数信息类
class AstValueInfo;				//结点的值
class AstFunctionInfo;			//结点内函数
class AstContext;			//结点上下文
class GlobalContext;		//全局上下文

class AstNode;
//程序类
class Program;
class VarInit;
class SimpleVarDecl;
class TypeDecl;
class FuncDef;
class FuncDecl;

//语句类
class Statement;
class StmtBlock;
class VarDef;
class VarAssi;
class SimpleStmtList;
class ExprStmt;
class IfElseStmt;
class ForStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class WhileStmt;
//表达式类
class Expression;
class LeftValueExpr;
class IdentExpr;
class BinaryOpExpr;
class BinaryLogicExpr;
class PrefixOpExpr;
class FuncInvoke;
//类型类
class Int;
class Char;
class Double;
class Bool;

using namespace std;

#endif
