#include "math_unit_tests.h"

#include "universal/unit_tests.h"

#include "math.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static const float eps = 0.001f;

void ExpectEqualVec4(const struct Vec4* a, float x, float y, float z, float w,
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