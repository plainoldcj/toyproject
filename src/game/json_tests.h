#pragma once

__REFLECTED__
struct JsonTestNested
{
	float fvalue;
};

__REFLECTED__
struct JsonTest
{
	float					fvalue0;
	struct JsonTestNested	nested;
	float					fvalue1;
	char					str[64];
};
