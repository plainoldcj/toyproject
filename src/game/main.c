#include "common.h"
#include "alloc_tests.h"
#include "editor.h"
#include "math_tests.h"
#include "renderer.h"

#include "common/reflect.h"
#include "common/unit_tests.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <stdbool.h>
#include <stdio.h>

static float MillisecsToSecs(float ms)
{
	return ms / 1000.0f;
}

#ifdef PLATFORM_WINDOWS
int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
#else
int main(int argc, char* argv[])
#endif
{
	RunAllUnitTests();
	RunAllocTests();
	RunTests();

	COM_Init();

	// TODO(cj): Remove this.
	{
		char buffer[4096] = { 0 };
		PrintReflectedType(buffer, "Transform");
		printf("%s", buffer);
	}

	if(SDL_Init(SDL_INIT_VIDEO))
	{
		COM_LogPrintf("Unable to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	const int screenWidth = 400;
	const int screenHeight = 400;

	SDL_Window* window = SDL_CreateWindow("Project",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		screenWidth, screenHeight,
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

	R_Init(screenWidth, screenHeight);

	Ed_Init();

	Uint32 lastTicks = SDL_GetTicks();

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

		Uint32 ticks = SDL_GetTicks();
		float elapsedMillisecs = (float)(ticks - lastTicks);
		lastTicks = ticks;

		Ed_Tick(MillisecsToSecs(elapsedMillisecs));

		R_Draw();
		Ed_Draw();

		SDL_GL_SwapWindow(window);
	}

	Ed_Shutdown();

	R_Shutdown();

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);

	return 0;
}
