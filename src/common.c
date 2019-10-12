#include "common.h"

#include <stdarg.h>
#include <stdio.h>

static FILE* s_logfile;

void COM_Init(void)
{
	s_logfile = fopen("logfile.txt", "wt");
	if(!s_logfile)
	{
		printf("Unable to open logfile.\n");
	}
}

void COM_LogPrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(s_logfile, format, args);
	fprintf(s_logfile, "\n");
	fflush(s_logfile);
	va_end(args);
}
