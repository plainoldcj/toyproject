#include "assets.h"
#include "common.h"
#include "font.h"
#include "font_renderer.h"
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

static void DrawChar(float posX, float posY, const struct FontChar* fontChar)
{
	const float width = s_fnt.font.scaleW;
	const float height = s_fnt.font.scaleH;

	float ns0 = (float)fontChar->x / width;
	float nt1 = (float)fontChar->y / height;

	float ns1 = ns0 + (float)fontChar->width / width;
	float nt0 = nt1 + (float)fontChar->height / height;

	const float sx = FNT_CHAR_SCALE * fontChar->width;
	const float sy = FNT_CHAR_SCALE * fontChar->height;

	{
		IMM_TexCoord(ns0, 1.0f - nt0);
		IMM_Vertex(posX + 0.0f, posY + 0.0f);

		IMM_TexCoord(ns1, 1.0f - nt1);
		IMM_Vertex(posX + sx, posY + sy);

		IMM_TexCoord(ns0, 1.0f - nt1);
		IMM_Vertex(posX + 0.0f, posY + sy);

		IMM_TexCoord(ns0, 1.0f - nt0);
		IMM_Vertex(posX + 0.0f, posY + 0.0f);

		IMM_TexCoord(ns1, 1.0f - nt0);
		IMM_Vertex(posX + sx, posY + 0.0f);

		IMM_TexCoord(ns1, 1.0f - nt1);
		IMM_Vertex(posX + sx, posY + sy);
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

		DrawChar(posX, posY, fontChar);
		++c;

		posX += fontChar->xadvance * FNT_CHAR_SCALE;
	}

	IMM_End();
}
