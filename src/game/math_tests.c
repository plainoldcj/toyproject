#include "math.h"
#include "math_unit_tests.h"

#include "universal/unit_tests.h"

#include <math.h>

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
