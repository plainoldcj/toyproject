#include "alloc.h"
#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

static struct
{
	FILE*			logfile;
	struct SAlloc	bigStack;
} s_com;

void COM_Init(void)
{
	// There is no corresponding fclose. We let the OS clean up the file handle on exit.
	s_com.logfile = fopen("logfile.txt", "wt");
	if(!s_com.logfile)
	{
		printf("Unable to open logfile.\n");
	}

	const uint32_t BIG_STACK_SIZE = 4000000; // TODO(cj): Do not malloc, get the memory from somewhere else.
	SA_Init(&s_com.bigStack, malloc(BIG_STACK_SIZE), BIG_STACK_SIZE, "Big Stack");
}

void COM_Deinit(void)
{
	SA_Deinit(&s_com.bigStack);
}

void COM_LogPrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(s_com.logfile, format, args);
	fprintf(s_com.logfile, "\n");
	fflush(s_com.logfile);
	va_end(args);
}

void BigStack_Begin(struct SScope* scope)
{
	SA_BeginScope(&s_com.bigStack, scope);
}

void BigStack_End(struct SScope* scope)
{
	SA_EndScope(&s_com.bigStack, scope);
}

void* BigStack_Alloc(uint32_t size)
{
	return SA_Alloc(&s_com.bigStack, size);
}
