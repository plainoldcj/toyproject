#include "alloc.h"

#include "common/unit_tests.h"

#include <stdlib.h>
#include <stdio.h>

#define EXPECT_EQUAL(...) ExpectEqual(__VA_ARGS__, __FILE__, __LINE__)
#define EXPECT_NOT_EQUAL(...) ExpectNotEqual(__VA_ARGS__, __FILE__, __LINE__)

static void ExpectEqual(void* p, void* e, const char* filename, int lineNumber)
{
	if(p != e)
	{
		printf("%s:%d: Expected pointers %p and %p to be equal.\n",
			filename, lineNumber,
			p, e);
		exit(-1);
	}
}


static void ExpectNotEqual(void* p, void* e, const char* filename, int lineNumber)
{
	if(p == e)
	{
		printf("%s:%d: Expected pointers %p and %p to be not equal.\n",
			filename, lineNumber,
			p, e);
		exit(-1);
	}
}

UNIT_TEST(TestFreeListAllocator)
{
	// Setup.

	const uint32_t TOTAL_MEM_SIZE = 2048u;
	void* totalMem = malloc(TOTAL_MEM_SIZE);

	struct FLAlloc alloc;
	struct Chunk chunk;
	struct Chunk chunk1;

	// First allocation uses up total memory, second allocation must fail.

	FL_Init(&alloc, totalMem, TOTAL_MEM_SIZE);
	
	chunk = FL_Alloc(&alloc, TOTAL_MEM_SIZE);
	EXPECT_NOT_EQUAL(chunk.mem, NULL);

	chunk = FL_Alloc(&alloc, 1);
	EXPECT_EQUAL(chunk.mem, NULL);

	// Two allocations, each half the total size. Third allocation must fail.

	FL_Init(&alloc, totalMem, TOTAL_MEM_SIZE);

	chunk = FL_Alloc(&alloc, TOTAL_MEM_SIZE / 2 );
	EXPECT_NOT_EQUAL(chunk.mem, NULL);

	chunk = FL_Alloc(&alloc, TOTAL_MEM_SIZE / 2 );
	EXPECT_NOT_EQUAL(chunk.mem, NULL);

	chunk = FL_Alloc(&alloc, 1);
	EXPECT_EQUAL(chunk.mem, NULL);

	// Two smaller allocations, then a bigger one. This tests coalesced free nodes.

	FL_Init(&alloc, totalMem, TOTAL_MEM_SIZE);

	chunk = FL_Alloc(&alloc, TOTAL_MEM_SIZE / 2 );
	chunk1 = FL_Alloc(&alloc, TOTAL_MEM_SIZE / 2 );

	FL_FreeZero(&alloc, &chunk);
	FL_FreeZero(&alloc, &chunk1);

	chunk = FL_Alloc(&alloc, TOTAL_MEM_SIZE);
	EXPECT_NOT_EQUAL(chunk.mem, NULL);

	// Cleanup.

	free(totalMem);
}
