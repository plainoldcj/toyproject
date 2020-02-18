#pragma once

// TODO(cj): Rename this struct. Reflection parser seems to have a bug
__REFLECTED__
struct XXXX
{
	float fValue;
};

__REFLECTED__
struct JsonWriterTest
{
	float						fValue;
	char						str[64];
	struct XXXX	nested;
};
