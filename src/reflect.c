#include "reflect_local.h"

#include "term.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

enum PrimitiveType	g_primType;

char*				yyrfltext;
int					yyrfllineno;

static int			s_token = TOK_UNKNOWN;
static const char*	s_filename;

#define FOR_TOKEN(x) #x,
static const char* s_tokenNames[] =
{
	FORALL_TOKENS
	NULL
};
#undef FOR_TOKEN

static void PrintUsage(const char* exeName)
{
	printf("Usage: %s infile ...\n", exeName);
}

int main(int argc, char* argv[])
{
	int i;

	if(argc < 2)
	{
		PrintUsage(argv[0]);
		return -1;
	}

	for(i = 1; i < argc; ++i)
	{
		printf( TERM_RED( "got file %s\n" ), argv[i]);
		Parse(argv[i]);
	}

	return 0;
}

static void NextToken(void)
{
	s_token = yyrfllex();
}

static void ExpectToken(int tok)
{
	assert(tok >= 0 && tok < TOKEN_COUNT);
	if(tok != s_token)
	{
		printf( TERM_RED( "Error: " ) "%s:%d: Expected token '%s', but got '%s' instead.\n",
			s_filename, yyrfllineno, s_tokenNames[tok], yyrfltext );
		exit(-1);
	}
}

static void ParseReflected(void)
{
	ExpectToken(TOK_PRIM);
}

void StartParsing(const char* filename)
{
	bool done = false;

	s_filename = filename;

	while(!done)
	{
		NextToken();
		if(s_token == TOK_EOF)
		{
			done = true;
		}
		else if(s_token == TOK_REFLECTED)
		{
			NextToken();
			ParseReflected();
		}
	}
}
