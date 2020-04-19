#pragma once

#include <stdbool.h>
#include <stdint.h>

struct Graphics;

struct GameServices
{
	const char*			(*getProjectRoot)(void);
	struct Graphics*	(*getGraphics)(void);
};

struct GameApi
{
	const char* (*msg)(void); // TODO(cj): Remove this.

	bool (*init)(struct GameServices* services);
	void (*deinit)(void);

	void (*resize)(uint16_t width, uint16_t height);

	void (*draw)(void);
	void (*tick)(void);
};
