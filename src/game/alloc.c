#include "alloc.h"

#include <assert.h>
#include <stdlib.h>

/*
==================================================
Freelist Allocator
==================================================
*/

struct FLNode
{
	struct FLNode* next;
	uint32_t size;
};

void FL_Init(struct FLAlloc* alloc, void* mem, uint32_t size)
{
	assert(size > sizeof(struct FLNode));

	struct FLNode* node = mem;
	node->next = NULL;
	node->size = size;

	alloc->freeList = node;
	alloc->mem = mem;
	alloc->size = size;
}

struct Chunk FL_Alloc(struct FLAlloc* alloc, uint32_t size)
{
	// TODO(cj): Size must be rounded upwards to that we can store
	// a FLNode inside the allocated memory.

	struct Chunk chunk;

	struct FLNode** pprev = &alloc->freeList;
	struct FLNode* it = alloc->freeList;
	while(it)
	{
		if(it->size >= size)
		{
			uint32_t restSize = it->size - size;
			if( restSize > sizeof(struct FLNode*) )
			{
				struct FLNode* newNode = (struct FLNode*)((char*)it + size);
				newNode->next = it->next;
				newNode->size = restSize;

				*pprev = newNode;
			}
			else
			{
				*pprev = it->next;
			}

			chunk.mem = it;
			chunk.size = size;
			return chunk;
		}
		pprev = &it->next;
		it = it->next;
	}

	chunk.mem = NULL;
	chunk.size = 0;
	return chunk;
}

void FL_Free(struct FLAlloc* alloc, struct Chunk chunk)
{
	assert((char*)chunk.mem >= alloc->mem && (char*)chunk.mem < (alloc->mem + alloc->size));

	struct FLNode* newNode = chunk.mem;
	newNode->size = chunk.size;

	struct FLNode** pprev = &alloc->freeList;
	struct FLNode* it = alloc->freeList;
	struct FLNode* prev = NULL;
	while(it && (it <= newNode))
	{
		prev = it;
		pprev = &it->next;
		it = it->next;
	}

	*pprev = newNode;
	newNode->next = it;

	struct FLNode* base = prev ? prev : newNode;
	it = base->next;
	while(it)
	{
		void* end = (char*)base + base->size;
		if(end == it)
		{
			base->size += it->size;
			base->next = it->next;
		}
		it = it->next;
	}
}

void FL_FreeZero(struct FLAlloc* alloc, struct Chunk* chunk)
{
	FL_Free(alloc, *chunk);

	chunk->mem = NULL;
	chunk->size = 0;
}

/*
==================================================
Pool Allocator
==================================================
*/

struct PNode
{
	struct PNode* next;
};

void P_Init(struct PAlloc* alloc, void* mem, uint32_t size, uint32_t objSize)
{
	assert(size >= sizeof(struct PNode));
	assert(objSize >= sizeof(struct PNode));
	assert(objSize <= size);

	alloc->freeList = NULL;
	alloc->mem = mem;
	alloc->size = size;
	alloc->objSize = objSize;

	char* it = mem;
	char* end = it + size;

	while(it < end)
	{
		struct PNode* newNode = (struct PNode*)it;
		newNode->next = alloc->freeList;
		alloc->freeList = newNode;

		it += objSize;
	}
}

void* P_Alloc(struct PAlloc* alloc)
{
	struct PNode* it = alloc->freeList;
	alloc->freeList = alloc->freeList->next;
	return it;
}

void P_Free(struct PAlloc* alloc, void* mem)
{
	struct PNode* newNode = (struct PNode*)mem;
	newNode->next = alloc->freeList;
	alloc->freeList = newNode;
}

/*
==================================================
Stack Allocator
==================================================
*/

// TODO(cj): More error handling. Out of memory?
// TODO(cj): Tests!

void SA_Init(struct SAlloc* alloc, void* mem, uint32_t size, const char* debugName)
{
	alloc->mem = mem;
	alloc->size = size;
	alloc->head = 0;
	alloc->debugName = debugName;
}

void SA_Deinit(struct SAlloc* alloc)
{
	assert(!alloc->head);
}

void SA_BeginScope(struct SAlloc* alloc, struct SScope* scope)
{
	scope->memRestore = alloc->mem;
	scope->next = alloc->head;
	alloc->head = scope;
}

void SA_EndScope(struct SAlloc* alloc, struct SScope* scope)
{
	assert(scope == alloc->head);
	alloc->mem = alloc->head->memRestore;
	alloc->head = alloc->head->next;
}

void* SA_Alloc(struct SAlloc* alloc, uint32_t size)
{
	assert(alloc->head);
	void* ret = alloc->mem;
	alloc->mem += size;
	return ret;
}
