%option prefix="yyrfl"
%option noyywrap

%{
#include "reflect_parser.h"
%}

%%

%%

void Reflect_Parse(const char* filename)
{
	yyrflin = fopen(filename, "r");
}
