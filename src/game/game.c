#include "game.h"

#include "assets.h"
#include "common.h"
#include "entity.h"
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

	hrmesh_t tile;

	hrmat_t playerMat;
	hrmat_t wallMat;
	hrmat_t bombMat;

	EntityId_t tilemap[MAX_TILE_COUNT];
} s_game;

// BEGIN duplicated code from editor.c

#define TILE_SIZE 1.0f

static void CreateTile()
{
	const float size = TILE_SIZE;
	// TODO(cj): Index drawing maybe?
	struct Vec2 pos[] =
	{
		{ 0.0f, 0.0f },
		{ size, 0.0f },
		{ size, size },

		{ 0.0f, 0.0f },
		{ size, size },
		{ 0.0f, size }
	};

	struct Vec2 texCoord[] =
	{
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },

		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f }
	};

	struct Vertex* vertices = malloc(sizeof(struct Vertex) * 6);

	for(int i = 0; i < 6; ++i)
	{
		vertices[i].pos[0] = pos[i].x;
		vertices[i].pos[1] = pos[i].y;

		vertices[i].texCoord[0] = texCoord[i].x;
		vertices[i].texCoord[1] = texCoord[i].y;
	}

	struct Mesh mesh;
	mesh.prim = ePrim_Triangles;
	mesh.attrib = VATT_POS | VATT_TEXCOORD;
	mesh.vertexCount = 6;
	mesh.vertices = vertices;

	s_game.tile = R_CreateMesh(&mesh);

	free(vertices); // TODO(cj): Remove malloc.
}

static void CreateTileEntity(float posX, float posY)
{
	EntityId_t entId = CreateEntity();

	struct Transform* transform = AddEntityComponent(&s_transforms, entId);
	struct Drawable* drawable = AddEntityComponent(&s_drawables, entId);

	(void)AddEntityComponent(&s_colliders, entId);

	transform->posX = posX;
	transform->posY = posY;

	drawable->hrobj = R_CreateObject(s_game.tile);

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

	drawable->hrobj = R_CreateObject(s_game.tile);

	R_SetObjectMaterial(drawable->hrobj, s_game.playerMat);

	g_playerEntity = entId;
}

static hrmat_t CreateMaterial(const char* tex)
{
	struct Asset* asset = AcquireAsset(tex);
	struct Image image;

	(void)LoadImageFromMemoryTGA(
		&image,
		Asset_GetData(asset),
		Asset_GetSize(asset));

	hrtex_t diffuseTex = R_CreateTexture(&image);

	struct Material mat;
	memset(&mat, 0, sizeof(struct Material));
	strcpy(mat.vertShader, "vert.glsl");
	strcpy(mat.fragShader, "frag.glsl");
	mat.diffuseTex = diffuseTex;

	hrmat_t hrmat = R_CreateMaterial(&mat);

	R_DestroyTexture(diffuseTex);

	ReleaseAsset(asset);

	return hrmat;
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

	for(int row = 0; row < map->rowCount; ++row)
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

void G_SetBomb(struct Drawable* drawable)
{
	drawable->hrobj = R_CreateObject(s_game.tile);

	R_SetObjectMaterial(drawable->hrobj, s_game.bombMat);
}

void G_Init(void)
{
	s_game.playerMat = CreateMaterial("player2.tga");
	s_game.wallMat = CreateMaterial("wall.tga");
	s_game.bombMat = CreateMaterial("bomb.tga");
	CreateTile();

	Sh_Init();

	LoadMap(&s_game.mapDesc, "map0.txt");

	InstantiateMap(&s_game.mapDesc);
}

void G_Shutdown(void)
{
	Sh_Shutdown();

	R_DestroyMaterial(s_game.wallMat);
	R_ReleaseMesh(s_game.tile);
}

void G_Tick(float elapsedSeconds)
{
	Sh_Tick(elapsedSeconds);
}

void G_Draw(void)
{
	Sh_Draw();
}
