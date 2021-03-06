#include "renderer.h"

#include "common.h"
#include "material_manager.h"
#include "shared_game.h"

#include <assert.h>
#include <stdio.h>

static struct GameSystem s_gameSystem;

#define BOMB_LIFETIME 1.9f
#define BOMB_CHAIN_TIME 0.2f

static void MakeBombsSolid()
{
	struct ComponentArray* requiredComponents[] =
	{
		&s_bombs,
		&s_triggers
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, KQ_ARRAY_COUNT(requiredComponents));

	EntityId_t solidBombs[64];
	int solidBombCount = 0;

	EntityId_t entId;
	while (NextEntityId(&entIt, &entId))
	{
		struct Trigger* trigger = FindComponent(&s_triggers, entId);
		if (!trigger->isPlayerTouching)
		{
			solidBombs[solidBombCount++] = entId;
		}
	}

	for (int i = 0; i < solidBombCount; ++i)
	{
		EntityId_t entId = solidBombs[i];
		AddEntityComponent(&s_colliders, entId);
		RemoveEntityComponent(&s_triggers, entId);
	}
}

enum
{
	DIR_LEFT,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN
};

struct Explosion
{
	EntityId_t	bombEnt;
	uint16_t	range[4];
};

#define RANGE 5

static void ExplodeDirection(struct Explosion* expl, int dir)
{
	const struct Tile* bombTile = FindComponent(&s_tiles, expl->bombEnt);
	assert(bombTile);

	static struct { int32_t rowOff, colOff; } off[] =
	{
		{ 0, -1 },
		{ 0, 1 },
		{ 1, 0 },
		{ -1, 0 }
	};

	expl->range[dir] = 0;

	// TODO(cj): i = 0 is sampled for every direction.
	for(int range = 0; range < RANGE; ++range)
	{
		int32_t hitRow = range * off[dir].rowOff + bombTile->row;
		int32_t hitCol = range * off[dir].colOff + bombTile->col;

		const bool validTile =
			hitRow >= 0 && hitRow < g_tilemap.rowCount &&
			hitCol >= 0 && hitCol < g_tilemap.colCount;

		// printf("row = %d, col = %d\n", hitRow, hitCol);

		if(validTile)
		{
			EntityId_t hitEnt = g_tilemap.tiles[hitRow * g_tilemap.rowCount + hitCol];
			if(hitEnt == expl->bombEnt || !hitEnt || !(FindComponent(&s_colliders, hitEnt) || FindComponent(&s_triggers, hitEnt)))
			{
				expl->range[dir] = range;
			}
			else
			{
				struct Health* health = FindComponent(&s_healths, hitEnt);
				if(health)
				{
					health->bombHits++;
				}
			}
		}

		if(expl->range[dir] < range)
		{
			break; // We hit something.
		}
	}
}

static void CreateExplosionEntity(float posX, float posY)
{
	EntityId_t entId = CreateEntity();

	struct Transform* transform = AddEntityComponent(&s_transforms, entId);
	struct Drawable* drawable = AddEntityComponent(&s_drawables, entId);
	struct ExplosionComp* explosion = AddEntityComponent(&s_explosions, entId);

	explosion->age = 0.0f;

	transform->posX = posX;
	transform->posY = posY;

	drawable->hrobj = R_CreateObject(GetTileMesh());

	R_SetObjectMaterial(drawable->hrobj, Materials_Get(MAT_EXPLOSION));
}

// TODO(cj): Maybe add new entities at a later time?
static void SpawnExplosionEntities(struct Explosion* expl)
{
	struct Transform* bombTransform = FindComponent(&s_transforms, expl->bombEnt);
	assert(bombTransform);

	static struct { int32_t rowOff, colOff; } off[] =
	{
		{ 0, -1 },
		{ 0, 1 },
		{ 1, 0 },
		{ -1, 0 }
	};

	for(int dir = 0; dir < 4; ++dir)
	{
		for(int i = 1; i <= expl->range[dir]; ++i)
		{
			float posY = off[dir].rowOff * TILE_SIZE * i + bombTransform->posY;
			float posX = off[dir].colOff * TILE_SIZE * i + bombTransform->posX;

			CreateExplosionEntity(posX, posY);
		}
	}
	CreateExplosionEntity(bombTransform->posX, bombTransform->posY);
}

static void DoExplosion(EntityId_t bombEnt)
{
	printf("--------------\n");

	struct Explosion expl;
	expl.bombEnt = bombEnt;

	for(int dir = 0; dir < 4; ++dir)
	{
		expl.range[dir] = RANGE;
		ExplodeDirection(&expl, dir);
	}
	SpawnExplosionEntities(&expl);

	for(int i = 0; i < 4; ++i)
	{
		const char* names[] = { "left", "right", "up", "down" };
		printf("range[%s] = %d\n", names[i], expl.range[i]);
	}
}

static void ExplodeExpiredBombs(float elapsedSeconds)
{
	struct ComponentArray* requiredComponents[] =
	{
		&s_bombs,
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, KQ_ARRAY_COUNT(requiredComponents));

	EntityId_t entId;
	while (NextEntityId(&entIt, &entId))
	{
		struct Bomb* bomb = FindComponent(&s_bombs, entId);

		bomb->age += elapsedSeconds;
		if (bomb->age >= BOMB_LIFETIME)
		{
			DoExplosion(entId);

			// Delete bomb.
			DeleteLater(entId);
		}
	}
}

static void ExplodeExplodedBombs(float elapsedSeconds)
{
	struct ComponentArray* requiredComponents[] =
	{
		&s_bombs,
		&s_healths
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, KQ_ARRAY_COUNT(requiredComponents));

	EntityId_t entId;
	while(NextEntityId(&entIt, &entId))
	{
		struct Health* health = FindComponent(&s_healths, entId);
		if(health->bombHits > 0)
		{
			struct Bomb* bomb = FindComponent(&s_bombs, entId);

			bomb->chain += elapsedSeconds;
			if(bomb->chain >= BOMB_CHAIN_TIME)
			{
				DoExplosion(entId);

				// Delete bomb.
				DeleteLater(entId);
			}
		}
	}
}

static void Tick(float elapsedSeconds)
{
	MakeBombsSolid();
	ExplodeExpiredBombs(elapsedSeconds);

	ExplodeExplodedBombs(elapsedSeconds);
}

struct GameSystem* AcquireBombSystem()
{
	s_gameSystem.tick = &Tick;
	return &s_gameSystem;
}
