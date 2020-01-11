#include "shared_game.h"

#include "entity.h"
#include "renderer.h"

#include <assert.h>

static struct GameSystem s_gameSystem;

static void Tick(float elapsedSeconds)
{
#if 0
	struct Transform* cameraTransform = FindComponent(&s_transforms, g_cameraEntity);
	struct Input* input = FindComponent(&s_inputs, g_activeInputEntity);
	assert(cameraTransform && input);

	const float delta = 2.0f * elapsedSeconds;

	if(input->buttons[BUTTON_LEFT])
	{
		cameraTransform->posX -= delta;
	}

	if(input->buttons[BUTTON_RIGHT])
	{
		cameraTransform->posX += delta;
	}

	if(input->buttons[BUTTON_UP])
	{
		cameraTransform->posY += delta;
	}

	if(input->buttons[BUTTON_DOWN])
	{
		cameraTransform->posY -= delta;
	}
#endif
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
