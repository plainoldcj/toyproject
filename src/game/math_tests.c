#include "math.h"

#include "common/unit_tests.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static const float eps = 0.001f;

#define EXPECT_EQUAL_VEC4(...) ExpectEqualVec4(__VA_ARGS__, __FILE__, __LINE__)

static void ExpectEqualVec4(const struct Vec4* a, float x, float y, float z, float w,
	const char* filename, int lineNumber)
{
	if(fabs(a->x - x) > eps ||
		fabs(a->y - y) > eps ||
		fabs(a->z - z) > eps ||
		fabs(a->w - w) > eps)
	{
		printf("%s:%d: Expected vectors (%f, %f, %f, %f) and (%f, %f, %f, %f) to be equal.\n",
			filename, lineNumber,
			a->x, a->y, a->z, a->w,
			x, y, z, w);
		exit(-1);
	}
}

UNIT_TEST(TestCreatePerspective)
{
	const float fovy = DegToRad(45.0f);
	const float aspect = 1.0f;
	const float zNear = 1.0f;
	const float zFar = 10.0f;

	const struct Mat4 proj = M_CreatePerspective(fovy, aspect, zNear, zFar);

	// The point in the lower left-hand corner on the near projection plane becomes -1,-1,-1 in NDC.
	{
		const float halfHeight =  zNear * tan(0.5f * fovy);
		const float halfWidth = aspect * halfHeight;

		struct Vec4 u =
		{
			-halfWidth,
			-halfHeight,
			-zNear,
			1.0f
		};

		struct Vec4 v = Mat4_MulVec(&proj, &u);
		Vec4_DivI(&v, v.w);

		EXPECT_EQUAL_VEC4(&v, -1.0f, -1.0f, -1.0f, 1.0f);
	}

	// The point in the upper right-hand corner on the near projection plane becomes 1,1,1 in NDC.
	{
		const float halfHeight =  zFar * tan(0.5f * fovy);
		const float halfWidth = aspect * halfHeight;

		struct Vec4 u =
		{
			halfWidth,
			halfHeight,
			-zFar,
			1.0f
		};

		struct Vec4 v = Mat4_MulVec(&proj, &u);
		Vec4_DivI(&v, v.w);

		EXPECT_EQUAL_VEC4(&v, 1.0f, 1.0f, 1.0f, 1.0f);
	}
}
