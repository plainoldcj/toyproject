#include "reflect_local.h"

#include "term.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define MAX_IDENT_LEN 512

struct Variable
{
	struct Variable*	next;

	char				ident[MAX_IDENT_LEN];
	enum PrimitiveType	primType;
	bool				prim;
};

struct Type
{
	struct Type*		next;

	char				ident[MAX_IDENT_LEN];
	struct Variable*	variables;
};

struct Type* s_types;

#define MEMORY_SIZE 4096

static struct
{
	char* cur;
	char* end;
} s_mem;

static void* Alloc(size_t size)
{
	s_mem.cur += size;
	if(s_mem.cur > s_mem.end)
	{
		printf( TERM_RED( "Error: ") "Out of memory.\n" );
		exit(-1);
	}
	return s_mem.cur;
}

static void StoreIdentifier(char* dst)
{
	if(strlen(yyrfltext) >= MAX_IDENT_LEN)
	{
		printf( TERM_RED( "Error: " ) "Identifier '%s' is too long.\n",
			yyrfltext );
		exit(-1);
	}
	strcpy(dst, yyrfltext);
}

static void PrintUsage(const char* exeName)
{
	printf("Usage: %s outfile infile ...\n", exeName);
}

static void WriteOutput(const char* filename)
{
	FILE* file = fopen(filename, "w");

	struct Type* type = s_types;
	while(type)
	{
		fprintf(file, "%s\n", type->ident);

		struct Variable* var = type->variables;
		while(var)
		{
			fprintf(file, "%s\n", var->ident);

			var = var->next;
		}

		type = type->next;
	}

	fclose(file);
}

int main(int argc, char* argv[])
{
	int i;

	if(argc < 3)
	{
		PrintUsage(argv[0]);
		return -1;
	}

	s_mem.cur = malloc(MEMORY_SIZE);
	s_mem.end = s_mem.cur + MEMORY_SIZE;

	for(i = 2; i < argc; ++i)
	{
		printf( TERM_RED( "got file %s\n" ), argv[i]);
		Parse(argv[i]);
	}

	WriteOutput(argv[1]);

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

static void UnexpectedToken(void)
{
	printf( TERM_RED( "Error: " ) "%s:%d: Unexpected token '%s'.\n",
			s_filename, yyrfllineno, yyrfltext );
	exit(-1);
}

static void ParseVariable(struct Type* type)
{
	struct Variable* var = Alloc(sizeof(struct Variable));

	var->prim = true;
	var->primType = g_primType;

	var->next = type->variables;
	type->variables = var;

	NextToken();
	ExpectToken(TOK_IDENT);

	StoreIdentifier(var->ident);

	NextToken();
	ExpectToken(TOK_SEMICOL);
}

static void ParseType(void)
{
	bool done;

	NextToken();
	ExpectToken(TOK_STRUCT);

	NextToken();
	ExpectToken(TOK_IDENT);

	struct Type* type = Alloc(sizeof(struct Type));

	type->next = s_types;
	s_types = type;

	StoreIdentifier(type->ident);

	type->variables = NULL;

	NextToken();
	ExpectToken(TOK_LBRACE);

	done = false;
	while(!done)
	{
		NextToken();
		if(s_token == TOK_PRIM)
		{
			ParseVariable(type);
		}
		else if(s_token == TOK_RBRACE)
		{
			done = true;
		}
		else
		{
			UnexpectedToken();
		}
	}

	NextToken();
	ExpectToken(TOK_SEMICOL);
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
			ParseType();
		}
	}
}
