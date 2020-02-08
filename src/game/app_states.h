#pragma once

#include <stdbool.h>

enum AppStates
{
	AS_EXIT,

	AS_MENU,
	AS_GAME,

	AS_COUNT
};

struct AppState
{
	// void	(*enter)(void);
	// void	(*leave)(void);

	void	(*tick)(float elapsedSeconds);
	void	(*draw)(void);

	// Filled in by system.
	void	(*transition)(enum AppStates newState);
};

struct AppState* AcquireMenuAppState(void);
struct AppState* AcquireGameAppState(void);

void	AS_Init(void);

bool	AS_IsDone(void);

void	AS_Tick(float elapsedSeconds);
void	AS_Draw(void);
