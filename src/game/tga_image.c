#include "common.h"
#include "tga_image.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==================================================
// TGA Format.
// ==================================================

#define TYPE_UNCOMPRESSED_RGB24 2

typedef unsigned char byte_t;
typedef unsigned short tgaWord_t;

#pragma pack(push, 1)

typedef struct {
	byte_t imageId;
	byte_t colorMapType;
	byte_t imageType;

	tgaWord_t	colorMapStart;
	tgaWord_t	colorMapLength;
	byte_t colorMapEntrySize;
} FileSpec_t;

typedef struct {
	tgaWord_t originx;
	tgaWord_t originy;

	tgaWord_t	width;
	tgaWord_t	height;
	byte_t	bitsPerPixel;

	byte_t attributes;
} ImageSpec_t;

typedef struct {
	FileSpec_t fileSpec;
	ImageSpec_t imageSpec;
} TGAHeader_t;

#pragma pack(pop)

const FileSpec_t supportedFileSpec = {
	0, 0, TYPE_UNCOMPRESSED_RGB24, 0, 0, 0
};


const char* LoadImageFromMemoryTGA(struct Image* image, void* memory, size_t sourceSize) {
	assert(image);

	TGAHeader_t* header;
	int bytesPerPixel;
	int size;

	byte_t* cursor = (byte_t*)memory;

	if (sourceSize < sizeof(TGAHeader_t))
	{
		return "File too small";
	}
	header = (TGAHeader_t*)cursor;
	cursor += sizeof(TGAHeader_t);

	if(0 != memcmp(&header->fileSpec, &supportedFileSpec, sizeof(FileSpec_t))) {
        return "Invalid tga format. Only uncompressed 24bit rga, 32bit rgba files are supported.";
	}

	if(0 != header->imageSpec.originx || 0 != header->imageSpec.originy) {
        return "Tga image origin must be 0,0";
	}

	// TODO: test for non-power-of-2 textures
	if(0 == header->imageSpec.width || 0 == header->imageSpec.height) {
        return "Image has zero area";
	}

	int format;
	switch(header->imageSpec.bitsPerPixel) {
	case 24:
		format = TgaPixelFormat_RGB8;
		break;
	case 32:
		format = TgaPixelFormat_RGBA8;
		break;
	default:
        return "Must have either 24 or 32bit color depth";
	}

	bytesPerPixel = header->imageSpec.bitsPerPixel / 8;
	size = bytesPerPixel *
		header->imageSpec.width *
		header->imageSpec.height;

	byte_t* pixelData = (byte_t*)malloc(size);
	if(NULL == pixelData) {
        return "out of memory loading";
	}

	memcpy(pixelData, cursor, size);

	byte_t tmp;

	for(int i = 0; i < size; i += bytesPerPixel) {
		tmp = pixelData[i];
		pixelData[i] = pixelData[i + 2];
		pixelData[i + 2] = tmp;
	}

    if(0 != (32 & header->imageSpec.attributes))
	{
        // the origin is in the upper left hand corner instead of the lower
        // left hand corner, so we have to flip the image
        const int stride = bytesPerPixel * header->imageSpec.width;
        int h = 0, l = header->imageSpec.height - 1;
        while(h < l) {
            // swap rows h and l
            for(int i = 0; i < stride; ++i) {
				tmp = pixelData[stride * h + i];
                pixelData[stride * h + i] = pixelData[stride * l + i];
				pixelData[stride * l + i] = tmp;
            }
            h++;
            l--;
        }
    }

	int width = header->imageSpec.width;
	int height = header->imageSpec.height;

	// free(pixelData);
	
	image->width = width;
	image->height = height;
	image->format = format;
	image->pixelData = pixelData;

	return 0;
}

void WriteTGAHeader(tgaWord_t width, tgaWord_t height, FILE* file, int bitsPerPixel) {
    TGAHeader_t header;

    memcpy(&header.fileSpec, &supportedFileSpec, sizeof(FileSpec_t));
    memset(&header.imageSpec, 0, sizeof(ImageSpec_t));

    header.imageSpec.width          = width;
    header.imageSpec.height         = height;
    header.imageSpec.bitsPerPixel   = bitsPerPixel;
    header.imageSpec.attributes		= 32; // <-- flips the origin

    fwrite(&header, sizeof(TGAHeader_t), 1, file);
}

void WriteTGA_BGR(
    const char* filename,
    tgaWord_t width,
    tgaWord_t height,
    byte_t* pixelData)
{
    FILE* file = fopen(filename, "wb");
    if(NULL == file) {
		// TODO(cj): Return error message instead of printing directly.
        COM_LogPrintf("ERROR - WriteTGA_BGR: unable to open file '%s'\n", filename);
        return;
    }

    WriteTGAHeader(width, height, file, 24);

    unsigned sz = width * height * 3;
    fwrite(pixelData, sz, 1, file);
}

void WriteTGA_BGRA(
    const char* filename,
    tgaWord_t width,
    tgaWord_t height,
    byte_t* pixelData)
{
    FILE* file = fopen(filename, "wb");
    if(NULL == file) {
		// TODO(cj): Return error message instead of printing directly.
        COM_LogPrintf("ERROR - WriteTGA_BGR: unable to open file '%s'\n", filename);
        return;
    }

    WriteTGAHeader(width, height, file, 32);

    unsigned sz = width * height * 4;
    fwrite(pixelData, sz, 1, file);

	fclose(file);
}
