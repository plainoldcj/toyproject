#pragma once

__REFLECTED__
struct JsonWriterTestNested
{
	float fValue;
};

__REFLECTED__
struct JsonWriterTest
{
	float						fValue;
	char						str[64];
	struct JsonWriterTestNested	nested;
};
