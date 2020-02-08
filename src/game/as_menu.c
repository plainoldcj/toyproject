#include "app_states.h"
#include "ui.h"

#include <string.h>

static struct AppState s_appState;

static void Tick(float elapsedSeconds)
{
	if(UI_Button(100.0f, 150.0f, "Game"))
	{
		s_appState.transition(AS_GAME);
	}

	if(UI_Button(100.0f, 100.0f, "Quit"))
	{
		s_appState.transition(AS_EXIT);
	}
}

static void Draw(void)
{
}

struct AppState* AcquireMenuAppState(void)
{
	memset(&s_appState, 0, sizeof(struct AppState));

	s_appState.tick = &Tick;
	s_appState.draw = &Draw;

	return &s_appState;
}
