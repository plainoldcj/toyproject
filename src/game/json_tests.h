#pragma once

#include <stdint.h>

__REFLECTED__
struct JsonTestNested
{
	float fValue;
};

__REFLECTED__
struct JsonTestNestedA
{
	int iValue;
};

__REFLECTED__
struct JsonTest
{
	float					fValue;
	int						iValue;
	uint8_t					u8Value;
	uint16_t				u16Value;
	char					str[64];
	struct JsonTestNested	nested;

	int						count;

	__REFL_ATTRIB__(elementCountVar, count)
	struct JsonTestNestedA	arr[2];
};
