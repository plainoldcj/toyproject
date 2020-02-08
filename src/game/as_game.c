#include "app_states.h"
#include "game.h"

#include <string.h>

static struct AppState s_appState;

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

	s_appState.tick = &Tick;
	s_appState.draw = &Draw;

	return &s_appState;
}
