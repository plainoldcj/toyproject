#include "shared_game.h"

static struct GameSystem s_gameSystem;

static void MakeBombsSolid()
{
	struct ComponentArray* requiredComponents[] =
	{
		&s_bombs,
		&s_triggers
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, CB_ARRAY_COUNT(requiredComponents));

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

static void ExplodeExpiredBombs(float elapsedSeconds)
{
	struct ComponentArray* requiredComponents[] =
	{
		&s_bombs,
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, CB_ARRAY_COUNT(requiredComponents));

	EntityId_t entId;
	while (NextEntityId(&entIt, &entId))
	{
		struct Bomb* bomb = FindComponent(&s_bombs, entId);

		bomb->age += elapsedSeconds;
		if (bomb->age >= 1.9 /* g_gameConfig.bombLifetime */)
		{
			// Delete bomb.
			DeleteLater(entId);
		}
	}
}

static void Tick(float elapsedSeconds)
{
	MakeBombsSolid();
	ExplodeExpiredBombs(elapsedSeconds);
}

struct GameSystem* AcquireBombSystem()
{
	s_gameSystem.tick = &Tick;
	return &s_gameSystem;
}
