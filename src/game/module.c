#include "assets.h"
#include "common.h"

#include "universal/game_api.h"
#include "universal/unit_tests.h"

static struct GameApi s_gameApi;
static struct GameServices* s_gameServices;

struct GameServices* GetGameServices(void)
{
	return s_gameServices;
}

static const char* msg(void)
{
	return "This is the game";
}

static bool Init(struct GameServices* services)
{
	s_gameServices = services;

	// Make sure the logfile is already initialized when running the unit-test.
	// TODO(cj): Mock the logfile and redirect output.
	COM_Init();

	RunAllUnitTests();

	InitAssets();

	return true;
}

static void Deinit(void)
{
	DeinitAssets();

	COM_Deinit();
}

struct GameApi* GetGameApi(void)
{
	static struct GameApi gameApi =
	{
		.msg = &msg,
		.init = &Init,
		.deinit = &Deinit
	};
	return &gameApi;
}
