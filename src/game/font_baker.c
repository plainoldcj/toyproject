#include "assets.h"
#include "common.h"
#include "font_baker.h"
#include "tga_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <stdio.h>

// TODO(cj): We need some kind of big stack allocator.

/*
Baseline is at bottom of upper-case characters.
Everything-font assumes positive y goes down.
*/

struct ColorBgr
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
};

unsigned char temp_bitmap[512*512];
static stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs

struct ColorBgr tgaPixelData[512*512];

static void WriteFontDesc(FILE* file)
{
	fprintf(file, "info face=\"whatever\"\n");
	fprintf(file, "common lineHeight=%f scaleW=%d, scaleH=%d\n", 0.0f, 512, 512);
	fprintf(file, "chars count=%d\n", 96);

	for(int i = 0; i < 96; ++i)
	{
		stbtt_bakedchar c = cdata[i];
		int id = 32 + i;
		fprintf(file, "char id=%d x=%d y=%d width=%d height=%d xoffset=%f yoffset=%f xadvance=%f page=0 chnl=0\n",
			id,
			c.x0, c.y0,
			c.x1 - c.x0, c.y1 - c.y0,
			c.xoff, c.yoff,
			c.xadvance);
	}
}

int BakeFont(const char* assetPath)
{
	assert(sizeof(struct ColorBgr) == 4);

	struct Asset* asset = AcquireAsset(assetPath);
	if(!asset)
	{
		COM_LogPrintf("Baking font '%f' failed: Unable to acquire asset.", assetPath);
		return 1;
	}

	const unsigned char* ttf_buffer = Asset_GetData(asset);
	
	stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!

	ReleaseAsset(asset);

	for(int i = 0; i < 512 * 512; ++i)
	{
		struct ColorBgr* col = &tgaPixelData[i];
		col->b = 255;
		col->g = 255;
		col->r = 255;
		col->a = temp_bitmap[i];
	}

	char output[256];

	sprintf(output, "%s/assets/Fonts/tf2.tga", GetProjectRoot());

	WriteTGA_BGRA(
			output,
			512,
			512,
			(unsigned char*)tgaPixelData);

	sprintf(output, "%s/assets/Fonts/tf2desc.txt", GetProjectRoot());

	FILE* desc = fopen(output, "w");
	WriteFontDesc(desc);
	fclose(desc);

	return 0;
}