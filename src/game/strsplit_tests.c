#include "strsplit.h"

#include "universal/unit_tests.h"

#include <string.h>

UNIT_TEST(TestStrSplit)
{
	struct StrSplit strSplit;
	StrSplit_Init(&strSplit, "  a word ", 0);

	int next;
	const char* str;
	int size;

	next = StrSplit_Next(&strSplit);
	str = StrSplit_String(&strSplit);
	size = StrSplit_Size(&strSplit);

	EXPECT_EQUAL_INT(next, 1);
	EXPECT_EQUAL_INT(strncmp("a", str, 1), 0);
	EXPECT_EQUAL_INT(size, 1);

	next = StrSplit_Next(&strSplit);
	str = StrSplit_String(&strSplit);
	size = StrSplit_Size(&strSplit);

	EXPECT_EQUAL_INT(next, 1);
	EXPECT_EQUAL_INT(strncmp("word", str, 4), 0);
	EXPECT_EQUAL_INT(size, 4);

	next = StrSplit_Next(&strSplit);

	EXPECT_EQUAL_INT(next, 0);
}
