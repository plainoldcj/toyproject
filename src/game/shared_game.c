#include "math.h"
#include "renderer.h"
#include "shared_game.h"

#include <string.h>

struct ComponentArray s_transforms;
struct ComponentArray s_drawables;
struct ComponentArray s_inputs;
struct ComponentArray s_colliders;
struct ComponentArray s_triggers;
struct ComponentArray s_bombs;

static struct GameSystem* s_gameSystems;

EntityId_t g_activeInputEntity;
EntityId_t g_cameraEntity;
EntityId_t g_playerEntity;

static float s_physAccu = 0.0f;

static EntityId_t s_deathRow[64];
int s_deathRowCount;

static void CreateComponentArrays()
{
	CreateComponentArray(&s_transforms, sizeof(struct Transform), NULL, NULL);
	CreateComponentArray(&s_drawables, sizeof(struct Drawable), NULL, NULL);
	CreateComponentArray(&s_inputs, sizeof(struct Input), NULL, NULL);
	CreateComponentArray(&s_colliders, 0, NULL, NULL);
	CreateComponentArray(&s_triggers, sizeof(struct Trigger), NULL, NULL);
	CreateComponentArray(&s_bombs, sizeof(struct Bomb), NULL, NULL);
}

static void CreateSpecialEntities()
{
	g_activeInputEntity = CreateEntity();
	struct Input* input = AddEntityComponent(&s_inputs, g_activeInputEntity);
	memset(input, 0, sizeof(struct Input));

	g_cameraEntity = CreateEntity();
	struct Transform* transform = AddEntityComponent(&s_transforms, g_cameraEntity);
	memset(transform, 0, sizeof(struct Transform));
	transform->posX = 2.0f;
	transform->posY = 2.0f;
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

	// Kill all entities.
	for (int i = 0; i < s_deathRowCount; ++i)
	{
		RemoveAllEntityComponents(s_deathRow[i]);
	}
	s_deathRowCount = 0;
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

static void Rect_Translate(struct Rect* rect, float x, float y)
{
	rect->lowerLeft.x += x;
	rect->lowerLeft.y += y;

	rect->upperRight.x += x;
	rect->upperRight.y += y;
}

void GetCollisionRect(float posX, float posY, struct Rect* rect, float shrink)
{
	Vec2_SetF(&rect->lowerLeft, shrink, shrink);
	Vec2_SetF(&rect->upperRight, 1.0f - shrink, 1.0f - shrink);
	Rect_Translate(rect, posX, posY);
}

void DeleteLater(EntityId_t entId)
{
	s_deathRow[s_deathRowCount++] = entId;
}
