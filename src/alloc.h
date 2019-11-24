#pragma once

#include <stdint.h>

struct FLNode;

struct Chunk
{
	void* mem;
	uint32_t size;
};

struct FLAlloc
{
	char* mem;
	struct FLNode* freeList;
	uint32_t size;
};

void FLAlloc_Init(struct FLAlloc* alloc, void* mem, uint32_t size);
struct Chunk FLAlloc_Alloc(struct FLAlloc* alloc, uint32_t size);
void FL_Free(struct FLAlloc* alloc, struct Chunk chunk);
void FL_FreeZero(struct FLAlloc* alloc, struct Chunk* chunk);
