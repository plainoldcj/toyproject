#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FONT_FACE_NAME_SIZE		128
#define FONT_DEBUG_NAME_SIZE	256

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

struct Font
{
	// TODO(cj): Debug name not needed in final build.
	char				debugName[FONT_DEBUG_NAME_SIZE];
	char				faceName[FONT_FACE_NAME_SIZE];
	float				lineHeight;
	uint16_t			scaleW;
	uint16_t			scaleH;
	uint16_t			count;
	struct FontChar*	chars;
};

bool CreateFont(
	struct Font*	font,
	const char*		desc,
	int				descLen,
	const char*		debugName);

void DestroyFont(struct Font* font);
