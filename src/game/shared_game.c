#include "renderer.h"

#include "shared_game.h"

#include "entity.h"

struct ComponentArray s_transforms;
struct ComponentArray s_drawables;

static struct GameSystem* s_gameSystems;

static void CreateComponentArrays()
{
	CreateComponentArray(&s_transforms, sizeof(struct Transform), NULL, NULL);
	CreateComponentArray(&s_drawables, sizeof(struct Drawable), NULL, NULL);
}

static void AddGameSystem(struct GameSystem* gameSystem)
{
	gameSystem->next = s_gameSystems;
	s_gameSystems = gameSystem;
}

static void InitGameSystems()
{
	AddGameSystem(AcquireDrawSystem());
}

void Sh_Init(void)
{
	CreateComponentArrays();
	InitGameSystems();
}

void Sh_Shutdown(void)
{
	DestroyAllComponentArrays();
}

void Sh_Tick(float elapsedSeconds)
{
	struct GameSystem* sysIt = s_gameSystems;
	while (sysIt)
	{
		if (sysIt->tick)
		{
			sysIt->tick(elapsedSeconds);
		}
		sysIt = sysIt->next;
	}
}

void Sh_Draw()
{
	struct GameSystem* sysIt = s_gameSystems;
	while (sysIt)
	{
		if (sysIt->draw)
		{
			sysIt->draw();
		}
		sysIt = sysIt->next;
	}
}
