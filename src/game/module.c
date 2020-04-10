#include "universal/game_api.h"

static struct GameApi s_gameApi;

static const char* msg(void)
{
	return "This is the game";
}

struct GameApi* GetGameApi(void)
{
	s_gameApi.msg = &msg;
	return &s_gameApi;
}
