%{
#include <stdio.h>
#include "statement.h"
#include "expression.h"
#include "parser.hpp"

using namespace std;
extern int yylex();
extern int yylineno;
extern int charno;
extern int yyleng;
extern FILE *yyin;
extern Program *program;

/*给出出错的行列信息*/
void yyerror(const char *msg){
	cout<<yylineno<<":"<<(charno-yyleng)<<": error: "<<msg<<endl;
	if(yyin != NULL){
		fclose(yyin);
	}
	exit(1);
}


void setLocation(AstNode *node,YYLTYPE *loc){
	loc->first_line = yylineno;
	loc->first_column = charno;

	if(node != NULL){
		node->firstLine = loc->first_line;
		node->firstColumn = loc->first_line;

	}
}
void setLocation(AstNode *node,YYLTYPE *loc,YYLTYPE *firstLoc,YYLTYPE *lastLoc){
	loc->first_line = firstLoc->first_line;
	loc->first_column = firstLoc->first_column;

	if(node != NULL){
		node->firstLine = loc->first_line;
		node->firstColumn = loc->first_column;

	}
}



void setNodeLocation(AstNode *node,YYLTYPE *loc){
	node->firstLine = loc->first_line;
	node->firstColumn = loc->first_line;
}

%}
%error-verbose
%debug

%union{
	int token;
	string *str;

	int64_t IntValue;
	double doubleValue;
	int32_t charValue;
	wstring *wstr;

	Program *program;
	TypeDecl *typeDecl;
	SimpleVarDecl *simpleVarDecl;
	StmtBlock *stmtBlock;	

	Statement* stmt;
	VarDef *varDef;
	VarInit *varInit;
	FuncDef *funcDef;
	FuncDecl *funcDecl;
	SimpleStmtList *spStmtList;

	Expression *exp;
	LeftValueExpr *leftValueExpr;
	FuncInvoke *funcInvoke;
	
	vector<TypeDecl*> *typeDeclList;
	vector<SimpleVarDecl*> *spvarDeclList;
	vector<Statement*> *stmtList;
	vector<VarInit*> *varInitList;
	vector<Expression*> *exprList;
}


%token <IntValue> INT
%token <doubleValue> DOUBLE
%token <charValue> CHAR
%token <str> IDENT ERROR
%token <token>  FOR IF ELSE WHILE  
%token <token> RETURN BREAK CONTINUE TRUE FALSE VOID 

%token <token>  AND OR EQUAL NEQUAL LE GE ADD_AS SUB_AS ADD_SEL SUB_SEL

%token <token> READ WRITE

%type <program> program def_list
%type <typeDecl> type_decl
%type <typeDeclList> type_decl_list
%type <funcDef> func_def
%type <funcDecl> func_decl
%type <funcInvoke> func_invoke
%type <simpleVarDecl> simple_var_decl
%type <spvarDeclList> spvar_decl_list
//statement
%type <spStmtList> single_stmt_list
%type <stmtList> stmt_list
%type <stmtBlock> stmt_block
%type <stmt> stmt single_stmt var_assi return_stmt
%type <stmt> if_stmt for_stmt for_init for_foot while_stmt 
%type <stmt> read_stmt write_stmt
//variables
%type <varDef> var_def
%type <varInit> var_init
%type <varInitList> var_init_list

//expression
%type <exp> for_cond exp numeric bool while_cond
%type <exprList> expr_list
%type <leftValueExpr> ident_expr left_expr


%left OR
%left AND
%left EQUAL NEQUAL
%left '<' '>' LE GE
%left '+' '-' ADD_AS SUB_AS	/*HERE*/
%left '*' '/' '%'
%left ADD_SEL SUB_SEL
%nonassoc UMINUS
%nonassoc LOGICNOT

%start program

%%

/************************** program ***************************/
program:
	def_list {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
		program = $1;
	};

def_list:
	/*blank*/ {
		$$ = new Program();
		setLocation(NULL,&@$);
	}
	|var_def ';'{
		$$ = new Program();
		$$->addVarDef($1);
		setLocation(NULL,&@$,&@1,&@2);
	}
	|func_def {
		$$ = new Program();
		$$->addFuncDef($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|def_list var_def ';'{
		$1->addVarDef($2);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@3);
	}
	|def_list func_def {
		$1->addFuncDef($2);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@2);
	};
	


simple_var_decl:
	type_decl IDENT {
		$$ = new SimpleVarDecl($1,*$2);
		setLocation($$,&@$,&@1,&@2);
		delete $2;
	};
spvar_decl_list:
	/*blank*/ {
		$$ = new vector<SimpleVarDecl*>();
		setLocation(NULL,&@$);
	}
	|simple_var_decl {
		$$ = new vector<SimpleVarDecl*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);}
	|spvar_decl_list ',' simple_var_decl {
		$1->push_back($3);
		$$ = $1; 
		setLocation(NULL,&@$,&@1,&@3);
	};

type_decl:
	IDENT {
		$$ = new TypeDecl(*$1);
		setLocation($$,&@$,&@1,&@1);
		delete $1;
	};
type_decl_list:
	type_decl {
		$$ = new vector<TypeDecl*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|type_decl_list ',' type_decl {
		$$ = $1;
		$$->push_back($3);
		setLocation(NULL,&@$,&@1,&@3);
	};

func_def:
	func_decl stmt_block {
		$$ = new FuncDef($1,$2);
		setLocation(NULL,&@$,&@1,&@2);
	};
func_decl:
	type_decl IDENT '(' spvar_decl_list ')' {
		vector<TypeDecl*> retTypes;
		retTypes.push_back($1);
		$$ = new FuncDecl(retTypes,*$2,*$4);
		setLocation($$,&@$,&@1,&@5);
		delete $2;
		delete $4;
	}
	|VOID IDENT '(' spvar_decl_list ')' {
		vector<TypeDecl*> retTypes;
		$$ = new FuncDecl(retTypes,*$2,*$4);
		setLocation($$,&@$,&@1,&@5);
		delete $2;
		delete $4;
	}
	|type_decl_list IDENT '(' spvar_decl_list ')' {
		$$ = new FuncDecl(*$1,*$2,*$4);
		setLocation($$,&@$,&@1,&@5);
		delete $1;
		delete $2;
		delete $4;
	};

/*********************** var ************************/
var_def:
	type_decl var_init_list {
		$$ = new VarDef($1,*$2);
		setLocation($$,&@$,&@1,&@2);
		delete $2;
	};
var_init:
	IDENT {
		$$ = new VarInit(*$1,NULL);
		setLocation($$,&@$,&@1,&@1);
		delete $1;
	}
	|IDENT '=' exp {
		$$ = new VarInit(*$1,$3);
		setLocation($$,&@$,&@1,&@3);
		delete $1;
	};
var_init_list:
	var_init {
		$$ = new vector<VarInit*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|var_init_list ',' var_init {
		$1->push_back($3);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@3);
	};
/************************** statement ***************************/
stmt_list:
	/*blank*/ {
		$$ = new vector<Statement*>();
		setLocation(NULL,&@$);
	}
	|stmt {
		$$ = new vector<Statement*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|stmt_list stmt {
		$1->push_back($2);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@2);
	};
stmt:
	';' {
		$$ = new NullStmt();
		setLocation($$,&@$,&@1,&@1);
	}
	|var_def ';' {
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	}
	|single_stmt_list ';' {
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	}
	|for_stmt {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|while_stmt {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|if_stmt {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|read_stmt {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|write_stmt {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|return_stmt ';' {
		$$ = $1;
		setLocation($$,&@$,&@1,&@2);
	}
	|BREAK ';' {
		$$ = new BreakStmt();
		setLocation($$,&@$,&@1,&@2);
	}
	|CONTINUE ';' {
		$$ = new ContinueStmt();
		setLocation($$,&@$,&@1,&@2);
	}
	|stmt_block {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};
	single_stmt:
	var_assi {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|exp {
		$$ = new ExprStmt($1);
		setLocation($$,&@$,&@1,&@1);
	}
/*HERE*/
	|left_expr ADD_SEL {
		$$ = new IncStmt($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|left_expr SUB_SEL {
		$$ = new IncStmt($1,false);
		setLocation($$,&@$,&@1,&@1);
	};
single_stmt_list:
	single_stmt {
		$$ = new SimpleStmtList();
		$$->add($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|single_stmt_list ',' single_stmt {
		$1->add($3);
		$$ = $1;
		setLocation($$,&@$,&@1,&@3);
	};

if_stmt:
	IF '(' exp ')' stmt ELSE stmt {
		$$ = new IfElseStmt($3,$5,$7);
		setLocation($$,&@$,&@1,&@7);
	}
	|IF '(' exp ')' stmt {
		$$ = new IfElseStmt($3,$5,NULL);
		setLocation($$,&@$,&@1,&@5);
	};

read_stmt:
	READ '('left_expr','exp')' ';'{
	};

write_stmt:
	WRITE'('exp')' ';'{
	};

for_stmt:
	FOR '(' for_init ';' for_cond ';' for_foot ')' stmt {
		$$ = new ForStmt($3,$5,$7,$9);
		setLocation($$,&@$,&@1,&@9);
	};
for_init:
	/*blank*/ {
		$$ = new NullStmt();
		setLocation($$,&@$);
	}
	|var_def {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|single_stmt_list {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};
for_cond:
	/*blank*/ {
		$$ = new Bool(true);
		setLocation($$,&@$);
	}
	|exp {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};
for_foot:
	/*blank*/ {
		$$ = new NullStmt();
		setLocation($$,&@$);
	}
	|single_stmt_list {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};
/*HERE*/
while_stmt:WHILE '(' while_cond ')' stmt{
                 $$ = new WhileStmt($3,$5);
                 setLocation($$,&@$,&@1,&@5);
         };
while_cond:
         /*blank*/ {
                 $$ = new Bool(true);
                 setLocation($$,&@$);
         }
         |exp {
                 $$ = $1;
                 setLocation($$,&@$,&@1,&@1);
         };

return_stmt:
	RETURN expr_list {
		$$ = new ReturnStmt(*$2);
		setLocation($$,&@$,&@1,&@2);
		delete $2;
	};
	
stmt_block:
	'{' stmt_list '}' {
		$$ = new StmtBlock(*$2);
		setLocation(NULL,&@$,&@1,&@3);
		delete $2;
	};

var_assi:
	left_expr '=' exp {
		$$ = new VarAssi($1,$3);
		setLocation($$,&@$,&@1,&@3);
	}
/*HERE*/|left_expr ADD_AS exp {
		$$ = new VarAssi($1,(new BinaryOpExpr($1,'+',$3)));
	}
	|left_expr SUB_AS exp {
		$$ = new VarAssi($1,(new BinaryOpExpr($1,'-',$3)));
	}
		
	


/*********************** expression ************************/
	
expr_list:
	/*blank*/ {
		$$ = new vector<Expression*>();
		setLocation(NULL,&@$);
	}
	|exp {
		$$ = new vector<Expression*>();
		$$->push_back($1);
		setLocation(NULL,&@$,&@1,&@1);
	}
	|expr_list ',' exp {
		$1->push_back($3);
		$$ = $1;
		setLocation(NULL,&@$,&@1,&@3);
	};
	
exp:
	exp '+' exp {
		$$ = new BinaryOpExpr($1,'+',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp '-' exp {
		$$ = new BinaryOpExpr($1,'-',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp '*' exp {
		$$ = new BinaryOpExpr($1,'*',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp '/' exp {
		$$ = new BinaryOpExpr($1,'/',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp '%' exp {
		$$ = new BinaryOpExpr($1,'%',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp EQUAL exp {
		$$ = new BinaryOpExpr($1,EQUAL,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp NEQUAL exp {
		$$ = new BinaryOpExpr($1,NEQUAL,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp LE exp {
		$$ = new BinaryOpExpr($1,LE,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp GE exp {
		$$ = new BinaryOpExpr($1,GE,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp '<' exp {
		$$ = new BinaryOpExpr($1,'<',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp '>' exp {
		$$ = new BinaryOpExpr($1,'>',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp AND exp {
		$$ = new BinaryLogicExpr($1,AND,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|exp OR exp {
		$$ = new BinaryLogicExpr($1,OR,$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|'(' exp ')' {
		$$ = $2;
		setLocation($$,&@$,&@1,&@3);
	}
	|'(' IDENT ')' {
		$$ = new IdentExpr(*$2);
		setLocation($$,&@$,&@1,&@3);
		delete $2;
	}
	|'-' exp %prec UMINUS {
		$$ = new PrefixOpExpr('-',$2);
		setLocation($$,&@$,&@1,&@2);
	}
	|'!' exp %prec LOGICNOT {
		$$ = new PrefixOpExpr('!',$2);
		setLocation($$,&@$,&@1,&@2);
	}
	|left_expr {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|left_expr '+' exp {
		$$ = new BinaryOpExpr($1,'+',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|left_expr '-' exp {
		$$ = new BinaryOpExpr($1,'-',$3);
		setLocation($$,&@$,&@1,&@3);
	}
	|numeric {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|bool {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	}
	|func_invoke {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};

left_expr:
	ident_expr {
		$$ = $1;
		setLocation($$,&@$,&@1,&@1);
	};

ident_expr:
	IDENT {
		$$ = new IdentExpr(*$1);
		setLocation($$,&@$,&@1,&@1);
		delete $1;
	};
	
numeric:
	INT {
		$$ = new Int($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|DOUBLE {
		$$ = new Double($1);
		setLocation($$,&@$,&@1,&@1);
	}
	|CHAR {
		$$ = new Char($1);
		setLocation($$,&@$,&@1,&@1);
	};

bool:
	TRUE {
		$$ = new Bool(true);
		setLocation($$,&@$,&@1,&@1);
	}
	|FALSE {
		$$ = new Bool(false);
		setLocation($$,&@$,&@1,&@1);
	};


/*********************Other**********************/
func_invoke:
	IDENT '(' expr_list ')' {
		$$ = new FuncInvoke(NULL,*$1,*$3);
		setLocation($$,&@$,&@1,&@4);
		delete $1;
		delete $3;
	};

%%
