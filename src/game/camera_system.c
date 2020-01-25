#include "shared_game.h"

#include "entity.h"
#include "renderer.h"

#include <assert.h>

static struct GameSystem s_gameSystem;

static void Tick(float elapsedSeconds)
{
	struct Transform* playerTransform = FindComponent(&s_transforms, g_playerEntity);
	assert(playerTransform);

	struct Transform* cameraTransform = FindComponent(&s_transforms, g_cameraEntity);
	assert(cameraTransform);

	cameraTransform->posX = playerTransform->posX;
	cameraTransform->posY = playerTransform->posY;
}

static void Draw(void)
{
	struct Transform* cameraTransform = FindComponent(&s_transforms, g_cameraEntity);
	assert(cameraTransform);

	R_SetCameraPosition(cameraTransform->posX, cameraTransform->posY);
}

struct GameSystem* AcquireCameraSystem(void)
{
	s_gameSystem.tick = &Tick;
	s_gameSystem.draw = &Draw;
	return &s_gameSystem;
}
