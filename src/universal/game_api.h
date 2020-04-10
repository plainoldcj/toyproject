#pragma once

#include <stdbool.h>

struct GameServices
{
};

struct GameApi
{
	const char* (*msg)(void); // TODO(cj): Remove this.

	bool (*init)(void);
	void (*deinit)(void);
};
