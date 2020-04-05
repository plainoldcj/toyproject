#pragma once

#include <stdint.h>

struct SScope;

void COM_Init(void);
void COM_Deinit(void);

void COM_LogPrintf(const char* format, ...);

/*
==================================================
BigStack
==================================================
*/

void	BigStack_Begin(struct SScope* scope);
void	BigStack_End(struct SScope* scope);

void*	BigStack_Alloc(uint32_t size);
