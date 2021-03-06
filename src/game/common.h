#pragma once

#include <stdint.h>

#define KQ_ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

// TODO(cj): This is gcc specific.
#ifdef __GNUC__
#define KQ_STATIC_ASSERT(cond) _Static_assert(cond, "no message")
#else
#define KQ_STATIC_ASSERT(cond) assert(cond)
#endif

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
