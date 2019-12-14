#pragma once

#define CB_ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

void COM_Init(void);

void COM_LogPrintf(const char* format, ...);
