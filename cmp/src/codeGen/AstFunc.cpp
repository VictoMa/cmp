#include "../../include/AstFunc.h"
#include "../../include/AstNode.h"
#include "statement.h"
#include "expression.h"
#include "parser.hpp"


//值的转换函数
bool AstValueInfo::castTo(TypeInfo *destClass) {
	bool res = false;
	//int -> double char
	if (clazz == destClass) {
		res = true;
	} else if (isInt() && destClass->isDoubleType()) {
		llvmValue = builder.CreateSIToFP(llvmValue, doubleType);
		res = true;
	} else if (isInt() && destClass->isCharType()) {
		llvmValue = builder.CreateTrunc(llvmValue, int32Type);
		res = true;
		//double -> char int
	} else if (isDouble() && destClass->isIntType()) {
		llvmValue = builder.CreateFPToSI(llvmValue, int64Type);
		res = true;
	} else if (isDouble() && destClass->isCharType()) {
		llvmValue = builder.CreateFPToSI(llvmValue, int32Type);
		res = true;
		//char -> int double
	} else if (isChar() && destClass->isIntType()) {
		llvmValue = builder.CreateSExt(llvmValue, int64Type);
		res = true;
	} else if (isChar() && destClass->isDoubleType()) {
		llvmValue = builder.CreateSIToFP(llvmValue, doubleType);
		res = true;
	}
	//如果到此都未转换成功 报错
	if (res) {
		clazz = destClass;
	} else {
		errorMsg = "no viable conversion from '" + clazz->name + "' to '"
				+ destClass->name + "'";
	}
	return res;
}

//判断值的类型
bool AstValueInfo::isBool() {
	return clazz->isBoolType();
}

bool AstValueInfo::isInt() {
	return clazz->isIntType();
}

bool AstValueInfo::isChar() {
	return clazz->isCharType();
}

bool AstValueInfo::isDouble() {
	return clazz->isDoubleType();
}


//给出LLVM常量初始值
Constant* TypeInfo::getInitial() {
	if (llvmType == int64Type) {
		return int64_0;
	} else if (llvmType == int32Type) {
		return int32_0;
	} else if (llvmType == doubleType) {
		return double_0;
	} else if (llvmType == boolType) {
		return bool_false;
	} else {
		errorMsg = "can't init a var of " + name + " class";
		return NULL;
	}
}

//给出类中LLVMType类型判断
bool TypeInfo::isBoolType() {
	return this == boolClass || llvmType == boolType;
}

bool TypeInfo::isIntType() {
	return this == intClass || llvmType == int64Type;
}

bool TypeInfo::isDoubleType() {
	return this == doubleClass || llvmType == doubleType;
}

bool TypeInfo::isCharType() {
	return this == charClass || llvmType == int32Type;
}



//AST类方法
//AST上下文-增加变量:变量名,变量值
bool AstContext::addVar(string& name, AstValueInfo value) {
	if (varTable[name].llvmValue != NULL) {
		errorMsg = "redefine variable '" + name + "'";
		return false;
	}
	varTable[name] = value;
	return true;
}

//AST上下文-返回一个给出名字的变量
AstValueInfo AstContext::getVar(string& name) {
	AstValueInfo var = varTable[name];
	if (var.llvmValue == NULL && superior != NULL) {
		return superior->getVar(name);
	}
	if (var.llvmValue == NULL) {
		var = globalContext.getVar(name);
	}
	if (var.llvmValue == NULL) {
		errorMsg = "undeclared identifier '" + name + "'";
	}
	return var;
}

//AST上下文-返回给出名字的函数
AstFunctionInfo AstContext::getFunc(string& name) {
	return globalContext.getFunctionV(name);
}


//全局上下文-返回给出名字的函数
AstFunctionInfo GlobalContext::getFunctionV(string &name) {
	FunctionInfo *funcInfo = functionTable[name];
	if (funcInfo == NULL) {
		errorMsg = "undeclared function '" + name + "'";
		return AstFunctionInfo();
	} else {
		return AstFunctionInfo(funcInfo->llvmFunction, funcInfo);
	}
}


//全局上下文-增加一个类型
bool GlobalContext::addClass(TypeInfo *clazz) {
	if (classTable[clazz->name] != NULL) {
		errorMsg = "redefine type  '" + clazz->name + "'";
		return false;
	}
	classTable[clazz->name] = clazz;
	return true;
}


//全局上下文-增加一个函数
bool GlobalContext::addFunction(FunctionInfo *func) {
	if (functionTable[func->name] != NULL) {
		errorMsg = "redefine function  '" + func->name + "'";
		return false;
	}
	functionTable[func->name] = func;
	return true;
}


//全局上下文-增加一个变量
bool GlobalContext::addVar(string &name, AstValueInfo value) {
	if (varTable[name].llvmValue != NULL) {
		errorMsg = "redefine variable  '" + name + "'";
		return false;
	}
	varTable[name] = value;
	return true;
}



//全局上下文-获取给定名字的类
TypeInfo* GlobalContext::getClass(string &name) {
	TypeInfo *type = classTable[name];
	if (type == NULL) {
		if (name == "void") {
			errorMsg = "variable has incomplete type 'void'";
		} else {
			errorMsg = "undeclared type '" + name + "'";
		}
	}
	return type;
}


//全局上下文-获取给定名字的变量
AstValueInfo GlobalContext::getVar(string &name) {
	AstValueInfo var = varTable[name];
	if (var.llvmValue == NULL) {
		errorMsg = "undeclared identifier '" + name + "'";
	}
	return var;
}


//由node类给出错误的行号列号
void throwError(AstNode *node) {
	cout << node->firstLine << ":" << node->firstColumn << ": error: "
			<< errorMsg << endl;
	exit(1);
}


//在当前块后分配块
Value* createAlloca(Type *type, BasicBlock *bb) {
	BasicBlock *currentBB = builder.GetInsertBlock();
	builder.SetInsertPoint(bb);
	Value *var = builder.CreateAlloca(type);
	builder.SetInsertPoint(currentBB);
	return var;
}


//获取操作符的string
string getOperatorName(int op) {
	string name;
	if (op < 128) {
		name.push_back(op);
	} else {
		switch (op) {
		case AND:
			name = "&&";
			break;
		case OR:
			name = "||";
			break;
		case NEQUAL:
			name = "!=";
			break;
		case EQUAL:
			name = "==";
			break;
		case LE:
			name = "<=";
			break;
		case GE:
			name = ">=";
			break;
		}
	}
	return name;
}
