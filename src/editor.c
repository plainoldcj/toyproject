#include "editor.h"

#include "grid.h"
#include "renderer.h"

#include <stdlib.h>

#define TILE_SIZE 1.0f

static struct
{
	hrmesh_t gridMesh;
	hrobj_t gridObj;
} s_ed;

static void Ed_CreateGrid()
{
	const int n = 16;

	const float gridX = -(n/2) * TILE_SIZE;
	const float gridY = -(n/2) * TILE_SIZE;

	int gridVertexCount;

	struct Vertex* vertices = CreateGrid(n, TILE_SIZE, gridX, gridY, &gridVertexCount);

	struct Mesh gridMesh;
	gridMesh.vertexCount = gridVertexCount;
	gridMesh.pos = &vertices->x;

	s_ed.gridMesh = R_CreateMesh(&gridMesh);
	s_ed.gridObj = R_CreateObject(s_ed.gridMesh);

	free(vertices); // TODO(cj): Remove malloc.
}

void Ed_Init(void)
{
	Ed_CreateGrid();
}

void Ed_Shutdown(void)
{
	R_DestroyObject(s_ed.gridObj);
	R_DestroyMesh(s_ed.gridMesh);
}

void Ed_Draw(void)
{
	R_DrawObject(s_ed.gridObj);
}
