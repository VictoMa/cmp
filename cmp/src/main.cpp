#include <stdio.h>
#include <unistd.h>
#include <libgen.h>

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Value.h>
#include <llvm/Type.h>
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/IRBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/CodeGen/CommandFlags.h>

#include "../include/AstFunc.h"
#include "statement.h"
#include "expression.h"
#include "parser.hpp"

extern int yyparse();
extern FILE *yyin;

//全局上下文
LLVMContext &context = getGlobalContext();
//IR模块定义
Module module("myMod", context);
IRBuilder<> builder(context);
//数据层
DataLayout *dataLayout = NULL;
GlobalContext globalContext;

//变量类型指针定义
Type *int64Type = NULL;
Type *int32Type = NULL;
Type *doubleType = NULL;
Type *boolType = NULL;
Type *voidType = NULL;

//自定义类型
TypeInfo *intClass = NULL;
TypeInfo *charClass = NULL;
TypeInfo *doubleClass = NULL;
TypeInfo *boolClass = NULL;
TypeInfo *voidClass = NULL;

//常数指针
Constant *int64_0 = NULL;
Constant *int32_0 = NULL;
Constant *double_0 = NULL;
Constant *bool_true = NULL;
Constant *bool_false = NULL;

//main函数指针
Function *mainFunc = NULL;


string errorMsg;
Function *startFunc = NULL;
//Program类指针
Program *program = NULL;

//生成自定义函数库
static void createSystemFunctions();
//初始化全局变量
static void initGlobals();


//编译程序入口
int main(int argc, char **argv) {
	//定义参数-输出类型变量
	bool irOutput = false;
	bool asmOutput = false;
	bool objOutput = false;
	bool execOutput = false;
	TargetMachine::CodeGenFileType outputFileType = TargetMachine::CGFT_Null;
	char *outputFileName = NULL;
	//编译接收命令行
	int option;
	while ((option = getopt(argc, argv, "o:scS")) != -1) {
		switch (option) {
		case 'o':
			if (outputFileName != NULL) {
				cout << "warning: ignoring '-o " << optarg << "' because '-o "
						<< outputFileName << "' has set before" << endl;
			} else {
				outputFileName = optarg;//更改文件名称
			}
			break;
		case 's'://汇编代码.s
			asmOutput = true;
			break;
		case 'c'://目标代码.o
			objOutput = true;
			break;
		case 'S'://中间代码.ir
			irOutput = true;
			break;
		}
	}
	//类型冲突时给出错误信息
	if (irOutput) {
		if (asmOutput) {
			cout << "warning: ignoring '-s' because '-S' has set" << endl;
		}
		if (objOutput) {
			cout << "warning: ignoring '-c' because '-S' has set" << endl;
		}
	} else if (asmOutput) {
		if (objOutput) {
			cout << "warning: ignoring '-c' because '-s' has set" << endl;
		}
		outputFileType = TargetMachine::CGFT_AssemblyFile;
	} else if (objOutput) {
		outputFileType = TargetMachine::CGFT_ObjectFile;
	} else {
		outputFileType = TargetMachine::CGFT_ObjectFile;
		execOutput = true;
	}

	//找到输入文件
	char *inputFileName = NULL;
	for (; optind < argc; optind++) {
		if (inputFileName == NULL) {
			inputFileName = argv[optind];
		} else {
			cout << "warning: ignoring input file " << argv[optind] << endl;
		}
	}

	if (inputFileName != NULL) {
		yyin = fopen(inputFileName, "r");
		if (yyin == NULL) {
			cout << "can not open file '" << inputFileName << "'" << endl;
			exit(1);
		}
	}

	if (yyin == NULL) {
		cout << ">>" << endl;
	}
	//从yyin读入
	yyparse();

	if (yyin != NULL) {
		fclose(yyin);
	}
	//初始化全文
	initGlobals();
	//创建包装后的函数库
	createSystemFunctions();
	//自上而下生成代码
	program->codeGen();

	//初始化LLVM本地目标代码
	InitializeNativeTarget();
	InitializeAllTargets();
	InitializeAllTargetMCs();
	//初始化LLVM汇编代码生成
	InitializeAllAsmPrinters();
	InitializeAllAsmParsers();

	string opFileName;
	if (irOutput) {
		if (outputFileName == NULL) {
			if (inputFileName == NULL) {
				//默认ir
				opFileName = "IRcode.ir";
			} else {
				opFileName = string(basename(inputFileName)) + ".ir";
			}
		} else {
			opFileName = outputFileName;
		}
		string errorMsg;
		tool_output_file outputFile(opFileName.c_str(), errorMsg);
		if (!errorMsg.empty()) {
			cout << errorMsg << endl;
			return 1;
		}
		outputFile.os() << module;
		outputFile.keep();
	} else {
		string errorStr;
		const Target *target = TargetRegistry::lookupTarget(
				sys::getDefaultTargetTriple(), errorStr);
		if (target == NULL) {
			cout << errorStr << endl;
			return 1;
		}
		TargetOptions targetOptions;
		TargetMachine *targetMachine = target->createTargetMachine(
				sys::getDefaultTargetTriple(), sys::getHostCPUName(), "",
				targetOptions);

		if (outputFileName == NULL) {
			if (inputFileName == NULL) {
				if (asmOutput) {
					opFileName = "ASM.s";
				} else {
					opFileName = "OBJ.o";
				}
			} else {
				if (asmOutput) {
					opFileName = string(basename(inputFileName)) + ".s";
				} else {
					opFileName = string(basename(inputFileName)) + ".o";
				}
			}
		} else if (execOutput) {
			opFileName = string(outputFileName) + ".o";
		} else {
			opFileName = outputFileName;
		}
		string errorStr2;
		tool_output_file outputFile(opFileName.c_str(), errorStr2);
		if (!errorStr2.empty()) {
			cout << errorStr2 << endl;
			return 1;
		}
		//块处理
		PassManager passManager;
		passManager.add(dataLayout);
		//处理汇编代码
		formatted_raw_ostream fos(outputFile.os());
		targetMachine->addPassesToEmitFile(passManager, fos, outputFileType);
		passManager.run(module);
		outputFile.keep();
	}
	//生成可执行程序
	if (execOutput) {
		//给出链接库
		string sysapi = string(dirname(argv[0])) + "/lib/io.o ";
		if (outputFileName == NULL) {
			outputFileName = "a.out";
		}
		//执行两个shell命令
		//将sysapi与gcc编译出的LLVM.obj链接
		string command = "gcc " + sysapi + opFileName + " -o " + outputFileName;
		int status = system(command.c_str());
		//删除中间文件
		command = "rm " + opFileName;
		system(command.c_str());
		cout << "Compile Finished -MT"<<endl;
		return status;
	}
	return 0;
}


void initGlobals() {
	//不可变块,提供编译信息
	dataLayout = new DataLayout(&module);
	//类型与LLVM接口
	int64Type = builder.getInt64Ty();
	int32Type = builder.getInt32Ty();
	doubleType = builder.getDoubleTy();
	boolType = builder.getInt1Ty();
	voidType = builder.getVoidTy();
	//常量
	int64_0 = ConstantInt::getSigned(int64Type, 0);
	int32_0 = ConstantInt::getSigned(int32Type, 0);
	double_0 = ConstantFP::get(doubleType, 0);
	bool_true = builder.getInt1(true);
	bool_false = builder.getInt1(false);

	//创建类buffer
	intClass = new TypeInfo("int", int64Type);
	charClass = new TypeInfo("char", int32Type);
	doubleClass = new TypeInfo("double", doubleType);
	boolClass = new TypeInfo("bool", boolType);
	voidClass = new TypeInfo("void", NULL);

	//在全文中添加类
	globalContext.addClass(intClass);
	globalContext.addClass(charClass);
	globalContext.addClass(doubleClass);
	globalContext.addClass(boolClass);
	globalContext.addClass(voidClass);
}



void createSystemFunctions() {
	vector<Type*> argllvmTypes;
	vector<TypeInfo*> argClasses;
	vector<TypeInfo*> emptyClasses;
	FunctionType *funcType = NULL;
	Constant *func = NULL;

	//create print int func
	argllvmTypes.push_back(int64Type);
	argClasses.push_back(intClass);
	funcType = FunctionType::get(voidType, ArrayRef<Type*>(argllvmTypes),
			false);
	func = module.getOrInsertFunction("printI", funcType);
	FunctionInfo *printI = new FunctionInfo("printI", (Function*) func,
			emptyClasses, argClasses);


	argllvmTypes.clear();
	argClasses.clear();
	argllvmTypes.push_back(doubleType);
	argClasses.push_back(doubleClass);
	funcType = FunctionType::get(voidType, ArrayRef<Type*>(argllvmTypes),
			false);
	func = module.getOrInsertFunction("printD", funcType);
	FunctionInfo *printD = new FunctionInfo("printD", (Function*) func,
			emptyClasses, argClasses);


	//create println func
	argllvmTypes.clear();
	argClasses.clear();
	funcType = FunctionType::get(voidType, false);
	func = module.getOrInsertFunction("println", funcType);
	FunctionInfo *println = new FunctionInfo("println", (Function*) func,
			emptyClasses, emptyClasses);



	

	globalContext.addFunction(printI);
	globalContext.addFunction(printD);
	globalContext.addFunction(println);

}




