#ifndef AST_STATEMENT_H_
#define AST_STATEMENT_H_

#include "AstNode.h"

using namespace std;


//语句相关声明
enum StmtType {
	NORMAL,
	STMT_BLOCK,
	VAR_DEF,
	VAR_ASSI,
	SIMPLE_LIST,
	EXPR_STMT,
	IFELSE_STMT,
	FOR_STMT,
	NULL_STMT,
	RETRUN_STMT,
	BREAK_STMT,
	CONTINUE_STMT,
	WHILE_STMT
};

//语句:语句类型
class Statement: public AstNode {
public:
	StmtType type;
	virtual void codeGen(AstContext &astContext)=0;
	virtual ~Statement() {
	}
};

//语句块:基类:语句类,语句<vector>
class StmtBlock: public Statement {
public:
	vector<Statement*> statements;

	//多重语句的构造函数
	StmtBlock(vector<Statement*> &statements) {
		this->statements = statements;
		this->type = STMT_BLOCK;
	}
	//单一语句的构造函数
	StmtBlock(Statement *statement) {
		statements.push_back(statement);
		this->type = STMT_BLOCK;
	}

	void codeGen(AstContext &astContext);
};

//语句-变量定义:类型,初始化值列表
class VarDef: public Statement {
public:
	TypeDecl *typeDecl;
	vector<VarInit*> varInitList;

	VarDef(TypeDecl *typeDecl, vector<VarInit*> &varInitList) {
		this->typeDecl = typeDecl;
		this->varInitList = varInitList;
		this->type = VAR_DEF;
	}

	void globalGen();
	void codeGen(AstContext &astContext);
};
//语句-变量赋值:左值表达式,表达式
class VarAssi: public Statement {
public:
	LeftValueExpr *leftExpr;
	Expression *expr;

	VarAssi(LeftValueExpr *leftExpr, Expression *expr) {
		this->leftExpr = leftExpr;
		this->expr = expr;
		this->type = VAR_ASSI;
	}

	void codeGen(AstContext &astContext);
};


//语句-简单语句列表:语句列表
class SimpleStmtList: public Statement {
public:
	vector<Statement*> stmtList;

	SimpleStmtList() {
		this->type = SIMPLE_LIST;
	}

	void codeGen(AstContext &astContext);
	void add(Statement* stmt) {
		stmtList.push_back(stmt);
	}
};

//语句-表达式语句:表达式
class ExprStmt: public Statement {
public:
	Expression *expr;

	ExprStmt(Expression *expr) {
		this->expr = expr;
		this->type = EXPR_STMT;
	}

	void codeGen(AstContext &astContext);
};

//语句-自增减语句:左值表达式,增减
class IncStmt: public Statement {
public:
	LeftValueExpr *expr;
	bool inc;

	IncStmt(LeftValueExpr *expr, bool inc = true) {
		this->expr = expr;
		this->inc = inc;
		this->type = EXPR_STMT;
	}

	void codeGen(AstContext &astContext);
};
//语句-IfElse语句:条件,IF块,ELSE块
class IfElseStmt: public Statement {
public:
	Expression *condExpr;
	Statement *thenBlock;
	Statement *elseBlock;

	IfElseStmt(Expression *condExpr, Statement *thenBlock,
			Statement *elseBlock) {
		this->condExpr = condExpr;
		this->thenBlock = thenBlock;
		this->elseBlock = elseBlock;
		this->type = IFELSE_STMT;
	}

	void codeGen(AstContext &astContext);
};

//语句-For语句:前置表达式,条件,后缀表达式,循环体
class ForStmt: public Statement {
public:
	Statement *initStmt;
	Expression *condExpr;
	Statement *loopStmt;
	Statement *block;

	ForStmt(Statement *initStmt, Expression *condExpr, Statement *loopStmt,
			Statement *block) {
		this->initStmt = initStmt;
		this->condExpr = condExpr;
		this->loopStmt = loopStmt;
		this->block = block;
		this->type = FOR_STMT;
	}

	void codeGen(AstContext &astContext);
};
//语句-While语句:条件,循环体
class WhileStmt:public Statement {
public:
	Expression *condExpr;
	Statement *block;
	
	WhileStmt(Expression *condExpr, Statement *block) {
		this->condExpr = condExpr;
		this->block = block;
		this->type = WHILE_STMT;
	}

	void codeGen(AstContext &astContext);
};

//语句-空语句:
class NullStmt: public Statement {
public:
	NullStmt() {
		this->type = NULL_STMT;
	}

	void codeGen(AstContext &astContext) {
	}
};

//语句-返回语句:表达式列表
class ReturnStmt: public Statement {
public:
	vector<Expression*> exprList;

	ReturnStmt(vector<Expression*> &exprList) {
		this->exprList = exprList;
		this->type = RETRUN_STMT;
	}

	void codeGen(AstContext &astContext);
};

//语句-Break语句
class BreakStmt: public Statement {
public:
	BreakStmt() {
		this->type = BREAK_STMT;
	}

	void codeGen(AstContext &astContext);
};

//语句-Continue语句
class ContinueStmt: public Statement {
public:
	ContinueStmt() {
		this->type = CONTINUE_STMT;
	}

	void codeGen(AstContext &astContext);
};

#endif
