#include "common.h"
#include "font.h"
#include "json_reader.h"

#include "common/reflect.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// TODO(cj): Remove this function.
bool InitFont(
	struct Font*	font,
	const char*		desc,
	int				descLen,
	const char*		debugName)
{
	struct Font f;
	const struct ReflectedType* type = FindReflectedType("Font");
	assert(type);

	bool success = ReadJson(type, &f, desc, descLen, debugName);
	assert(success);

	memcpy(font, &f, sizeof(struct Font));
	for(int i = 0; i < font->count; ++i)
	{
		struct FontChar fontChar = f.chars[i];
		font->chars[fontChar.id] = fontChar;
	}

	return true;
}
