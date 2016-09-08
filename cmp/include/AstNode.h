#ifndef AST_NODE_H_
#define AST_NODE_H_

#include "share.h"

using namespace std;
using namespace llvm;

//记录出错信息块,所有AST的节点都从此类派生
class AstNode {
public:
	int firstLine;
	int firstColumn;

};


//程序块:变量定义,函数定义
class Program: public AstNode {
public:
	vector<VarDef*> varDefs;
	vector<FuncDef*> funcDefs;

	void addVarDef(VarDef *varDef) {
		varDefs.push_back(varDef);
	}

	void addFuncDef(FuncDef *funcDef) {
		funcDefs.push_back(funcDef);
	}

	void codeGen();
};

//变量定义:类型,名称
class SimpleVarDecl: public AstNode {
public:
	TypeDecl *typeDecl;
	string varName;

	SimpleVarDecl(TypeDecl *typeDecl, string &varName) {
		this->typeDecl = typeDecl;
		this->varName = varName;
	}
};

//类型定义:类型,维数(扩展)
class TypeDecl: public AstNode {
public:
	string typeName;
	unsigned dimension;

	TypeDecl(string &typeName, unsigned dimension = 0) {
		this->typeName = typeName;
		this->dimension = dimension;
	}

	TypeInfo* getClassInfo();
};
//单一变量初始化:变量,表达式
class VarInit: public AstNode {
public:
	string varName;
	Expression *expr;

	VarInit(string &varName, Expression *expr = NULL) {
		this->varName = varName;
		this->expr = expr;
	}
};

//函数声明:返回类型,函数名,参数
class FuncDecl: public AstNode {
public:
	vector<TypeDecl*> returnTypes;
	vector<SimpleVarDecl*> returnDecls;
	string funcName;
	vector<SimpleVarDecl*> argDecls;
	int style; //0为单一变量,1为返回列表

	FunctionInfo *functionInfo;

	FuncDecl(vector<TypeDecl*> &returnTypes, string &funcName,
			vector<SimpleVarDecl*> &argDecls) {
		this->returnTypes = returnTypes;
		this->funcName = funcName;
		this->argDecls = argDecls;
		this->style = 0;
		this->functionInfo = NULL;
	}

	FuncDecl(vector<SimpleVarDecl*> &returnDecls, string &funcName,
			vector<SimpleVarDecl*> &argDecls) {
		this->returnDecls = returnDecls;
		this->funcName = funcName;
		this->argDecls = argDecls;
		this->style = 1;
		this->functionInfo = NULL;
	}

	FunctionInfo* codeGen();
};

//函数定义:函数声明.函数体,函数相关信息指针
class FuncDef: public AstNode {
public:
	FuncDecl *funcDecl;
	StmtBlock *stmtBlock;

	FunctionInfo *functionInfo;

	FuncDef(FuncDecl *funcDecl, StmtBlock *stmtBlock) {
		this->funcDecl = funcDecl;
		this->stmtBlock = stmtBlock;
		this->functionInfo = NULL;
	}

	Function* declGen();
	void codeGen();
};



#endif
