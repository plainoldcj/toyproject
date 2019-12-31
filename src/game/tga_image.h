#pragma once

#include <stddef.h>
#include <stdint.h>

enum PixelFormat
{
	PixelFormat_RGB8,
	PixelFormat_RGBA8,
};

struct Image
{
	uint32_t width;
	uint32_t height;
	uint32_t format; // in PixelFormat.
	void* pixelData; // Row-major storage, bottom-to-top.
};

const char* LoadImageFromMemoryTGA(struct Image* image, void* memory, size_t size);
