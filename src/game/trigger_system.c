#include "common.h"
#include "math.h"
#include "shared_game.h"

static struct GameSystem s_gameSystem;

static void PhysicsTick()
{
	struct Transform* playerTransform = FindComponent(&s_transforms, g_playerEntity);

	struct Rect playerAabr;
	GetCollisionRect(playerTransform->posX, playerTransform->posY, &playerAabr, PLAYER_SHRINK);

	struct Vec2 pen;

	struct ComponentArray* requiredComponents[] =
	{
		&s_transforms,
		&s_triggers
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, KQ_ARRAY_COUNT(requiredComponents));

	EntityId_t entId;
	while (NextEntityId(&entIt, &entId))
	{
		struct Transform* transform = FindComponent(&s_transforms, entId);
		struct Trigger* trigger = FindComponent(&s_triggers, entId);

		struct Rect aabr;
		GetCollisionRect(transform->posX, transform->posY, &aabr, 0.0f);
		trigger->isPlayerTouching = Rect_Intersect(&playerAabr, &aabr, &pen);
	}
}

struct GameSystem* AcquireTriggerSystem()
{
	s_gameSystem.physicsTick = &PhysicsTick;
	return &s_gameSystem;
}
