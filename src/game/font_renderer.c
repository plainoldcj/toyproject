#include "assets.h"
#include "common.h"
#include "font.h"
#include "font_renderer.h"
#include "json_reader.h"
#include "math.h"
#include "renderer.h"
#include "material_manager.h"

#include "common/reflect.h"

#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FNT_FORMATTED_BUFFER_SIZE 256

#define FNT_CHAR_SCALE 1.0f

static struct
{
	struct Font	font;
	uint16_t	charMap[FONT_CHAR_COUNT];
	char		formatted[FNT_FORMATTED_BUFFER_SIZE];

	struct Rect	brect; // Bounding rect in world-space (y goes up)
} s_fnt;

void FNT_Init(void)
{
	// struct Asset* asset = AcquireAsset("Fonts/consola.ttf_sdf.txt");
	struct Asset* asset = AcquireAsset("Fonts/tf2desc.txt");

	const char* desc = (const char*)Asset_GetData(asset);
	int descLen = Asset_GetSize(asset);

	const struct ReflectedType* type = FindReflectedType("Font");
	if(!type)
	{
		COM_LogPrintf("Cannot find reflection data for Font");
		exit(-1);
	}

	bool success = ReadJson(type, &s_fnt.font, desc, descLen, "sdf font");
	if(!success)
	{
		COM_LogPrintf("Unable to read font description.");
		exit(-1);
	}

	memset(&s_fnt.charMap, 0, sizeof(uint16_t) * FONT_CHAR_COUNT);
	for(int charIdx = 0; charIdx < s_fnt.font.count; ++charIdx)
	{
		const struct FontChar* fontChar = s_fnt.font.chars + charIdx;
		s_fnt.charMap[fontChar->id] = charIdx;
	}

	ReleaseAsset(asset);
}

void FNT_Deinit(void)
{
}

static void DrawChar(float posX, float posY, const struct FontChar* fontChar, float yy)
{
	const float width = s_fnt.font.scaleW;
	const float height = s_fnt.font.scaleH;

	struct Rect texRect;
	texRect.lowerLeft.x = fontChar->x;
	texRect.lowerLeft.y = fontChar->y + fontChar->height;
	texRect.upperRight.x = fontChar->x + fontChar->width;
	texRect.upperRight.y = fontChar->y;

	// Normalize texture coordinates
	texRect.lowerLeft.x /= width;
	texRect.lowerLeft.y /= height;
	texRect.upperRight.x /= width;
	texRect.upperRight.y /= height;

	// Flip y-coordinate
	texRect.lowerLeft.y = 1.0f - texRect.lowerLeft.y;
	texRect.upperRight.y = 1.0f - texRect.upperRight.y;

	struct Vec2 vertices[] =
	{
		{ posX + fontChar->xoffset + fontChar->width * 0.0f, posY - fontChar->yoffset + fontChar->height * 0.0f },
		{ posX + fontChar->xoffset + fontChar->width * 0.0f, posY - fontChar->yoffset + fontChar->height * -1.0f },
		{ posX + fontChar->xoffset + fontChar->width * 1.0f, posY - fontChar->yoffset + fontChar->height * -1.0f },
		{ posX + fontChar->xoffset + fontChar->width * 1.0f, posY - fontChar->yoffset + fontChar->height * 0.0f }
	};

	struct Vec2 texCoords[] =
	{
		{ texRect.lowerLeft.x, texRect.upperRight.y },
		{ texRect.lowerLeft.x, texRect.lowerLeft.y },
		{ texRect.upperRight.x, texRect.lowerLeft.y },
		{ texRect.upperRight.x, texRect.upperRight.y }
	};

	int indices[] = { 0, 1, 2, 0, 2, 3 };

	for(int i = 0; i < 6; ++i)
	{
		int v = indices[i];
		IMM_TexCoord( texCoords[v].x, texCoords[v].y );
		IMM_Vertex( vertices[v].x, vertices[v].y );
	}

	struct Rect* br = &s_fnt.brect;
	for(int i = 0; i < 4; ++i)
	{
		if(vertices[i].x < br->lowerLeft.x)
		{
			br->lowerLeft.x = vertices[i].x;
		}
		if(vertices[i].y < br->lowerLeft.y)
		{
			br->lowerLeft.y = vertices[i].y;
		}
		if(vertices[i].x > br->upperRight.x)
		{
			br->upperRight.x = vertices[i].x;
		}
		if(vertices[i].y > br->upperRight.y)
		{
			br->upperRight.y = vertices[i].y;
		}
	}
}

void FNT_Printf(float posX, float posY, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	memset(s_fnt.formatted, 0, sizeof(s_fnt.formatted));
	vsnprintf(s_fnt.formatted, FNT_FORMATTED_BUFFER_SIZE - 1, format, args);
	va_end(args);

	s_fnt.brect.lowerLeft.x = FLT_MAX;
	s_fnt.brect.lowerLeft.y = FLT_MAX;
	s_fnt.brect.upperRight.x = -FLT_MAX;
	s_fnt.brect.upperRight.y = -FLT_MAX;

	IMM_Begin(Materials_Get(MAT_FONT));

	const char* c = s_fnt.formatted;
	while(*c != '\0')
	{
		uint16_t charIdx = s_fnt.charMap[(int)*c];
		struct FontChar* fontChar = &s_fnt.font.chars[charIdx];
		if(fontChar->id != (int)*c)
		{
			// char not in charset, skip.
			// TODO(cj): Draw ? or some other placeholder.
			continue;
		}

		// DrawChar(posX, posY, fontChar, 0.0f);
		DrawChar(posX, posY, fontChar, 1.0f);
		++c;

		posX += fontChar->xadvance * FNT_CHAR_SCALE;
	}

	IMM_End();
}

const struct Rect* FNT_GetBoundingRect(void)
{
	return &s_fnt.brect;
}
