#pragma once

#include <stddef.h>
#include <stdint.h>

enum TgaPixelFormat
{
	TgaPixelFormat_RGB8,
	TgaPixelFormat_RGBA8,
};

struct Image
{
	uint32_t width;
	uint32_t height;
	uint32_t format; // in TgaPixelFormat.
	void* pixelData; // Row-major storage, bottom-to-top.
};

const char* LoadImageFromMemoryTGA(struct Image* image, void* memory, size_t size);

// TODO(cj): Write to memory instead of file.
void WriteTGA_BGR(
    const char* filename,
    unsigned short width,
    unsigned short height,
    unsigned char* pixelData);

// TODO(cj): Write to memory instead of file.
void WriteTGA_BGRA(
    const char* filename,
    unsigned short width,
    unsigned short height,
    unsigned char* pixelData);
