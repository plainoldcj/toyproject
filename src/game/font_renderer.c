#include "assets.h"
#include "common.h"
#include "font.h"
#include "font_renderer.h"
#include "math.h"
#include "renderer.h"
#include "material_manager.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FNT_FORMATTED_BUFFER_SIZE 256

#define FNT_CHAR_SCALE 1.0f

static struct
{
	struct Font	font;
	char		formatted[FNT_FORMATTED_BUFFER_SIZE];
} s_fnt;

void FNT_Init(void)
{
	struct Asset* asset = AcquireAsset("Fonts/consola.ttf_sdf.txt");

	const char* desc = (const char*)Asset_GetData(asset);
	int descLen = Asset_GetSize(asset);

	if(!CreateFont(&s_fnt.font, desc, descLen, "sdf font"))
	{
		COM_LogPrintf("Unable to create font.");
		exit(-1);
	}

	ReleaseAsset(asset);
}

void FNT_Deinit(void)
{
	DestroyFont(&s_fnt.font);
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
}

void FNT_Printf(float posX, float posY, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	memset(s_fnt.formatted, 0, sizeof(s_fnt.formatted));
	vsnprintf(s_fnt.formatted, FNT_FORMATTED_BUFFER_SIZE - 1, format, args);
	va_end(args);

	IMM_Begin(Materials_Get(MAT_FONT));

	const char* c = s_fnt.formatted;
	while(*c != '\0')
	{
		// TODO(cj): Dumb loop, can be index lookup.
		struct FontChar* fontChar;
		for(int i = 0; i < s_fnt.font.count; ++i)
		{
			fontChar = &s_fnt.font.chars[i];
			if(fontChar->id == (int)*c)
			{
				break;
			}
		}

		// DrawChar(posX, posY, fontChar, 0.0f);
		DrawChar(posX, posY, fontChar, 1.0f);
		++c;

		posX += fontChar->xadvance * FNT_CHAR_SCALE;
	}

	IMM_End();
}
