#pragma once

#include <stdbool.h>

#define EXPECT_EQUAL_VEC4(...) ExpectEqualVec4(__VA_ARGS__, __FILE__, __LINE__)

struct Vec4;

void ExpectEqualVec4(const struct Vec4* a, float x, float y, float z, float w,
	const char* filename, int lineNumber);
