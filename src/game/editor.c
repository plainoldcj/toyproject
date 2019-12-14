#include "editor.h"

#include "entity.h"
#include "grid.h"
#include "renderer.h"
#include "shared_game.h"

#include <stdlib.h>

#define TILE_SIZE 1.0f

#define TILEMAP_SIZE 128

static struct
{
	hrobj_t grid;
	hrmesh_t tile;

	EntityId_t tilemap[TILEMAP_SIZE * TILEMAP_SIZE];
} s_ed;

static void Ed_CreateGrid()
{
	const int n = 16;

	const float gridX = -(n/2) * TILE_SIZE;
	const float gridY = -(n/2) * TILE_SIZE;

	int gridVertexCount;

	struct Vertex* vertices = CreateGrid(n, TILE_SIZE, gridX, gridY, &gridVertexCount);

	struct Mesh gridMesh;
	gridMesh.prim = ePrim_Lines;
	gridMesh.vertexCount = gridVertexCount;
	gridMesh.pos = &vertices->x;

	hrmesh_t rmesh = R_CreateMesh(&gridMesh);
	s_ed.grid = R_CreateObject(rmesh);

	R_ReleaseMesh(rmesh);

	free(vertices); // TODO(cj): Remove malloc.
}

static void CreateTile()
{
	const float size = TILE_SIZE;
	// TODO(cj): Index drawing maybe?
	struct Vertex vertices[] =
	{
		{ 0.0f, 0.0f },
		{ size, 0.0f },
		{ size, size },

		{ 0.0f, 0.0f },
		{ size, size },
		{ 0.0f, size }
	};

	struct Mesh mesh;
	mesh.prim = ePrim_Triangles;
	mesh.vertexCount = 6;
	mesh.pos = &vertices->x;

	s_ed.tile = R_CreateMesh(&mesh);
}

static void CreateTileEntity(float posX, float posY)
{
	EntityId_t entId = CreateEntity();

	struct Transform* transform = AddEntityComponent(&s_transforms, entId);
	struct Drawable* drawable = AddEntityComponent(&s_drawables, entId);

	transform->posX = posX;
	transform->posY = posY;

	drawable->hrobj = R_CreateObject(s_ed.tile);
}

void Ed_Init(void)
{
	Ed_CreateGrid();
	CreateTile();

	Sh_Init();

	CreateTileEntity(0.0f, 0.0f);
	CreateTileEntity(-1.0f, -1.0f);
}

void Ed_Shutdown(void)
{
	Sh_Shutdown();

	R_DestroyObject(s_ed.grid);
	R_ReleaseMesh(s_ed.tile);
}

void Ed_Tick(float elapsedSeconds)
{
	Sh_Tick(elapsedSeconds);
}

void Ed_Draw(void)
{
	Sh_Draw();

	R_DrawObject(s_ed.grid);
}
