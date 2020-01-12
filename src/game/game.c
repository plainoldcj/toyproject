#include "game.h"
#include "renderer.h"

#include "assets.h"
#include "common.h"
#include "entity.h"
#include "material_manager.h"
#include "math.h"
#include "shared_game.h"
#include "tga_image.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TILE_COUNT 128

#define WALL_TILE 'x'
#define PLAYER_TILE 'p'

struct MapDesc
{
	char	tiles[MAX_TILE_COUNT];
	int		rowCount;
	int		colCount;
};

static struct
{
	struct MapDesc mapDesc;

	hrmat_t playerMat;
	hrmat_t wallMat;
	hrmat_t bombMat;

	EntityId_t tilemap[MAX_TILE_COUNT];
} s_game;

// BEGIN duplicated code from editor.c

static void CreateTileEntity(float posX, float posY)
{
	EntityId_t entId = CreateEntity();

	struct Transform* transform = AddEntityComponent(&s_transforms, entId);
	struct Drawable* drawable = AddEntityComponent(&s_drawables, entId);

	(void)AddEntityComponent(&s_colliders, entId);

	transform->posX = posX;
	transform->posY = posY;

	drawable->hrobj = R_CreateObject(GetTileMesh());

	R_SetObjectMaterial(drawable->hrobj, s_game.wallMat);
}

static void CreatePlayerEntity(float posX, float posY)
{
	EntityId_t entId = CreateEntity();

	struct Transform* transform = AddEntityComponent(&s_transforms, entId);
	struct Drawable* drawable = AddEntityComponent(&s_drawables, entId);
	struct Player* player = AddEntityComponent(&s_players, entId);

	memset(player, 0, sizeof(struct Player));

	transform->posX = posX;
	transform->posY = posY;

	drawable->hrobj = R_CreateObject(GetTileMesh());

	R_SetObjectMaterial(drawable->hrobj, s_game.playerMat);

	g_playerEntity = entId;
}

// END duplicated code

static const char* GetLine(const char* in, char* buffer, size_t bufferSize)
{
	// TODO(cj): Would not work with windows \r\n

	const char* it = in;
	while(*it != '\n' && *it != '\0')
	{
		++it;
	}

	size_t len = it - in + 1; // Length of line including terminating 0.
	assert(len < bufferSize);

	// TODO(cj): Error out if buffer is too small.
	memset(buffer, 0, bufferSize);
	snprintf(buffer, len, "%s", in);

	buffer[len - 1] = '\0';

	if(*it == '\0')
	{
		return NULL;
	}

	return it + 1;
}

// TODO(cj): Separate map reading and spawining of tiles.
static void LoadMap(struct MapDesc* map, const char* assetPath)
{
	// Load the map.
	struct Asset* asset = AcquireAsset(assetPath);
	if(!asset)
	{
		COM_LogPrintf("Cannot load map '%s'", assetPath);
		exit(1);
	}

	char line[256];
	const char* it = (const char*)Asset_GetData(asset);

	it = GetLine(it, line, sizeof(line));
	if(it == NULL)
	{
		COM_LogPrintf("Map file too short.");
		exit(1);
	}

	map->rowCount = atoi(line);

	it = GetLine(it, line, sizeof(line));
	if(it == NULL)
	{
		COM_LogPrintf("Map file too short.");
		exit(1);
	}

	map->colCount = atoi(line);

	if(map->rowCount * map->colCount >= MAX_TILE_COUNT)
	{
		COM_LogPrintf("Map is too large");
		exit(1);
	}

	for(int row = map->rowCount - 1; row >= 0; --row)
	{
		// Read next line.
		it = GetLine(it, line, sizeof(line));
		if(it == NULL)
		{
			COM_LogPrintf("Map file too short.");
			exit(1);
		}

		for(int col = 0; col < map->colCount; ++col)
		{
			map->tiles[col + map->rowCount * row] = line[col];
		}
	}

	ReleaseAsset(asset);
}

static void InstantiateMap(const struct MapDesc* map)
{
	for(int row = 0; row < map->rowCount; ++row)
	{
		for(int col = 0; col < map->colCount; ++col)
		{
			float posX = TILE_SIZE * col;
			float posY = TILE_SIZE * row;

			char ent = map->tiles[col + row * map->rowCount];

			if(ent == 'x')
			{
				CreateTileEntity(posX, posY);
			}
			else if(ent == 'p')
			{
				CreatePlayerEntity(posX, posY);
			}
		}
	}
}

void G_Init(void)
{
	s_game.playerMat = Materials_Get(MAT_PLAYER);
	s_game.wallMat = Materials_Get(MAT_WALL);
	s_game.bombMat = Materials_Get(MAT_BOMB);

	Sh_Init();

	LoadMap(&s_game.mapDesc, "map0.txt");

	InstantiateMap(&s_game.mapDesc);
}

void G_Shutdown(void)
{
	Sh_Shutdown();

	R_DestroyMaterial(s_game.wallMat);
}

void G_Tick(float elapsedSeconds)
{
	Sh_Tick(elapsedSeconds);
}

void G_Draw(void)
{
	Sh_Draw();
}
