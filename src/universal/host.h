#pragma once

#include <stdbool.h>

struct GameApi;

bool Host_LoadGame(const char* projectRoot, struct GameApi** gameApi);
