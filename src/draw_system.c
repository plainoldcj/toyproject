#include "renderer.h"

#include "shared_game.h"

#include "common.h"
#include "entity.h"

static struct GameSystem s_gameSystem;

static void Tick(float elapsedSeconds)
{
}

static void Draw()
{
	struct ComponentArray* requiredComponents[] =
	{
		&s_transforms,
		&s_drawables
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, CB_ARRAY_COUNT(requiredComponents));

	EntityId_t entId;
	while (NextEntityId(&entIt, &entId))
	{
		struct Transform* const transform = FindComponent(&s_transforms, entId);
		struct Drawable* const drawable = FindComponent(&s_drawables, entId);
		(void)transform;

		R_DrawObject(drawable->hrobj);

#if 0
		pos.x = transform->pos.x;
		pos.y = transform->pos.y;
		pos.z = drawable->posZ;
		DrawQuad(&pos, &drawable->material);
#endif
	}
}

struct GameSystem* AcquireDrawSystem()
{
	s_gameSystem.tick = &Tick;
	s_gameSystem.draw = &Draw;
	return &s_gameSystem;
}
