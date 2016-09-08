#include "../../include/AstFunc.h"
#include "statement.h"
#include "expression.h"

FunctionInfo* FuncDecl::codeGen() {
	vector<TypeInfo*> returnInfos;
	vector<Type*> returnllvmTypes;
	if (style == 0) {
		for (unsigned i = 0; i < returnTypes.size(); i++) {
			TypeInfo *clazz = returnTypes[i]->getClassInfo();
			if (clazz == NULL) {
				throwError(this);
			}
			returnInfos.push_back(clazz);
			returnllvmTypes.push_back(clazz->llvmType);
		}
	} else {
		for (unsigned i = 0; i < returnDecls.size(); i++) {
			TypeInfo *clazz = returnDecls[i]->typeDecl->getClassInfo();
			if (clazz == NULL) {
				throwError(this);
			}
			returnInfos.push_back(clazz);
			returnllvmTypes.push_back(clazz->llvmType);
		}

	}

	Type *returnType = NULL;
	if (returnllvmTypes.size() == 0) {
		returnType = builder.getVoidTy();
	} else if (returnllvmTypes.size() == 1) {
		returnType = returnllvmTypes[0];
	} else {
		ArrayRef<Type*> typesArray(returnllvmTypes);
		returnType = StructType::create(context, typesArray);
	}

	vector<TypeInfo*> argInfos;
	vector<Type*> argllvmTypes;
	for (unsigned i = 0; i < argDecls.size(); i++) {
		SimpleVarDecl *argDecl = argDecls[i];
		TypeInfo *clazz = argDecl->typeDecl->getClassInfo();
		if (clazz == NULL) {
			throwError(argDecl);
		}
		argInfos.push_back(clazz);
		argllvmTypes.push_back(clazz->llvmType);
	}

	FunctionType *functionType = NULL;
	if (argllvmTypes.size() == 0) {
		functionType = FunctionType::get(returnType, false);
	} else {
		ArrayRef<Type*> argTypeArrayRef(argllvmTypes);
		functionType = FunctionType::get(returnType, argTypeArrayRef, false);
	}
	Function *function = Function::Create(functionType,
			Function::ExternalLinkage, funcName + "_sp", &module);
	FunctionInfo *functionInfo = new FunctionInfo(funcName, function,
			returnInfos, argInfos, style);
	if (!globalContext.addFunction(functionInfo)) {
		throwError(this);
	}

	this->functionInfo = functionInfo;
	return functionInfo;
}

Function* FuncDef::declGen() {
	functionInfo = funcDecl->codeGen();
	if (funcDecl->funcName == "main") {
		if (functionInfo->returnClasses.size() != 1
				|| functionInfo->returnClasses[0] != intClass
				|| functionInfo->argClasses.size() != 0) {
			errorMsg =
					"main function must be of type 'long main(char[][] args)'";
			throwError(funcDecl);
		}
	}
	return functionInfo->llvmFunction;
}

void FuncDef::codeGen() {
	Function *function = functionInfo->llvmFunction;
	vector<TypeInfo*> &returnClasses = functionInfo->returnClasses;
	vector<TypeInfo*> &argClasses = functionInfo->argClasses;
	AstContext astContext;

	BasicBlock *allocBB = BasicBlock::Create(context, "alloc", function);
	BasicBlock *entryBB = BasicBlock::Create(context, "entry", function);
	astContext.allocBB = allocBB;
	builder.SetInsertPoint(allocBB);
	unsigned i = 0;
	for (Function::arg_iterator ai = function->arg_begin();
			ai != function->arg_end(); ai++, i++) {
		SimpleVarDecl *argDecl = funcDecl->argDecls[i];
		TypeInfo *argClazz = argClasses[i];
		Value *alloc = builder.CreateAlloca(argClazz->llvmType);
		builder.CreateStore(ai, alloc);
		if (!astContext.addVar(argDecl->varName, AstValueInfo(alloc, argClazz))) {
			throwError(argDecl);
		}
	}

	vector<Value*> returnVars;
	if (functionInfo->returnNum > 0) {
		Value *retAlloc = builder.CreateAlloca(functionInfo->returnType);
		astContext.returnAlloc = retAlloc;
		for (i = 0; i < functionInfo->returnNum; i++) {
			TypeInfo *retClazz = returnClasses[i];
			Value *retElement = NULL;
			if (functionInfo->returnNum == 1) {
				retElement = retAlloc;
			} else {
				retElement = builder.CreateStructGEP(retAlloc, i);
			}
			builder.CreateStore(retClazz->getInitial(), retElement);
			returnVars.push_back(retElement);
			if (funcDecl->style == 1) {
				SimpleVarDecl *retDecl = funcDecl->returnDecls[i];
				if (!astContext.addVar(retDecl->varName,
						AstValueInfo(retElement, retClazz))) {
					throwError(retDecl);
				}
			}
		}
	}
	astContext.returnVars = &returnVars;

	astContext.currentFunc = functionInfo;
	builder.SetInsertPoint(entryBB);
	stmtBlock->codeGen(astContext);

	if (functionInfo->returnNum == 0) {
		builder.CreateRetVoid();
	} else {
		builder.CreateRet(builder.CreateLoad(astContext.returnAlloc));
	}

	builder.SetInsertPoint(allocBB);
	builder.CreateBr(entryBB);
}



void Program::codeGen() {
	// func decl gen
	for (unsigned i = 0; i < funcDefs.size(); i++) {
		funcDefs[i]->declGen();
	}

	// create main func and global var gen
	FunctionType *mainFuncType = FunctionType::get(int64Type, false);
	mainFunc = Function::Create(mainFuncType, Function::ExternalLinkage,
			"start_program", &module);
	builder.SetInsertPoint(BasicBlock::Create(context, "entry", mainFunc));
	for (unsigned i = 0; i < varDefs.size(); i++) {
		varDefs[i]->globalGen();
	}
	string mainStr = "main";
	AstFunctionInfo mainF = globalContext.getFunctionV(mainStr);
	Function *mainf = (Function*) mainF.llvmFunc;
	if (mainf == NULL) {
		cout << errorMsg << endl;
		builder.CreateRet(int64_0);
	} else {
		builder.CreateRet(builder.CreateCall(mainf));
	}

	// function gen
	for (unsigned i = 0; i < funcDefs.size(); i++) {
		funcDefs[i]->codeGen();
	}
}

TypeInfo* TypeDecl::getClassInfo() {
	TypeInfo *classInfo = globalContext.getClass(typeName);
	if (classInfo == NULL) {
		throwError(this);
	}
	return classInfo;
}
