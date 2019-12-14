#pragma once

#include <stdint.h>

/*
==================================================
Freelist Allocator
==================================================
*/

struct Chunk
{
	void* mem;
	uint32_t size;
};

struct FLNode;

struct FLAlloc
{
	struct FLNode* freeList;
	char* mem;
	uint32_t size;
};

void FL_Init(struct FLAlloc* alloc, void* mem, uint32_t size);

struct Chunk FL_Alloc(struct FLAlloc* alloc, uint32_t size);

void FL_Free(struct FLAlloc* alloc, struct Chunk chunk);
void FL_FreeZero(struct FLAlloc* alloc, struct Chunk* chunk);

/*
==================================================
Pool Allocator
==================================================
*/

struct PNode;

struct PAlloc
{
	struct PNode* freeList;
	char* mem;
	uint32_t size;
	uint32_t objSize;
};

void P_Init(struct PAlloc* alloc, void* mem, uint32_t size, uint32_t objSize);

void* P_Alloc(struct PAlloc* alloc);

void P_Free(struct PAlloc* alloc, void* mem);

