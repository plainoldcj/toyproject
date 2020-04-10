#include "common.h"

#include "universal/game_api.h"
#include "universal/unit_tests.h"

static struct GameApi s_gameApi;

static const char* msg(void)
{
	return "This is the game";
}

static bool Init(void)
{
	// Make sure the logfile is already initialized when running the unit-test.
	// TODO(cj): Mock the logfile and redirect output.
	COM_Init();

	RunAllUnitTests();

	return true;
}

static void Deinit(void)
{
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
