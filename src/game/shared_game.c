#include "math.h"
#include "renderer.h"
#include "shared_game.h"

#include <stdlib.h>
#include <string.h>

struct ComponentArray s_transforms;
struct ComponentArray s_drawables;
struct ComponentArray s_inputs;
struct ComponentArray s_colliders;
struct ComponentArray s_triggers;
struct ComponentArray s_bombs;
struct ComponentArray s_players;
struct ComponentArray s_tiles;
struct ComponentArray s_healths;
struct ComponentArray s_explosions;

static struct GameSystem* s_gameSystems;

EntityId_t g_activeInputEntity;
EntityId_t g_cameraEntity;
EntityId_t g_playerEntity;

static float s_physAccu = 0.0f;

static EntityId_t s_deathRow[64];
int s_deathRowCount;

static hrmesh_t s_tile;

struct Tilemap g_tilemap;

static void CreateTile()
{
	const float size = TILE_SIZE;
	// TODO(cj): Index drawing maybe?
	struct Vec2 pos[] =
	{
		{ 0.0f, 0.0f },
		{ size, 0.0f },
		{ size, size },

		{ 0.0f, 0.0f },
		{ size, size },
		{ 0.0f, size }
	};

	struct Vec2 texCoord[] =
	{
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },

		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f }
	};

	struct Vertex* vertices = malloc(sizeof(struct Vertex) * 6);

	for(int i = 0; i < 6; ++i)
	{
		vertices[i].pos[0] = pos[i].x;
		vertices[i].pos[1] = pos[i].y;

		vertices[i].texCoord[0] = texCoord[i].x;
		vertices[i].texCoord[1] = texCoord[i].y;
	}

	struct Mesh mesh;
	mesh.prim = ePrim_Triangles;
	mesh.attrib = VATT_POS | VATT_TEXCOORD;
	mesh.vertexCount = 6;
	mesh.vertices = vertices;

	s_tile = R_CreateMesh(&mesh);

	free(vertices); // TODO(cj): Remove malloc.
}


static void CreateComponentArrays()
{
	CreateComponentArray(&s_transforms, sizeof(struct Transform), NULL, NULL);
	CreateComponentArray(&s_drawables, sizeof(struct Drawable), NULL, NULL);
	CreateComponentArray(&s_inputs, sizeof(struct Input), NULL, NULL);
	CreateComponentArray(&s_colliders, 0, NULL, NULL);
	CreateComponentArray(&s_triggers, sizeof(struct Trigger), NULL, NULL);
	CreateComponentArray(&s_bombs, sizeof(struct Bomb), NULL, NULL);
	CreateComponentArray(&s_players, sizeof(struct Player), NULL, NULL);
	CreateComponentArray(&s_tiles, sizeof(struct Tile), NULL, NULL);
	CreateComponentArray(&s_healths, sizeof(struct Health), NULL, NULL);
	CreateComponentArray(&s_explosions, sizeof(struct ExplosionComp), NULL, NULL);
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

	CreateTile();
}

void Sh_Shutdown(void)
{
	R_ReleaseMesh(s_tile);

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
		struct Tile* tile = FindComponent(&s_tiles, s_deathRow[i]);
		if(tile)
		{
			g_tilemap.tiles[g_tilemap.rowCount * tile->row + tile->col] = 0;
		}

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

hrmesh_t GetTileMesh(void)
{
	return s_tile;
}
