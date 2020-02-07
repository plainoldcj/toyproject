#include "assets.h"
#include "common.h"
#include "font_baker.h"
#include "tga_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <stdio.h>

// TODO(cj): We need some kind of big stack allocator.

struct ColorBgr
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
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
	assert(sizeof(struct ColorBgr) == 3);

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
		col->b = temp_bitmap[i];
		col->g = temp_bitmap[i];
		col->r = temp_bitmap[i];
	}

	WriteTGA_BGR(
			"output_font.tga",
			512,
			512,
			(unsigned char*)tgaPixelData);

	FILE* desc = fopen("/home/cj/Projects/toyproject/assets/Fonts/tf2desc.txt", "w");
	WriteFontDesc(desc);
	fclose(desc);

	return 0;
}
