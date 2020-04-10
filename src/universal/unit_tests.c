#include "universal/unit_tests.h"

#include "math.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static const float eps = 0.001f;

void ExpectEqualPtr(void* p, void* e, const char* filename, int lineNumber)
{
	if(p != e)
	{
		printf("%s:%d: Expected pointers %p and %p to be equal.\n",
			filename, lineNumber,
			p, e);
		exit(-1);
	}
}

void ExpectNotEqualPtr(void* p, void* e, const char* filename, int lineNumber)
{
	if(p == e)
	{
		printf("%s:%d: Expected pointers %p and %p to be not equal.\n",
			filename, lineNumber,
			p, e);
		exit(-1);
	}
}

void ExpectEqualInt(int p, int e, const char* filename, int lineNumber)
{
	if(p != e)
	{
		printf("%s:%d: Expected integers %d and %d to be equal.\n",
			filename, lineNumber,
			p, e);
		exit(-1);
	}
}

void ExpectNotEqualInt(int p, int e, const char* filename, int lineNumber)
{
	if(p == e)
	{
		printf("%s:%d: Expected integers %d and %d to be not equal.\n",
			filename, lineNumber,
			p, e);
		exit(-1);
	}
}

void ExpectEqualFloat(float p, float e, const char* filename, int lineNumber)
{
	if(fabs(p - e) > eps)
	{
		printf("%s:%d: Expected floats %f and %f to be equal.\n",
			filename, lineNumber,
			p, e);
		exit(-1);
	}
}

void ExpectNotEqualFloat(float p, float e, const char* filename, int lineNumber)
{
	if(fabs(p - e) < eps)
	{
		printf("%s:%d: Expected floats %f and %f to be not equal.\n",
			filename, lineNumber,
			p, e);
		exit(-1);
	}
}

void ExpectTrue(bool value, const char* filename, int lineNumber)
{
	if(value == false)
	{
		printf("%s:%d: Expected condition to be true\n",
			filename, lineNumber);
		exit(-1);
	}
}

void ExpectFalse(bool value, const char* filename, int lineNumber)
{
	if(value == true)
	{
		printf("%s:%d: Expected condition to be false\n",
			filename, lineNumber);
		exit(-1);
	}
}