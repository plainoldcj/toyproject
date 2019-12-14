#pragma once

/* Tokens */
#define FORALL_TOKENS\
	FOR_TOKEN(TOK_UNKNOWN)\
	FOR_TOKEN(TOK_EOF)\
	FOR_TOKEN(TOK_SEMICOL)\
	FOR_TOKEN(TOK_LBRACE)\
	FOR_TOKEN(TOK_RBRACE)\
	FOR_TOKEN(TOK_STRUCT)\
	FOR_TOKEN(TOK_IDENT)\
	FOR_TOKEN(TOK_PRIM)\
	FOR_TOKEN(TOK_REFLECTED)

#define FOR_TOKEN(x) x,
enum
{
	FORALL_TOKENS

	TOKEN_COUNT
};
#undef FOR_TOKEN

struct ReflectedType;

extern int						g_primType; /* enum PrimitiveType */

extern char*					yyrfltext;
extern int						yyrfllineno;

int		yyrfllex(void);

void	Parse(const char* filename);
void	StartParsing(const char* filename);
