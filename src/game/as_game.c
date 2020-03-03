#include "app_states.h"
#include "game.h"

#include <string.h>

static struct AppState s_appState;

static void Enter(void)
{
	G_Init();
}

static void Leave(void)
{
	G_Shutdown();
}

static void Tick(float elapsedSeconds)
{
	G_Tick(elapsedSeconds);
}

static void Draw(void)
{
	G_Draw();
}

struct AppState* AcquireGameAppState(void)
{
	memset(&s_appState, 0, sizeof(struct AppState));

	s_appState.enter = &Enter;
	s_appState.leave = &Leave;
	s_appState.tick = &Tick;
	s_appState.draw = &Draw;

	return &s_appState;
}
