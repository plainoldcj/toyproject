#pragma once

#include <stdbool.h>

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

	void (*draw)(void);
};
