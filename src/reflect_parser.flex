%option prefix="yyrfl"
%option noyywrap
%option yylineno

%{
#include "reflect.h"
#include "reflect_local.h"
%}

IDENT [a-zA-Z][a-zA-Z0-9]*

REFLECTED "__REFLECTED__"

%%

<<EOF>> { return TOK_EOF; }

[\t\n ]+ /* ignore whitespace */

";"         { return TOK_SEMICOL; }
"{"         { return TOK_LBRACE; }
"}"         { return TOK_RBRACE; }

"struct"    { return TOK_STRUCT; }

"float"     { g_primType = PT_FLOAT; return TOK_PRIM; }

{REFLECTED}	{ return TOK_REFLECTED; }

{IDENT}     { return TOK_IDENT; }

.           { return TOK_UNKNOWN; }

%%

void Parse(const char* filename)
{
	yyrflin = fopen(filename, "r");
	StartParsing(filename);
}
