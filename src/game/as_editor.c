#include "app_states.h"
#include "editor.h"

#include <string.h>

static struct AppState s_appState;

static void Enter(void)
{
	Ed_Init();
}

static void Leave(void)
{
	Ed_Shutdown();
}

static void Tick(float elapsedSeconds)
{
	Ed_Tick(elapsedSeconds);
}

static void Draw(void)
{
	Ed_Draw();
}

struct AppState* AcquireEditorAppState(void)
{
	memset(&s_appState, 0, sizeof(struct AppState));

	s_appState.enter = &Enter;
	s_appState.leave = &Leave;
	s_appState.tick = &Tick;
	s_appState.draw = &Draw;

	return &s_appState;
}
