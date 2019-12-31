#pragma once

#define EXPECT_EQUAL(...) ExpectEqual(__VA_ARGS__, __FILE__, __LINE__)
#define EXPECT_NOT_EQUAL(...) ExpectNotEqual(__VA_ARGS__, __FILE__, __LINE__)

#define EXPECT_EQUAL_VEC4(...) ExpectEqualVec4(__VA_ARGS__, __FILE__, __LINE__)

struct Vec4;

void ExpectEqual(void* p, void* e, const char* filename, int lineNumber);
void ExpectNotEqual(void* p, void* e, const char* filename, int lineNumber);

void ExpectEqualVec4(const struct Vec4* a, float x, float y, float z, float w,
	const char* filename, int lineNumber);

#define UNIT_TEST(x) void x(void)

void RunAllUnitTests();
