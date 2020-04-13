#pragma once

#include <stdint.h>

typedef struct
{
	uint16_t index;
	uint16_t generation;
} hgbuffer_t;

struct GfxUniforms
{
	float projection[16];
	float modelView[16];
};

struct Graphics
{
	hgbuffer_t	(*createBuffer)(void* ins, void* data, uint32_t size);
	void		(*destroyBuffer)(void* ins, hgbuffer_t hgbuffer);
	void		(*setBufferData)(void* ins, hgbuffer_t hgbuffer);

	void		(*setUniforms)(void* ins, struct GfxUniforms* uniforms);

	void		(*drawPrimitives)(
			void* ins,
			hgbuffer_t vertexBuffer,
			uint16_t first,
			uint16_t count);

	void*		ins;
};
