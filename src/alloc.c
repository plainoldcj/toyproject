#include "alloc.h"

#include <assert.h>
#include <stdlib.h>

struct FLNode
{
	struct FLNode* next;
	uint32_t size;
};

void FLAlloc_Init(struct FLAlloc* alloc, void* mem, uint32_t size)
{
	assert(sizeof(struct FLNode) < size);

	struct FLNode* node = mem;
	node->next = NULL;
	node->size = size;

	alloc->mem = mem;
	alloc->size = size;
	alloc->freeList = node;
}

struct Chunk FLAlloc_Alloc(struct FLAlloc* alloc, uint32_t size)
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
	assert((char*)chunk.mem >= (char*)alloc->mem && (char*)chunk.mem < ((char*)alloc->mem + alloc->size));

	struct FLNode* newNode = (struct FLNode*)chunk.mem;
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

