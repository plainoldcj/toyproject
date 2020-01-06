#pragma once

#include "entity.h"
#include "renderer.h"

// Physics time-step in seconds.
#define PHYS_DT 0.001f

//==================================================
// Components
//==================================================

__REFLECTED__
struct Transform
{
	float posX;
	float posY;

	float testpos[2];
	char teststring[256];
};

struct Drawable
{
	hrobj_t hrobj;
};

enum
{
	BUTTON_LEFT,
	BUTTON_UP,
	BUTTON_RIGHT,
	BUTTON_DOWN,

	BUTTON_COUNT
};

struct Input
{
	int buttons[BUTTON_COUNT];
};

//==================================================
// Well-known entities
//==================================================

extern EntityId_t g_activeInputEntity;
extern EntityId_t g_cameraEntity;
extern EntityId_t g_playerEntity;

//==================================================
// Component Arrays
//==================================================

struct ComponentArray;

extern struct ComponentArray s_transforms;
extern struct ComponentArray s_drawables;
extern struct ComponentArray s_inputs;
extern struct ComponentArray s_colliders;

//==================================================
// Game Systems
//==================================================

struct GameSystem
{
	void(*physicsTick)(void);
	void(*tick)(float elapsedSeconds);
	void(*draw)(void);

	struct GameSystem* next;
};

struct GameSystem* AcquireDrawSystem(void);
struct GameSystem* AcquireCameraSystem(void);
struct GameSystem* AcquirePhysicsSystem(void);

//==================================================
// Shared Game
//==================================================

void Sh_Init(void);
void Sh_Shutdown(void);
void Sh_Tick(float elapsedTime);
void Sh_Draw(void);
