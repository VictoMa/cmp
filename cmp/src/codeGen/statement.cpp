#include "statement.h"

#include "../../include/AstFunc.h"
#include "expression.h"

void ForStmt::codeGen(AstContext &astContext) {
	Function *func = astContext.currentFunc->llvmFunction;
	AstContext headContext(&astContext);
	initStmt->codeGen(headContext);
	BasicBlock *forHeadBB = BasicBlock::Create(context, "forhead", func);
	BasicBlock *forBodyBB = BasicBlock::Create(context, "forbody");
	BasicBlock *forFootBB = BasicBlock::Create(context, "forfoot");
	BasicBlock *outBB = BasicBlock::Create(context, "outfor");
	builder.CreateBr(forHeadBB);

	builder.SetInsertPoint(forHeadBB);
	condExpr->expectedType = boolClass;
	AstValueInfo cond = condExpr->codeGen(headContext);
	builder.CreateCondBr(cond.llvmValue, forBodyBB, outBB);

	func->getBasicBlockList().push_back(forBodyBB);
	builder.SetInsertPoint(forBodyBB);
	AstContext bodyContext(&headContext);
	bodyContext.breakOutBB = outBB;
	bodyContext.continueBB = forFootBB;
	block->codeGen(bodyContext);
	builder.CreateBr(forFootBB);

	func->getBasicBlockList().push_back(forFootBB);
	builder.SetInsertPoint(forFootBB);
	loopStmt->codeGen(headContext);
	builder.CreateBr(forHeadBB);

	func->getBasicBlockList().push_back(outBB);
	builder.SetInsertPoint(outBB);
}


void IfElseStmt::codeGen(AstContext &astContext) {
	AstValueInfo cond = condExpr->codeGen(astContext);
	if (!cond.castTo(boolClass)) {
		throwError(condExpr);
	}
	Function *func = astContext.currentFunc->llvmFunction;
	BasicBlock *thenBB = BasicBlock::Create(context, "then", func);
	BasicBlock *elseBB = BasicBlock::Create(context, "else");
	BasicBlock *outBB = BasicBlock::Create(context, "outif");
	builder.CreateCondBr(cond.llvmValue, thenBB, elseBB);

	builder.SetInsertPoint(thenBB);
	AstContext ifContext(&astContext);
	thenBlock->codeGen(ifContext);
	builder.CreateBr(outBB);
	func->getBasicBlockList().push_back(elseBB);

	builder.SetInsertPoint(elseBB);
	AstContext elseContext(&astContext);
	if (elseBlock != NULL) {
		elseBlock->codeGen(elseContext);
	}
	builder.CreateBr(outBB);
	func->getBasicBlockList().push_back(outBB);
	builder.SetInsertPoint(outBB);
}

void ReturnStmt::codeGen(AstContext &astContext) {
	FunctionInfo *currentFunc = astContext.currentFunc;
	if (currentFunc->style == 0) {
		vector<TypeInfo*> &returnClasses = currentFunc->returnClasses;
		if (exprList.size() < returnClasses.size()) {
			errorMsg = "too few values to return in function '"
					+ currentFunc->name + "'";
			throwError(this);
		} else if (exprList.size() > returnClasses.size()) {
			errorMsg = "too many values to return in function '"
					+ currentFunc->name + "'";
			throwError(this);
		}

		vector<Value*> &returnVars = *astContext.returnVars;
		for (unsigned i = 0; i < exprList.size(); i++) {
			exprList[i]->expectedType = returnClasses[i];
			AstValueInfo v = exprList[i]->codeGen(astContext);
			builder.CreateStore(v.llvmValue, returnVars[i]);
		}
		if (returnClasses.size() == 0) {
			builder.CreateRetVoid();
		} else {
			builder.CreateRet(builder.CreateLoad(astContext.returnAlloc));
		}
	} else {
		if (exprList.size() > 0) {
			errorMsg =
					"needn't declare any expression behind 'return' in style 1 function";
			throwError(this);
		}
		if (currentFunc->returnNum == 0) {
			builder.CreateRetVoid();
		} else {
			builder.CreateRet(builder.CreateLoad(astContext.returnAlloc));
		}
	}
	BasicBlock *anonyBB = BasicBlock::Create(context, "after_return",
			currentFunc->llvmFunction);
	builder.SetInsertPoint(anonyBB);
}



void SimpleStmtList::codeGen(AstContext &astContext) {
	for (unsigned i = 0; i < stmtList.size(); i++) {
		stmtList[i]->codeGen(astContext);
	}
}

void ExprStmt::codeGen(AstContext &astContext) {
	expr->codeGen(astContext);
}

void BreakStmt::codeGen(AstContext &astContext) {
	if (astContext.breakOutBB == NULL) {
		errorMsg = "break statement not within for";
		throwError(this);
	}
	builder.CreateBr(astContext.breakOutBB);
	BasicBlock *anonyBB = BasicBlock::Create(context, "after_break",
			astContext.currentFunc->llvmFunction);
	builder.SetInsertPoint(anonyBB);
}

void ContinueStmt::codeGen(AstContext &astContext) {
	if (astContext.continueBB == NULL) {
		errorMsg = "continue statement not within for";
		throwError(this);
	}
	builder.CreateBr(astContext.continueBB);
	BasicBlock *anonyBB = BasicBlock::Create(context, "after_continue",
			astContext.currentFunc->llvmFunction);
	builder.SetInsertPoint(anonyBB);
}

void StmtBlock::codeGen(AstContext &astContext) {
	AstContext newContext(&astContext);
	for (unsigned i = 0; i < statements.size(); i++) {
		statements[i]->codeGen(newContext);
	}
}

void IncStmt::codeGen(AstContext &astContext) {
	AstValueInfo var = expr->lvalueGen(astContext);
	if (var.isBool()) {
		if (inc) {
			errorMsg = "invalid operand to expression (" + var.clazz->name
					+ "++)";
		} else {
			errorMsg = "invalid operand to expression (" + var.clazz->name
					+ "--)";
		}
		throwError(this);
	}
	Value *v = builder.CreateLoad(var.llvmValue);
	if (inc) {
		if (var.isInt()) {
			v = builder.CreateAdd(v, builder.getInt64(1));
		} else if (var.isChar()) {
			v = builder.CreateAdd(v, builder.getInt32(1));
		} else {
			v = builder.CreateFAdd(v, ConstantFP::get(doubleType, 1));
		}
	} else {
		if (var.isInt()) {
			v = builder.CreateSub(v, builder.getInt64(1));
		} else if (var.isChar()) {
			v = builder.CreateSub(v, builder.getInt32(1));
		} else {
			v = builder.CreateFSub(v, ConstantFP::get(doubleType, 1));
		}
	}
	builder.CreateStore(v, var.llvmValue);
}


void VarAssi::codeGen(AstContext &astContext) {
	AstValueInfo var = leftExpr->lvalueGen(astContext);
	expr->expectedType = var.clazz;
	AstValueInfo value = expr->codeGen(astContext);
	builder.CreateStore(value.llvmValue, var.llvmValue);
}




void VarDef::globalGen() {
	TypeInfo *classInfo = typeDecl->getClassInfo();
	Constant *initial = classInfo->getInitial();
	for (unsigned i = 0; i < varInitList.size(); i++) {
		VarInit *varInit = varInitList[i];
		Value *llvmVar = new GlobalVariable(module, classInfo->llvmType, false,
				GlobalValue::ExternalLinkage, initial);
		AstContext astContext;
		if (varInit->expr != NULL) {
			varInit->expr->expectedType = classInfo;
			AstValueInfo v = varInit->expr->codeGen(astContext);
			builder.CreateStore(v.llvmValue, llvmVar);
		}
		globalContext.addVar(varInit->varName, AstValueInfo(llvmVar, classInfo));
	}
}

void VarDef::codeGen(AstContext &astContext) {
	TypeInfo *classInfo = typeDecl->getClassInfo();
	for (unsigned i = 0; i < varInitList.size(); i++) {
		VarInit *varInit = varInitList[i];
		Value *value = NULL;
		if (varInit->expr != NULL) {
			varInit->expr->expectedType = classInfo;
			AstValueInfo v = varInit->expr->codeGen(astContext);
			value = v.llvmValue;
		} else {
			value = classInfo->getInitial();
		}
		Value *var = createAlloca(classInfo->llvmType, astContext.allocBB);
		if (!astContext.addVar(varInit->varName, AstValueInfo(var, classInfo))) {
			throwError(varInit);
		}
		builder.CreateStore(value, var);
	}
}


void WhileStmt::codeGen(AstContext &astContext) {
	Function *func = astContext.currentFunc->llvmFunction;
	AstContext headContext(&astContext);
	BasicBlock *whileHeadBB = BasicBlock::Create(context, "whilehead", func);
	BasicBlock *whileBodyBB = BasicBlock::Create(context, "whilebody");
	BasicBlock *outBB = BasicBlock::Create(context, "outwhile");
	builder.CreateBr(whileHeadBB);

	builder.SetInsertPoint(whileHeadBB);
	condExpr->expectedType = boolClass;
	AstValueInfo cond = condExpr->codeGen(headContext);
	builder.CreateCondBr(cond.llvmValue, whileBodyBB, outBB);

	func->getBasicBlockList().push_back(whileBodyBB);
	builder.SetInsertPoint(whileBodyBB);
	AstContext bodyContext(&headContext);
	bodyContext.breakOutBB = outBB;
	bodyContext.continueBB = whileHeadBB;
	block->codeGen(bodyContext);
	builder.CreateBr(whileHeadBB);

	func->getBasicBlockList().push_back(outBB);
	builder.SetInsertPoint(outBB);
}
