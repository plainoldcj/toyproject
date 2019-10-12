#include "common.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <Windows.h>

#include <stdbool.h>
#include <stdio.h>

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
	COM_Init();

	if(SDL_Init(SDL_INIT_VIDEO))
	{
		COM_LogPrintf("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_Window* window = SDL_CreateWindow("Project",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		400, 400,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );
	if(!window)
	{
		COM_LogPrintf("Unable to create window: %s", SDL_GetError());
		return 1;
	}

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
	if(!glContext)
	{
		COM_LogPrintf("Unable to create OpenGL context: %s", SDL_GetError());
		return 1;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	SDL_Event event;
	bool isDone = false;
	while(!isDone)
	{
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT
				|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
			{
				isDone = true;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);
		glBegin(GL_TRIANGLES);
			glVertex3f(-1.0f, -1.0f, 0.0f);
			glVertex3f(1.0f, -1.0f, 0.0f);
			glVertex3f(0.0f, 1.0f, 0.0f);
		glEnd();

		SDL_GL_SwapWindow(window);
	}

	return 0;
}
