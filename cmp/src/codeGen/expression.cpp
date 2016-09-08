#include "expression.h"

#include "../../include/AstFunc.h"
#include "statement.h"
#include "parser.hpp"

//表达式-二元逻辑表达式
AstValueInfo BinaryLogicExpr::gen(AstContext &astContext) {
	Function *currentFunc = astContext.currentFunc->llvmFunction;
	Value *res = createAlloca(boolType, astContext.allocBB);
	leftExpr->expectedType = boolClass;
	AstValueInfo lv = leftExpr->codeGen(astContext);
	builder.CreateStore(lv.llvmValue, res);
	BasicBlock *rexprBB = BasicBlock::Create(context, "", currentFunc);
	BasicBlock *endBB = BasicBlock::Create(context, "");
	if (op == AND) {
		builder.CreateCondBr(lv.llvmValue, rexprBB, endBB);
	} else {
		builder.CreateCondBr(lv.llvmValue, endBB, rexprBB);
	}

	builder.SetInsertPoint(rexprBB);
	rightExpr->expectedType = boolClass;
	AstValueInfo rv = rightExpr->codeGen(astContext);
	builder.CreateStore(rv.llvmValue, res);
	builder.CreateBr(endBB);

	currentFunc->getBasicBlockList().push_back(endBB);
	builder.SetInsertPoint(endBB);
	return AstValueInfo(builder.CreateLoad(res), boolClass);
}

AstValueInfo PrefixOpExpr::gen(AstContext &astContext) {
	AstValueInfo val = expr->codeGen(astContext);
	if (op == '-') {
		if (val.isDouble()) {
			val.llvmValue = builder.CreateFNeg(val.llvmValue);
			return val;
		} else if (val.isInt() || val.isChar()) {
			val.llvmValue = builder.CreateNeg(val.llvmValue);
			return val;
		}
	} else if (op == '!') {
		if (val.isBool()) {
			val.llvmValue = builder.CreateNot(val.llvmValue);
			return val;
		}
	}
	errorMsg = "invalid argument type '" + val.clazz->name + "' to unary '"
			+ getOperatorName(op) + "' expression";
	throwError(this);
	return val;
}

AstValueInfo BinaryOpExpr::gen(AstContext &astContext) {
	AstValueInfo lv = leftExpr->codeGen(astContext);
	AstValueInfo rv = rightExpr->codeGen(astContext);
	AstValueInfo res;
	if (op == '%') {
		if ((lv.isInt() || lv.isChar()) && (rv.isInt() || rv.isChar())) {
			if (lv.isInt()) {
				rv.castTo(intClass);
			} else {
				lv.castTo(rv.clazz);
			}
			res = AstValueInfo(builder.CreateSRem(lv.llvmValue, rv.llvmValue),lv.clazz);
		}
	} else if (lv.isBool() && rv.isBool()) {
		switch (op) {
		case EQUAL:
			res = AstValueInfo(builder.CreateICmpEQ(lv.llvmValue, rv.llvmValue),
					boolClass);
			break;
		case NEQUAL:
			res = AstValueInfo(builder.CreateICmpNE(lv.llvmValue, rv.llvmValue),
					boolClass);
			break;
		}
	} else if ((lv.isInt() || lv.isDouble() || lv.isChar())
			&& (rv.isInt() || rv.isDouble() || rv.isChar())) {
		if (lv.isDouble() || rv.isDouble()) {
			if (!lv.castTo(doubleClass)) {
				throwError(leftExpr);
			}
			if (!rv.castTo(doubleClass)) {
				throwError(rightExpr);
			}
		} else if (lv.isInt() || rv.isInt()) {
			if (!lv.castTo(intClass)) {
				throwError(leftExpr);
			}
			if (!rv.castTo(intClass)) {
				throwError(rightExpr);
			}
		}
		if (lv.isDouble()) {
			switch (op) {
			case '+':
				res = AstValueInfo(builder.CreateFAdd(lv.llvmValue, rv.llvmValue),
						doubleClass);
				break;
			case '-':
				res = AstValueInfo(builder.CreateFSub(lv.llvmValue, rv.llvmValue),
						doubleClass);
				break;
			case '*':
				res = AstValueInfo(builder.CreateFMul(lv.llvmValue, rv.llvmValue),
						doubleClass);
				break;
			case '/':
				res = AstValueInfo(builder.CreateFDiv(lv.llvmValue, rv.llvmValue),
						doubleClass);
				break;
			case EQUAL:
				res = AstValueInfo(builder.CreateFCmpOEQ(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case NEQUAL:
				res = AstValueInfo(builder.CreateFCmpONE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case '<':
				res = AstValueInfo(builder.CreateFCmpOLT(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case '>':
				res = AstValueInfo(builder.CreateFCmpOGT(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case LE:
				res = AstValueInfo(builder.CreateFCmpOLE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case GE:
				res = AstValueInfo(builder.CreateFCmpOGE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			}
		} else {
			switch (op) {
			case '+':
				res = AstValueInfo(builder.CreateAdd(lv.llvmValue, rv.llvmValue),
						lv.clazz);
				break;
			case '-':
				res = AstValueInfo(builder.CreateSub(lv.llvmValue, rv.llvmValue),
						lv.clazz);
				break;
			case '*':
				res = AstValueInfo(builder.CreateMul(lv.llvmValue, rv.llvmValue),
						lv.clazz);
				break;
			case '/':
				res = AstValueInfo(builder.CreateSDiv(lv.llvmValue, rv.llvmValue),
						lv.clazz);
				break;
						case '<':
				res = AstValueInfo(builder.CreateICmpSLT(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case '>':
				res = AstValueInfo(builder.CreateICmpSGT(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case LE:
				res = AstValueInfo(builder.CreateICmpSLE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case GE:
				res = AstValueInfo(builder.CreateICmpSGE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case EQUAL:
				res = AstValueInfo(builder.CreateICmpEQ(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			case NEQUAL:
				res = AstValueInfo(builder.CreateICmpNE(lv.llvmValue, rv.llvmValue),
						boolClass);
				break;
			}
		}
	}
	if (res.llvmValue == NULL) {
		errorMsg = "invalid operands to binary expression (" + lv.clazz->name
				+ " " + getOperatorName(op) + " " + rv.clazz->name + ")";
		throwError(this);
	}

	return res;
}


vector<AstValueInfo> FuncInvoke::multiCodeGen(AstContext &astContext) {
	AstValueInfo object;
	AstFunctionInfo funcV = astContext.getFunc(funcName);

	if (funcV.llvmFunc == NULL) {
		throwError(this);
	}

	vector<TypeInfo*> &argClasses = funcV.funcInfo->argClasses;
	if (exprList.size() < argClasses.size()) {
		errorMsg = "too few arguments to function '" + funcName + "''";
		throwError(this);
	} else if (exprList.size() > argClasses.size()) {
		errorMsg = "too many arguments to function '" + funcName + "'";
		throwError(this);
	}

	vector<AstValueInfo> argValues;
	vector<Value*> argllvmValues;

	for (unsigned i = 0; i < exprList.size(); i++) {
		exprList[i]->expectedType = argClasses[i];
		AstValueInfo v = exprList[i]->codeGen(astContext);
		argValues.push_back(v);
		argllvmValues.push_back(v.llvmValue);
	}

	Value *callResult = NULL;
	if (argValues.size() == 0) {
		callResult = builder.CreateCall(funcV.llvmFunc);
	} else {
		ArrayRef<Value*> args(argllvmValues);
		callResult = builder.CreateCall(funcV.llvmFunc, args);
	}

	vector<AstValueInfo> resultValues;
	vector<TypeInfo*> &resultClasses = funcV.funcInfo->returnClasses;
	if (resultClasses.size() == 0) {
		resultValues.push_back(AstValueInfo(NULL, voidClass));
	} else if (resultClasses.size() == 1) {
		resultValues.push_back(AstValueInfo(callResult, resultClasses[0]));
	} else {
		Value *alloc = createAlloca(funcV.funcInfo->returnType,
				astContext.allocBB);
		builder.CreateStore(callResult, alloc);
		for (unsigned i = 0; i < resultClasses.size(); i++) {
			Value *element = builder.CreateStructGEP(alloc, i);
			AstValueInfo v(builder.CreateLoad(element), resultClasses[i]);
			resultValues.push_back(v);
		}
	}
	return resultValues;
}

AstValueInfo FuncInvoke::gen(AstContext &astContext) {
	vector<AstValueInfo> resultValues = multiCodeGen(astContext);
	return resultValues[0];
}



AstValueInfo IdentExpr::lvalueGen(AstContext &astContext) {
	AstValueInfo var = astContext.getVar(ident);
	if (var.llvmValue == NULL) {
		throwError(this);
	}
	return var;
}

AstValueInfo IdentExpr::gen(AstContext &astContext) {
	AstValueInfo value = lvalueGen(astContext);
	value.llvmValue = builder.CreateLoad(value.llvmValue);
	return value;
}


AstValueInfo Int::gen(AstContext &astContext) {
	return AstValueInfo(ConstantInt::getSigned(int64Type, value), intClass);
}

AstValueInfo Char::gen(AstContext &astContext) {
	return AstValueInfo(ConstantInt::getSigned(int32Type, value), charClass);
}

AstValueInfo Double::gen(AstContext &astContext) {
	return AstValueInfo(ConstantFP::get(doubleType, value), doubleClass);
}

AstValueInfo Bool::gen(AstContext &astContext) {
	return AstValueInfo(builder.getInt1(value), boolClass);
}
