#include "common.h"
#include "alloc_tests.h"
#include "grid.h"
#include "math_tests.h"
#include "renderer.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <stdbool.h>
#include <stdio.h>

#define TILE_SIZE 1.0f

#ifdef PLATFORM_WINDOWS
int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
#else
int main(int argc, char* argv[])
#endif
{
	RunAllocTests();
	RunTests();

	COM_Init();

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

	// Create the grid.
	const int n = 16;

	const float gridX = -(n/2) * TILE_SIZE;
	const float gridY = -(n/2) * TILE_SIZE;

	int gridVertexCount;

	struct Vertex* vertices = CreateGrid(n, TILE_SIZE, gridX, gridY, &gridVertexCount);

	struct Mesh gridMesh;
	gridMesh.vertexCount = gridVertexCount;
	gridMesh.pos = &vertices->x;

	hrmesh_t gridRenderMesh = R_CreateMesh(&gridMesh);

	free(vertices);

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

		R_Draw();
		R_DrawMesh(gridRenderMesh);

		SDL_GL_SwapWindow(window);
	}

	R_DestroyMesh(gridRenderMesh);

	R_Shutdown();

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(window);

	return 0;
}
