#include "app_states.h"
#include "assets.h"
#include "common.h"
#include "font_baker.h"
#include "font_renderer.h"
#include "renderer.h"
#include "timer.h"
#include "ui.h"

#include "universal/game_api.h"
#include "universal/unit_tests.h"

#include <assert.h>

static struct GameApi s_gameApi;
static struct GameServices* s_gameServices;

static struct Timer s_tickTimer;

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

	int ret = BakeFont("Fonts/tf2build.ttf");
	assert(!ret);

	const int screenWidth = 800;
	const int screenHeight = 600;
	R_Init(screenWidth, screenHeight);

	FNT_Init();

	UI_Init(screenWidth, screenHeight);

	AS_Init();

	Timer_Init(&s_tickTimer);
	Timer_Start(&s_tickTimer);

	return true;
}

static void Deinit(void)
{
	UI_Deinit();

	FNT_Deinit();

	R_Shutdown();

	DeinitAssets();

	COM_Deinit();
}

static void Resize(uint16_t width, uint16_t height)
{
	R_Resize(width, height);
}

static void Draw(void)
{
	R_BeginFrame();
	AS_Draw();
	R_EndFrame();
}

static void Tick(void)
{
	Timer_Stop(&s_tickTimer);
	const float elapsedSecs = (float)Timer_GetElapsedSeconds(&s_tickTimer);
	Timer_Start(&s_tickTimer);

	AS_Tick(elapsedSecs);
}

struct GameApi* GetGameApi(void)
{
	static struct GameApi gameApi =
	{
		.msg = &msg,
		.init = &Init,
		.deinit = &Deinit,
		.resize = &Resize,
		.draw = &Draw,
		.tick = &Tick
	};
	return &gameApi;
}
