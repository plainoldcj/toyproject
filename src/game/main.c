#include "app_states.h"
#include "assets.h"
#include "common.h"
#include "alloc_tests.h"
#include "editor.h"
#include "entity.h"
#include "font_baker.h"
#include "font_renderer.h"
#include "game.h"
#include "math_tests.h"
#include "renderer.h"
#include "shared_game.h"
#include "ui.h"

#include "common/reflect.h"
#include "common/unit_tests.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static float MillisecsToSecs(float ms)
{
	return ms / 1000.0f;
}

static void FillInput(SDL_Event* event)
{
	struct Input* input = FindComponent(&s_inputs, g_activeInputEntity);
	if(!input)
	{
		return;
	}

	if(event->type == SDL_KEYDOWN)
	{
		switch(event->key.keysym.sym)
		{
			case SDLK_LEFT:
				input->buttons[BUTTON_LEFT] = 1;
				break;
			case SDLK_UP:
				input->buttons[BUTTON_UP] = 1;
				break;
			case SDLK_RIGHT:
				input->buttons[BUTTON_RIGHT] = 1;
				break;
			case SDLK_DOWN:
				input->buttons[BUTTON_DOWN] = 1;
				break;
			case SDLK_SPACE:
				input->buttons[BUTTON_DROP_BOMB] = 1;
				break;
			default:
				break;
		}
	}
	else if(event->type == SDL_KEYUP)
	{
		switch(event->key.keysym.sym)
		{
			case SDLK_LEFT:
				input->buttons[BUTTON_LEFT] = 0;
				break;
			case SDLK_UP:
				input->buttons[BUTTON_UP] = 0;
				break;
			case SDLK_RIGHT:
				input->buttons[BUTTON_RIGHT] = 0;
				break;
			case SDLK_SPACE:
				input->buttons[BUTTON_DROP_BOMB] = 0;
				break;
			default:
				break;
		}
	}
}

#ifdef PLATFORM_WINDOWS
int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
#else
int main(int argc, char* argv[])
#endif
{
	// Make sure the logfile is already initialized when running the unit-test.
	// TODO(cj): Mock the logfile and redirect output.
	COM_Init();

	RunAllUnitTests();

	InitAssets();

	int ret = BakeFont("Fonts/tf2build.ttf");
	assert(!ret);

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

	const int screenWidth = 800;
	const int screenHeight = 600;

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

	FNT_Init();

	UI_Init(screenWidth, screenHeight);

	AS_Init();

	Uint32 lastTicks = SDL_GetTicks();

	SDL_Event event;
	bool isDone = false;
	while(!isDone)
	{
		bool mouseUp = false;

		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT
				|| (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
			{
				isDone = true;
			}

			// if(event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONDOWN)
			{
				int mouseX;
				int mouseY;

				SDL_GetMouseState(&mouseX, &mouseY);

				UI_SetMousePos(mouseX, mouseY);
			}

			mouseUp |= event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT;

			FillInput(&event);
		}

		UI_SetMouseButtonUp(mouseUp);

		Uint32 ticks = SDL_GetTicks();
		float elapsedMillisecs = (float)(ticks - lastTicks);
		lastTicks = ticks;

		AS_Tick(MillisecsToSecs(elapsedMillisecs));

		R_BeginFrame();
		AS_Draw();
		R_EndFrame();

		SDL_GL_SwapWindow(window);

		if(AS_IsDone())
		{
			isDone = true;
		}
	}

	UI_Deinit();

	FNT_Deinit();

	R_Shutdown();

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);

	DeinitAssets();

	COM_Deinit();

	printf("Exited normally. That's all!\n");

	return 0;
}
