#pragma once

struct Timer
{
	int data[8];
};

void	Timer_Init(struct Timer* timer);

void	Timer_Start(struct Timer* timer);
void	Timer_Stop(struct Timer* timer);

double	Timer_GetElapsedSeconds(const struct Timer* timer);
