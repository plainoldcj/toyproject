#include "common.h"
#include "shared_game.h"

static struct GameSystem s_gameSystem;

#define EXPLOSION_LIFETIME 0.2f

static void Tick(float elapsedSeconds)
{
	struct ComponentArray* requiredComponents[] =
	{
		&s_explosions
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, KQ_ARRAY_COUNT(requiredComponents));

	EntityId_t entId;
	while(NextEntityId(&entIt, &entId))
	{
		struct ExplosionComp* explosion = FindComponent(&s_explosions, entId);
		explosion->age += elapsedSeconds;
		if(explosion->age >= EXPLOSION_LIFETIME)
		{
			DeleteLater(entId);
		}
	}
}

struct GameSystem* AcquireExplosionSystem()
{
	s_gameSystem.tick = &Tick;
	return &s_gameSystem;
}
