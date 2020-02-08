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

/*
==================================================
Stack Allocator
==================================================
*/

struct SScope
{
	struct SScope*	next;
	char*			memRestore;
};

struct SAlloc
{
	char*			mem;
	uint32_t		size;
	struct SScope*	head;
	const char*		debugName; // TODO(cj): Might not be a literal
};

void	SA_Init(struct SAlloc* alloc, void* mem, uint32_t size, const char* debugName);
void	SA_Deinit(struct SAlloc* alloc);

void	SA_BeginScope(struct SAlloc* alloc, struct SScope* scope);
void	SA_EndScope(struct SAlloc* alloc, struct SScope* scope);

void*	SA_Alloc(struct SAlloc* alloc, uint32_t size);
