#pragma once

__REFLECTED__
struct JsonReaderTestNested
{
	float fvalue;
};

__REFLECTED__
struct JsonReaderTest
{
	float						fvalue0;
	struct JsonReaderTestNested	nested;
	float						fvalue1;
	char						str[64];
};
