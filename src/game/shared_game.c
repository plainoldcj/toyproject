#include "renderer.h"

#include "shared_game.h"

#include <string.h>

struct ComponentArray s_transforms;
struct ComponentArray s_drawables;
struct ComponentArray s_inputs;
struct ComponentArray s_colliders;

static struct GameSystem* s_gameSystems;

EntityId_t g_activeInputEntity;
EntityId_t g_cameraEntity;
EntityId_t g_playerEntity;

static float s_physAccu = 0.0f;

static void CreateComponentArrays()
{
	CreateComponentArray(&s_transforms, sizeof(struct Transform), NULL, NULL);
	CreateComponentArray(&s_drawables, sizeof(struct Drawable), NULL, NULL);
	CreateComponentArray(&s_inputs, sizeof(struct Input), NULL, NULL);
	CreateComponentArray(&s_colliders, 0, NULL, NULL);
}

static void CreateSpecialEntities()
{
	g_activeInputEntity = CreateEntity();
	struct Input* input = AddEntityComponent(&s_inputs, g_activeInputEntity);
	memset(input, 0, sizeof(struct Input));

	g_cameraEntity = CreateEntity();
	struct Transform* transform = AddEntityComponent(&s_transforms, g_cameraEntity);
	memset(transform, 0, sizeof(struct Transform));
}

static void AddGameSystem(struct GameSystem* gameSystem)
{
	gameSystem->next = s_gameSystems;
	s_gameSystems = gameSystem;
}

static void InitGameSystems()
{
#define FOR_GAMESYSTEM(x) AddGameSystem(Acquire##x());
	FORALL_GAMESYSTEMS
#undef FOR_GAMESYSTEM
}

void Sh_Init(void)
{
	CreateComponentArrays();
	InitGameSystems();

	CreateSpecialEntities();
}

void Sh_Shutdown(void)
{
	DestroyAllComponentArrays();
}

void Sh_Tick(float elapsedSeconds)
{
	s_physAccu += elapsedSeconds;
	while (s_physAccu >= PHYS_DT)
	{
		struct GameSystem* sysIt = s_gameSystems;
		while (sysIt)
		{
			if (sysIt->physicsTick)
			{
				sysIt->physicsTick();
			}
			sysIt = sysIt->next;
		}

		s_physAccu -= PHYS_DT;
	}

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
