#pragma once

#define REFLECTED

//==================================================
// Components
//==================================================

REFLECTED struct Transform
{
	float posX, posY;
};

struct Drawable
{
	hrobj_t hrobj;
};

//==================================================
// Component Arrays
//==================================================

struct ComponentArray;

extern struct ComponentArray s_transforms;
extern struct ComponentArray s_drawables;

//==================================================
// Game Systems
//==================================================

struct GameSystem
{
	void(*tick)(float elapsedSeconds);
	void(*draw)();

	struct GameSystem* next;
};

struct GameSystem* AcquireDrawSystem();

//==================================================
// Shared Game
//==================================================

void Sh_Init(void);
void Sh_Shutdown(void);
void Sh_Tick(float elapsedTime);
void Sh_Draw(void);
