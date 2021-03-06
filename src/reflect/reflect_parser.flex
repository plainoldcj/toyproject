%option prefix="yyrfl"
%option noyywrap
%option yylineno

%{
#include "common/reflect.h"

#include "reflect/reflect_local.h"
%}

IDENT [a-zA-Z][a-zA-Z0-9_]*
INTEGER [0-9]+

REFLECTED "__REFLECTED__"
REFL_ATTRIB "__REFL_ATTRIB__"

%%

<<EOF>> { return TOK_EOF; }

[\t\n ]+ /* ignore whitespace */

","         	{ return TOK_COLON; }
";"         	{ return TOK_SEMICOL; }
"{"         	{ return TOK_LBRACE; }
"}"         	{ return TOK_RBRACE; }
"["				{ return TOK_LBRACKET; }
"]"				{ return TOK_RBRACKET; }
"("				{ return TOK_LPAREN; }
")"				{ return TOK_RPAREN; }

"struct"    	{ return TOK_STRUCT; }

"char"			{ g_primType = PT_CHAR; return TOK_PRIM; }
"float"     	{ g_primType = PT_FLOAT; return TOK_PRIM; }
"int"			{ g_primType = PT_INT; return TOK_PRIM; }
"uint8_t"		{ g_primType = PT_UINT8; return TOK_PRIM; }
"uint16_t"		{ g_primType = PT_UINT16; return TOK_PRIM; }

{REFLECTED}		{ return TOK_REFLECTED; }
{REFL_ATTRIB}	{ return TOK_REFL_ATTRIB; }

{IDENT}     	{ return TOK_IDENT; }

{INTEGER}		{ return TOK_INTEGER; }

.           	{ return TOK_UNKNOWN; }

%%

void Parse(const char* filename)
{
	yyrflin = fopen(filename, "r");
	StartParsing(filename);
}
