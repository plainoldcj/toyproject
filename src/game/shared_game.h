#pragma once

#include "entity.h"
#include "renderer.h"

struct Rect;

// Physics time-step in seconds.
#define PHYS_DT 0.001f

#define PLAYER_SHRINK 0.1f

#define CB_ARRAY_COUNT(x) (sizeof(x)/sizeof(x[0]))

void GetCollisionRect(float posX, float posY, struct Rect* rect, float shrink);

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

struct Trigger
{
	bool isPlayerTouching;
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
extern struct ComponentArray s_triggers;

//==================================================
// Game Systems
//==================================================

#define FORALL_GAMESYSTEMS\
	FOR_GAMESYSTEM(DrawSystem)\
	FOR_GAMESYSTEM(CameraSystem)\
	FOR_GAMESYSTEM(PhysicsSystem)\
	FOR_GAMESYSTEM(TriggerSystem)

struct GameSystem
{
	void(*physicsTick)(void);
	void(*tick)(float elapsedSeconds);
	void(*draw)(void);

	struct GameSystem* next;
};

// Function prototypes

#define FOR_GAMESYSTEM(x) struct GameSystem* Acquire##x(void);
FORALL_GAMESYSTEMS
#undef FOR_GAMESYSTEM

//==================================================
// Shared Game
//==================================================

void Sh_Init(void);
void Sh_Shutdown(void);
void Sh_Tick(float elapsedTime);
void Sh_Draw(void);
