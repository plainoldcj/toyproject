#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FONT_FACE_NAME_SIZE		128
#define FONT_DEBUG_NAME_SIZE	256

#define FONT_CHAR_COUNT			256

__REFLECTED__
struct FontChar
{
	uint16_t	id;
	uint16_t	x;
	uint16_t	y;
	uint8_t		width;
	uint8_t		height;
	float		xoffset;
	float		yoffset;
	float		xadvance;
	uint8_t		page;
	uint8_t		chnl;
};

// TODO(cj): Debug name not needed in final build.
__REFLECTED__
struct Font
{
	char				faceName[FONT_FACE_NAME_SIZE];
	float				lineHeight;
	uint16_t			scaleW;
	uint16_t			scaleH;
	uint16_t			count;

	__REFL_ATTRIB__(elementCountVar, count)
	struct FontChar		chars[FONT_CHAR_COUNT];
};

bool InitFont(
	struct Font*	font,
	const char*		desc,
	int				descLen,
	const char*		debugName);
