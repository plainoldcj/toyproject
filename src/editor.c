#include "editor.h"

#include "grid.h"
#include "renderer.h"

#include <stdlib.h>

#define TILE_SIZE 1.0f

static struct
{
	hrobj_t grid;
	hrobj_t tile;
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

	R_DestroyMesh(rmesh);

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

	hrmesh_t rmesh = R_CreateMesh(&mesh);

	s_ed.tile = R_CreateObject(rmesh);

	R_DestroyMesh(rmesh);
}

void Ed_Init(void)
{
	Ed_CreateGrid();
	CreateTile();
}

void Ed_Shutdown(void)
{
	R_DestroyObject(s_ed.grid);
	R_DestroyObject(s_ed.tile);
}

void Ed_Draw(void)
{
	R_DrawObject(s_ed.grid);
	R_DrawObject(s_ed.tile);
}
