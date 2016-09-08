#ifndef AST_SUPPORT_H_
#define AST_SUPPORT_H_

#include "share.h"

using namespace std;
using namespace llvm;


//引用./main.cpp中的变量定义


extern LLVMContext &context;
extern Module module;
extern IRBuilder<> builder;
extern DataLayout *dataLayout;
extern GlobalContext globalContext;

extern Type *int64Type;
extern Type *int32Type;
extern Type *doubleType;
extern Type *boolType;
extern Type *voidType;
extern TypeInfo *intClass;
extern TypeInfo *charClass;
extern TypeInfo *doubleClass;
extern TypeInfo *boolClass;
extern TypeInfo *voidClass;

extern Constant *int64_0;
extern Constant *int32_0;
extern Constant *double_0;
extern Constant *bool_true;
extern Constant *bool_false;

extern Function *mainFunc;

extern string errorMsg;

extern void throwError(AstNode *node);
extern string getOperatorName(int op);
extern Value* createAlloca(Type *type, BasicBlock *bb);


//类型信息:类型名,LLVM类型
class TypeInfo {
public:
	string name;
	Type *llvmType;

	TypeInfo(string name, Type *llvmType) {
		this->name = name;
		this->llvmType = llvmType;
	}

	Constant* getInitial();

	bool isBoolType();
	bool isIntType();
	bool isDoubleType();
	bool isCharType();
};

//函数相关信息:函数名,LLVM函数类型,返回类型,返回值,参数值
class FunctionInfo {
public:
	string name;
	Function *llvmFunction;

	Type *returnType;
	int returnNum;
	vector<TypeInfo*> returnClasses;
	vector<TypeInfo*> argClasses;
	int style;  //函数类型说明,0普通函数,1return decl,2cons

	FunctionInfo(string name, Function *llvmFunction,
			vector<TypeInfo*> &returnClasses, vector<TypeInfo*> &argClasses,
			int style = 0) {
		this->name = name;
		this->llvmFunction = llvmFunction;
		this->returnClasses = returnClasses;
		this->argClasses = argClasses;
		this->style = style;

		this->returnNum = returnClasses.size();
		this->returnType = llvmFunction->getReturnType();
	}
	//无参构造函数
	FunctionInfo() {
		this->llvmFunction = NULL;
		this->returnType = NULL;
		this->style = 0;
		this->returnNum = 0;
	}
};

//节点-值类型:类型信息,值
class AstValueInfo {
public:
	Value *llvmValue;
	TypeInfo *clazz;

	AstValueInfo(Value *llvmValue = NULL, TypeInfo *clazz = NULL, bool isReadOnly =
			false) {
		this->llvmValue = llvmValue;
		this->clazz = clazz;
	}

	bool castTo(TypeInfo *destClazz);
	bool isBool();
	bool isInt();
	bool isChar();
	bool isDouble();
};

//结点-函数:函数信息,结点值
class AstFunctionInfo {
public:
	Value *llvmFunc;
	FunctionInfo *funcInfo;

	AstFunctionInfo(Value *llvmFunc = NULL, FunctionInfo *funcInfo = NULL) {
		this->llvmFunc = llvmFunc;
		this->funcInfo = funcInfo;
	}

};


//结点-上下文信息:指针,变量map,当前函数信息,基本块+循环结构块,返回值
class AstContext {
public:
	AstContext *superior;
	map<string, AstValueInfo> varTable;
	FunctionInfo *currentFunc;
	BasicBlock *allocBB;
	BasicBlock *breakOutBB;
	BasicBlock *continueBB;
	vector<Value*> *returnVars;
	Value *returnAlloc;

	explicit AstContext(AstContext *superior = NULL) {
		this->superior = superior;
		if (superior != NULL) {
			this->currentFunc = superior->currentFunc;
			this->allocBB = superior->allocBB;
			this->breakOutBB = superior->breakOutBB;
			this->continueBB = superior->continueBB;
			this->returnVars = superior->returnVars;
			this->returnAlloc = superior->returnAlloc;
		} else {
			this->currentFunc = NULL;
			this->allocBB = NULL;
			this->breakOutBB = NULL;
			this->continueBB = NULL;
			this->returnVars = NULL;
			this->returnAlloc = NULL;
		}
	}

	bool addVar(string &name, AstValueInfo avalue);
	AstValueInfo getVar(string &name);
	AstFunctionInfo getFunc(string &name);
};

//全局上下文
class GlobalContext {
public:
	map<string, TypeInfo*> classTable;
	map<string, FunctionInfo*> functionTable;
	map<string, AstValueInfo> varTable;

	bool addClass(TypeInfo *clazz);
	bool addFunction(FunctionInfo *func);
	bool addVar(string &name, AstValueInfo avalue);
	TypeInfo* getClass(string &name);
	AstFunctionInfo getFunctionV(string &name);
	AstValueInfo getVar(string &name);
};

#endif
