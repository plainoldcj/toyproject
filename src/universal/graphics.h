#pragma once

#include <stdint.h>

typedef struct
{
	uint16_t index;
	uint16_t generation;
} hgbuffer_t;

typedef struct
{
	uint16_t index;
	uint16_t generation;
} hgtex_t;

enum GfxPixelFormat
{
	GfxPixelFormat_RGBA8
};

struct GfxUniforms
{
	float projection[16];
	float modelView[16];
};

struct Graphics
{
	hgbuffer_t	(*createBuffer)(void* ins, void* data, uint32_t size);
	void		(*destroyBuffer)(void* ins, hgbuffer_t hgbuffer);
	void		(*setBufferData)(void* ins, hgbuffer_t hgbuffer, void* data, uint32_t size);

	hgtex_t		(*createTexture)(void* ins, uint16_t width, uint16_t height, uint16_t format);
	void		(*destroyTexture)(void* ins, hgtex_t hgtex);
	void		(*setTextureData)(void* ins, hgtex_t hgtex, void* data);
	void		(*bindTexture)(void* ins, hgtex_t hgtex);

	void		(*setUniforms)(void* ins, struct GfxUniforms* uniforms);

	void		(*setUniformBuffer)(void* ins, hgbuffer_t uniformBuffer, uint32_t offset);

	void		(*drawPrimitives)(
			void* ins,
			hgbuffer_t vertexBuffer,
			uint16_t first,
			uint16_t count);

	void*		ins; // TODO(cj,soon): Can this be typed?
};
