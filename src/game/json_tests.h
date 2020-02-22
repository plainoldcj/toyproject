#pragma once

__REFLECTED__
struct JsonTestNested
{
	float fValue;
};

__REFLECTED__
struct JsonTest
{
	float					fValue;
	int						iValue;
	char					str[64];
	struct JsonTestNested	nested;
};
