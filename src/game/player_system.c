#include "renderer.h"

#include "game.h"
#include "math.h"
#include "material_manager.h"
#include "shared_game.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static struct GameSystem s_gameSystem;

void Rect_GetCenter(const struct Rect* rect, struct Vec2* outCenter)
{
	struct Vec2 size, halfSize;
	Vec2_Sub(&size, &rect->upperRight, &rect->lowerLeft);
	Vec2_Mul(&halfSize, &size, 0.5f);
	Vec2_Add(outCenter, &rect->lowerLeft, &halfSize);
}

void GetBombPosition(const struct Vec2* playerCenter, struct Vec2* outPos)
{
	struct Vec2 pos;
	Vec2_Copy(&pos, playerCenter);
	if (pos.x < 0.0f)
	{
		pos.x -= 1.0f;
	}
	if (pos.y < 0.0f)
	{
		pos.y -= 1.0f;
	}
	pos.x = (float)(int)pos.x;
	pos.y = (float)(int)pos.y;
	Vec2_Copy(outPos, &pos);
}

static void SpawnBomb(const struct Vec2* playerPos)
{
	struct Rect playerAabr;
	struct Vec2 playerCenter;

	GetCollisionRect(playerPos->x, playerPos->y, &playerAabr, PLAYER_SHRINK);
	Rect_GetCenter(&playerAabr, &playerCenter);

	EntityId_t entId = CreateEntity();

	struct Transform* transform = AddEntityComponent(&s_transforms, entId);
	struct Bomb* newBomb = AddEntityComponent(&s_bombs, entId);
	struct Drawable* drawable = AddEntityComponent(&s_drawables, entId);
	struct Trigger* trigger = AddEntityComponent(&s_triggers, entId);
	struct Tile* tile = AddEntityComponent(&s_tiles, entId);
	struct Health* health = AddEntityComponent(&s_healths, entId);

	memset(health, 0, sizeof(struct Health));

	// TODO(cj): We know that bombs are spawned at the player position and thus touch the player.
	// We need to initialize this field because bombs are spawned during the player system tick
	// and immediately handled during the bomb tick with no triggers resolved yet.
	// This is a critical update order issue. Maybe we can defer entity creation?
	trigger->isPlayerTouching = true;

	struct Vec2 bombPos;
	GetBombPosition(&playerCenter, &bombPos);
	newBomb->age = 0.0f;
	newBomb->chain = 0.0f;
	transform->posX = bombPos.x;
	transform->posY = bombPos.y;

	tile->row = (uint16_t)bombPos.y;
	tile->col = (uint16_t)bombPos.x;
	printf("create at %d, %d\n", tile->row, tile->col);
	g_tilemap.tiles[g_tilemap.rowCount * tile->row + tile->col] = entId;

	// AcquireMaterial(&drawable->material, eMaterialId_Bomb);
	// drawable->posZ = 0.0f;
	drawable->hrobj = R_CreateObject(GetTileMesh());
	R_SetObjectMaterial(drawable->hrobj, Materials_Get(MAT_BOMB));
}

static float Player_GetInputVel()
{
	const float speed = 2.396000f; // g_gameConfig.playerMovementVelocity;

	float velX = 0.0f;

	const struct Input* input = FindComponent(&s_inputs, g_activeInputEntity);
	assert(input);

	if(input->buttons[BUTTON_LEFT])
	{
		velX -= speed;
	}

	if(input->buttons[BUTTON_RIGHT])
	{
		velX += speed;
	}

	return velX;
}

static void Player_Tick(float elapsedSeconds)
{
	struct Player* player = FindComponent(&s_players, g_playerEntity);

	player->bombTimeout -= elapsedSeconds;
	if (player->bombTimeout < 0.0f)
	{
		player->bombTimeout = 0.0f;
	}

	const struct Input* input = FindComponent(&s_inputs, g_activeInputEntity);
	assert(input);

	if (player->bombTimeout == 0.0f && input->buttons[BUTTON_DROP_BOMB])
	{
		struct Transform* playerTransform = FindComponent(&s_transforms, g_playerEntity);

		struct Vec2 playerPos = { playerTransform->posX, playerTransform->posY };
		SpawnBomb(&playerPos);

		player->bombTimeout = 0.5f /* g_gameConfig.playerDropBombTimeout */;
	}

	player->inputVelX = Player_GetInputVel();
}

struct GameSystem* AcquirePlayerSystem()
{
	s_gameSystem.tick = &Player_Tick;
	return &s_gameSystem;
}
