#ifndef AST_EXPRESSION_H_
#define AST_EXPRESSION_H_

#include "AstFunc.h"
#include "AstNode.h"

//结点-值定义
class AstValueInfo;

using namespace std;


//表达式:表达式自身类型
class Expression: public AstNode {
public:
	TypeInfo *expectedType;

	Expression() {
		this->expectedType = NULL;
	}

	virtual ~Expression() {
	}

	AstValueInfo codeGen(AstContext &astContext) {
		AstValueInfo value = gen(astContext);
		//类型检查
		if (expectedType != NULL && !value.castTo(expectedType)) {
			throwError(this);
		}
		return value;
	}
protected:
	virtual AstValueInfo gen(AstContext &astContext)=0;

};

//表达式-左值表达式
class LeftValueExpr: public Expression {
public:
	virtual AstValueInfo lvalueGen(AstContext &astContext)=0;
};


//表达式-标识符:标识符名称
class IdentExpr: public LeftValueExpr {
public:
	string ident;

	IdentExpr(string &ident) {
		this->ident = ident;
	}

	AstValueInfo gen(AstContext &astContext);
	AstValueInfo lvalueGen(AstContext &astContext);
};

//表达式-二元表达式:左操作数,操作符,右操作数
class BinaryOpExpr: public Expression {
public:
	Expression *leftExpr;
	Expression *rightExpr;
	int op;

	BinaryOpExpr(Expression *leftExpr, int op, Expression *rightExpr) {
		this->leftExpr = leftExpr;
		this->op = op;
		this->rightExpr = rightExpr;
	}

	AstValueInfo gen(AstContext &astContext);
};


//表达式-二元逻辑表达式:左操作数,操作符,右操作数(EXPECTED BOOL)
class BinaryLogicExpr: public Expression {
public:
	Expression *leftExpr;
	Expression *rightExpr;
	int op;

	BinaryLogicExpr(Expression *leftExpr, int op, Expression *rightExpr) {
		this->leftExpr = leftExpr;
		this->op = op;
		this->rightExpr = rightExpr;
	}

	AstValueInfo gen(AstContext &astContext);
};


//表达式-前置表达式
class PrefixOpExpr: public Expression {
public:
	int op;
	Expression *expr;

	PrefixOpExpr(int op, Expression *expr) {
		this->op = op;
		this->expr = expr;
	}

	AstValueInfo gen(AstContext &astContext);
};

//表达式-函数调用:表达式,函数名,表达式表,是否构造函数
class FuncInvoke: public Expression {
public:
	Expression *expr;
	string funcName;
	vector<Expression*> exprList;
	bool isConstructor;

	FuncInvoke(Expression *expr, string &funcName,
			vector<Expression*> &exprList, bool isConstructor = false) {
		this->funcName = funcName;
		this->exprList = exprList;
		this->expr = expr;
		this->isConstructor = isConstructor;
	}

	vector<AstValueInfo> multiCodeGen(AstContext &astContext);
	AstValueInfo gen(AstContext &astContext);
};

//表达式-整数型:值
class Int: public Expression {
public:
	int64_t value;

	Int(int64_t value) {
		this->value = value;
	}

	AstValueInfo gen(AstContext &astContext);
};

//表达式-字符型:值
class Char: public Expression {
public:
	int32_t value;
	Char(int32_t value) {
		this->value = value;
	}

	AstValueInfo gen(AstContext &astContext);
};


//表达式-双精度:值
class Double: public Expression {
public:
	double value;

	Double(double value) {
		this->value = value;
	}

	AstValueInfo gen(AstContext &astContext);
};

//表达式-布尔:值
class Bool: public Expression {
public:
	bool value;

	Bool(bool value) {
		this->value = value;
	}

	AstValueInfo gen(AstContext &astContext);
};

#endif
