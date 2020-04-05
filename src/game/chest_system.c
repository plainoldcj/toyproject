#include "common.h"
#include "shared_game.h"

#include "universal/universal.h"

static struct GameSystem s_gameSystem;

static void Tick(float elapsedSeconds)
{
	struct ComponentArray* requiredComponents[] =
	{
		&s_chests,
		&s_healths
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, KQ_ARRAY_COUNT(requiredComponents));

	EntityId_t entId;
	while (NextEntityId(&entIt, &entId))
	{
		struct Health* health = FindComponent(&s_healths, entId);
		if(health->bombHits > 0)
		{
			DeleteLater(entId);
		}
	}
}

struct GameSystem* AcquireChestSystem()
{
	s_gameSystem.tick = &Tick;
	return &s_gameSystem;
}

