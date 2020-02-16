#pragma once

#include <stdbool.h>

// TODO(cj): Add ptr suffix here
#define EXPECT_EQUAL(...) ExpectEqualPtr(__VA_ARGS__, __FILE__, __LINE__)
#define EXPECT_NOT_EQUAL(...) ExpectNotEqualPtr(__VA_ARGS__, __FILE__, __LINE__)

#define EXPECT_EQUAL_INT(...) ExpectEqualInt(__VA_ARGS__, __FILE__, __LINE__)
#define EXPECT_NOT_EQUAL_INT(...) ExpectNotEqualInt(__VA_ARGS__, __FILE__, __LINE__)

#define EXPECT_EQUAL_FLOAT(...) ExpectEqualFloat(__VA_ARGS__, __FILE__, __LINE__)
#define EXPECT_NOT_EQUAL_FLOAT(...) ExpectNotEqualFloat(__VA_ARGS__, __FILE__, __LINE__)

#define EXPECT_EQUAL_VEC4(...) ExpectEqualVec4(__VA_ARGS__, __FILE__, __LINE__)

#define EXPECT_TRUE(...) ExpectTrue(__VA_ARGS__, __FILE__, __LINE__)
#define EXPECT_FALSE(...) ExpectFalse(__VA_ARGS__, __FILE__, __LINE__)

struct Vec4;

void ExpectEqualPtr(void* p, void* e, const char* filename, int lineNumber);
void ExpectNotEqualPtr(void* p, void* e, const char* filename, int lineNumber);

void ExpectEqualInt(int p, int e, const char* filename, int lineNumber);
void ExpectNotEqualInt(int p, int e, const char* filename, int lineNumber);

void ExpectEqualFloat(float p, float e, const char* filename, int lineNumber);
void ExpectNotEqualFloat(float p, float e, const char* filename, int lineNumber);

void ExpectEqualVec4(const struct Vec4* a, float x, float y, float z, float w,
	const char* filename, int lineNumber);

void ExpectTrue(bool value, const char* filename, int lineNumber);
void ExpectFalse(bool value, const char* filename, int lineNumber);

#define UNIT_TEST(x) void x(void)

void RunAllUnitTests();
