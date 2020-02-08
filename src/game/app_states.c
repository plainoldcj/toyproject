#include "app_states.h"

#include <assert.h>
#include <string.h>

static struct
{
	struct AppState		exitState;
	struct AppState*	states[AS_COUNT];

	enum AppStates		active;
	enum AppStates		transition;
} s_as;

static void ExitTick(float elapsedSeconds) {}
static void ExitDraw(void) {}

static void Transition(enum AppStates newState)
{
	s_as.transition = newState;
}

void AS_Init(void)
{
	memset(s_as.states, 0, sizeof(struct AppState*) * AS_COUNT);

	s_as.exitState.tick = &ExitTick;
	s_as.exitState.draw = &ExitDraw;

	s_as.states[AS_EXIT] = &s_as.exitState;
	s_as.states[AS_MENU] = AcquireMenuAppState();
	s_as.states[AS_GAME] = AcquireGameAppState();

	for(int i = 0; i < AS_COUNT; ++i)
	{
		s_as.states[i]->transition = &Transition;
	}

	s_as.active = AS_MENU;
	s_as.transition = AS_MENU;
}

bool AS_IsDone(void)
{
	return s_as.active == AS_EXIT;
}

void AS_Tick(float elapsedSeconds)
{
	struct AppState* state = s_as.states[s_as.active]; 
	state->tick(elapsedSeconds);

	if(s_as.active != s_as.transition)
	{
		// TODO(cj) enter, leave
		s_as.active = s_as.transition;
	}
}

void AS_Draw(void)
{
	s_as.states[s_as.active]->draw();
}
