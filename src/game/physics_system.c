#include "math.h"
#include "shared_game.h"

#include <assert.h>

#define TILE_SIZE 1.0f

/*
gravity -20.000000
playerJumpVelocity 11.042001
playerMovementVelocity 2.396000
playerDropBombTimeout 0.521000
bombLifetime 1.901000
*/

// TODO(cj): Move this to a component.
struct Player
{
	struct Vec2 accel;
	struct Vec2 vel;

	bool isWedged;
} g_player;

static struct GameSystem s_gameSystem;

static bool Player_IntersectsColliders(struct Vec2* outPen)
{
	struct Transform* playerTransform = FindComponent(&s_transforms, g_playerEntity);

	struct Rect playerCollisionRect;
	GetCollisionRect(playerTransform->posX, playerTransform->posY, &playerCollisionRect, PLAYER_SHRINK);

	struct Vec2 minPen;
	struct Vec2 maxPen;
	Vec2_SetF(&minPen, 0.0f, 0.0f);
	Vec2_SetF(&maxPen, 0.0f, 0.0f);

	struct ComponentArray* requiredComponents[] =
	{
		&s_transforms,
		&s_colliders
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, CB_ARRAY_COUNT(requiredComponents));

	EntityId_t entId;
	while (NextEntityId(&entIt, &entId))
	{
		struct Transform* const transform = FindComponent(&s_transforms, entId);

		struct Rect aabr;
		GetCollisionRect(transform->posX, transform->posY, &aabr, 0.0f);

		struct Vec2 pen;
		struct Vec2 sep;
		if (Rect_Intersect(&playerCollisionRect, &aabr, &pen))
		{
			Vec2_Copy(&sep, &pen);
			if (Absf(sep.x) < Absf(sep.y))
			{
				sep.y = 0.0f;
			}
			else
			{
				sep.x = 0.0f;
			}
			minPen.x = Minf(minPen.x, sep.x);
			maxPen.x = Maxf(maxPen.x, sep.x);
			minPen.y = Minf(minPen.y, sep.y);
			maxPen.y = Maxf(maxPen.y, sep.y);
		}
	}

	outPen->x = Absf(minPen.x) > Absf(maxPen.x) ? minPen.x : maxPen.x;
	outPen->y = Absf(minPen.y) > Absf(maxPen.y) ? minPen.y : maxPen.y;

	return outPen->x != 0.0f || outPen->y != 0.0f;
}


static void Player_GetInputVel(struct Vec2* vel)
{
	const float speed = 2.396000f; // g_gameConfig.playerMovementVelocity;

	Vec2_SetF(vel, 0.0f, 0.0f);

	const struct Input* input = FindComponent(&s_inputs, g_activeInputEntity);
	assert(input);

	if(input->buttons[BUTTON_LEFT])
	{
		vel->x -= speed;
	}

	if(input->buttons[BUTTON_RIGHT])
	{
		vel->x += speed;
	}

	/*
	if (g_input.left)
	{
		vel->x -= speed;
	}
	if (g_input.right)
	{
		vel->x += speed;
	}
	*/
}

static void UpdatePlayerGravity()
{
	struct Transform* playerTransform = FindComponent(&s_transforms, g_playerEntity);

	struct Vec2 gravity;
	Vec2_SetF(&gravity, 0.0f, -20.000000f /* g_gameConfig.gravity */);

	struct Vec2 oldPos;
	struct Vec2 oldVel;
	// Vec2_Copy(&oldPos, &playerTransform->pos);
	Vec2_SetF(&oldPos, playerTransform->posX, playerTransform->posY);
	Vec2_Copy(&oldVel, &g_player.vel);


	struct Vec2 a_dt;
	Vec2_Add(&a_dt, &g_player.accel, &gravity);
	Vec2_Mul(&a_dt, &a_dt, PHYS_DT);

	Vec2_Add(&g_player.vel, &g_player.vel, &a_dt);

	playerTransform->posX += PHYS_DT * g_player.vel.x;
	playerTransform->posY += PHYS_DT * g_player.vel.y;

	struct Vec2 inputVel;
	Player_GetInputVel(&inputVel);

	playerTransform->posX += PHYS_DT * inputVel.x;
	playerTransform->posY += PHYS_DT * inputVel.y;

	struct Vec2 pen;
	struct Vec2 sep;
	if (Player_IntersectsColliders(&pen))
	{
		Vec2_Copy(&sep, &pen);

		/*
		struct Vec2 tmp;
		Vec2_Add(&tmp, &playerTransform->pos, &sep);
		Vec2_Copy(&playerTransform->pos, &tmp);
		*/
		playerTransform->posX += sep.x;
		playerTransform->posY += sep.y;
	}
}

enum Probes
{
	Probes_Down,
	Probes_Up
};

static void Player_GetProbes(const struct Vec2* const playerPos, struct Vec2* outProbes, enum Probes probes)
{
	float shrink = PLAYER_SHRINK * 1.2f;

	const float leftProbeX = playerPos->x + shrink;
	const float rightProbeX = playerPos->x + TILE_SIZE - shrink;

	float probeY;
	
	if (probes == Probes_Up)
	{
		probeY = playerPos->y + TILE_SIZE + 0.01f;
	}
	else if (probes == Probes_Down)
	{
		probeY = playerPos->y - 0.01f;
	}

	Vec2_SetF(&outProbes[0], leftProbeX, probeY);
	Vec2_SetF(&outProbes[1], rightProbeX, probeY);
}

static void Player_GetProbesDown(const struct Vec2* const playerPos, struct Vec2* outProbes)
{
	Player_GetProbes(playerPos, outProbes, Probes_Down);
}

static void Player_GetProbesUp(const struct Vec2* const playerPos, struct Vec2* outProbes)
{
	Player_GetProbes(playerPos, outProbes, Probes_Up);
}

static bool Rect_ContainsPoint(struct Rect* const rect, struct Vec2* const point)
{
	return
		rect->lowerLeft.x <= point->x && point->x <= rect->upperRight.x &&
		rect->lowerLeft.y <= point->y && point->y <= rect->upperRight.y;
}

static bool Player_IsTouchingProbes(struct Vec2* probes)
{
	const float halfSize = TILE_SIZE * 0.5f;
	(void)halfSize; // TODO(cj)

	struct ComponentArray* requiredComponents[] =
	{
		&s_transforms,
		&s_colliders
	};
	struct EntityIterator entIt;
	InitEntityIterator(&entIt, requiredComponents, CB_ARRAY_COUNT(requiredComponents));

	EntityId_t entId;
	while (NextEntityId(&entIt, &entId))
	{
		struct Transform* const transform = FindComponent(&s_transforms, entId);

		struct Rect aabr;
		GetCollisionRect(transform->posX, transform->posY, &aabr, 0.0f);

		if (Rect_ContainsPoint(&aabr, &probes[0]) || Rect_ContainsPoint(&aabr, &probes[1]))
		{
			return true;
		}
	}

	return false;
}

static bool Player_IsTouchingDown()
{
	struct Transform* playerTransform = FindComponent(&s_transforms, g_playerEntity);

	struct Vec2 probes[2];

	struct Vec2 playerPos;
	playerPos.x = playerTransform->posX;
	playerPos.y = playerTransform->posY;
	Player_GetProbesDown(&playerPos, probes);

	return Player_IsTouchingProbes(probes);
}

static bool Player_IsTouchingUp()
{
	struct Transform* playerTransform = FindComponent(&s_transforms, g_playerEntity);

	struct Vec2 probes[2];

	struct Vec2 playerPos;
	playerPos.x = playerTransform->posX;
	playerPos.y = playerTransform->posY;
	Player_GetProbesUp(&playerPos, probes);

	return Player_IsTouchingProbes(probes);
}

static void UpdatePlayerPhysicsTick()
{
	bool isTouchingDown = Player_IsTouchingDown();
	bool isTouchingUp = Player_IsTouchingUp();

	if (isTouchingDown)
	{
		if (g_player.vel.y < 0.0f)
		{
			g_player.vel.y = 0.0f;
		}

		g_player.vel.y = 11.042001f; // g_gameConfig.playerJumpVelocity;
	}

	if (isTouchingUp)
	{
		if (g_player.vel.y > 0.0f)
		{
			g_player.vel.y = 0.0f;
		}
	}

	g_player.isWedged = isTouchingUp && isTouchingDown;
}

static void PhysicsTick()
{
	UpdatePlayerGravity();
	UpdatePlayerPhysicsTick();
}

static void DrawPlayerProbes(const struct Player* player)
{
#if 0
	struct Vec2 probes[2];

	struct Transform* playerTransform = FindComponent(&s_transforms, s_playerEntId);

	struct Vec2 playerPos;
	playerPos.x = playerTransform->posX;
	playerPos.y = playerTransform->posY;
	Player_GetProbesDown(&playerPos, probes);

	for (int i = 0; i < 2; ++i)
	{
		DrawPoint(&probes[i]);
	}

	Player_GetProbesUp(&playerTransform->pos, probes);

	for (int i = 0; i < 2; ++i)
	{
		DrawPoint(&probes[i]);
	}
#endif
}

static void DrawPlayerCollisionRect(const struct Player* player)
{
#if 0
	struct Transform* playerTransform = FindComponent(&s_transforms, s_playerEntId);

	struct Rect collisionRect;
	GetCollisionRect(playerTransform->posX, playerTransform->posY, &collisionRect, PLAYER_SHRINK);

	DrawRect(&collisionRect);
#endif
}

static void Draw()
{
	DrawPlayerProbes(&g_player);
	DrawPlayerCollisionRect(&g_player);
}

struct GameSystem* AcquirePhysicsSystem()
{
	s_gameSystem.physicsTick = &PhysicsTick;
	s_gameSystem.draw = &Draw;
	return &s_gameSystem;
}