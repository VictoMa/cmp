%{
#include <string>
#include <iostream>
#include <stdint.h>
#include "share.h"
#include "parser.hpp"

using namespace std;

extern "C" int yywrap(){}
extern void yyerror(const char *msg);

int charno = 1;
wstring strBuf;

void saveLocation(){
	yylloc.first_line = yylineno;
	yylloc.first_column = charno;
	charno += yyleng;
}



int32_t getWchar(){
	if(strBuf.size() == 0){
		yyerror("empty character constant");
	}else if(strBuf.size() > 1){
		yyerror("multi-character in char syntax");
	}
	return strBuf[0];
}

%}
%option yylineno

%x CMNT CHARSTATUS STRINGSTATUS
%%
[\t ]*			{saveLocation();}
\n				{charno = 1;}
\/\/.*\n		{charno = 1;}
"/*"			{saveLocation(); BEGIN CMNT;}
<CMNT>.			{saveLocation();}
<CMNT>\n		{charno = 1;}
<CMNT>"*/"		{saveLocation(); BEGIN INITIAL;}

'				{saveLocation(); BEGIN CHARSTATUS; strBuf.clear();}

<CHARSTATUS>\n	{yyerror("missing terminating ' character");}
<CHARSTATUS>'	{saveLocation(); BEGIN INITIAL; yylval.charValue = getWchar(); return CHAR;}
<CHARSTATUS>.	{saveLocation(); strBuf.push_back(yytext[0]);}
<CHARSTATUS>\\	{saveLocation(); }




"void"	{saveLocation(); yylval.token = VOID; return VOID;}
"for"	{saveLocation(); yylval.token = FOR; return FOR;}
"while"	{saveLocation(); yylval.token = WHILE; return WHILE;} /*HERE*/
"if"	{saveLocation(); yylval.token = IF; return IF;}
"else"	{saveLocation(); yylval.token = ELSE; return ELSE;}
"break"	{saveLocation(); yylval.token = BREAK; return BREAK;}
"continue"	{saveLocation(); yylval.token = CONTINUE; return CONTINUE;}
"return"	{saveLocation(); yylval.token = RETURN; return RETURN;}
"true"	{saveLocation(); yylval.token = TRUE; return TRUE;}
"false"	{saveLocation(); yylval.token = FALSE; return FALSE;}




[a-zA-Z_][a-zA-Z0-9_]*	{
	saveLocation();
	yylval.str = new string(yytext,yyleng);
	return IDENT;
}
([0-9]+\.[0-9]+)	{
	saveLocation();
	string tempStr(yytext,yyleng);
	yylval.doubleValue = atof(tempStr.c_str());
	return DOUBLE;
}
[0-9]+	{
	saveLocation();
	string tempStr(yytext,yyleng);
	yylval.IntValue = atoi(tempStr.c_str());
	return INT;
}

"&&"	{saveLocation(); yylval.token = AND; return AND;}
"||"	{saveLocation(); yylval.token = OR; return OR;}
"!="	{saveLocation(); yylval.token = NEQUAL; return NEQUAL;}
"=="	{saveLocation(); yylval.token = EQUAL; return EQUAL;}
"<="	{saveLocation(); yylval.token = LE; return LE;}
">="	{saveLocation(); yylval.token = GE; return GE;}
"++"	{saveLocation(); return ADD_SEL;}	/*HERE*/
"+="	{saveLocation(); return ADD_AS;}
"--"	{saveLocation(); return SUB_SEL;}
"-="	{saveLocation(); return SUB_AS;}

"="	{saveLocation(); return '=';}
"("	{saveLocation(); return '(';}
")"	{saveLocation(); return ')';}
"+"	{saveLocation(); return '+';}
"-"	{saveLocation(); return '-';}
"*"	{saveLocation(); return '*';}
"/"	{saveLocation(); return '/';}
"%"	{saveLocation(); return '%';}
";"	{saveLocation(); return ';';}
"}"	{saveLocation(); return '}';}
"{"	{saveLocation(); return '{';}
","	{saveLocation(); return ',';}
"<"	{saveLocation(); return '<';}
">"	{saveLocation(); return '>';}
"!"	{saveLocation(); return '!';}


.	{saveLocation(); return ERROR;}
%%
