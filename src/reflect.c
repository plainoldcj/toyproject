#include "reflect_parser.h"

#include <stdio.h>

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
		printf("got file %s\n", argv[i]);
	}

	return 0;
}
