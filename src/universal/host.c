#include "host.h"

#include "game_api.h"

#include <dlfcn.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct GameApi* (*GetGameApiFunc)(void);

static void* s_gameModule;

bool Host_LoadGame(const char* projectRoot, struct GameApi** gameApi)
{
	assert(!s_gameModule);

	const int BUFFER_SIZE = 1024;

	char buffer[BUFFER_SIZE];
	snprintf(buffer, BUFFER_SIZE, "%s/build/bin/libgame.dylib", projectRoot);

	s_gameModule = dlopen(buffer, RTLD_LAZY);
	if(!s_gameModule)
	{
		printf("Unable to load game library.\n");
		return false;
	}

	GetGameApiFunc getGameApi = (GetGameApiFunc)dlsym(s_gameModule, "GetGameApi");
	if(!getGameApi)
	{
		printf("Unable to find GetGameApi.\n");
		return false;
	}

	*gameApi = getGameApi();

	return true;
}

