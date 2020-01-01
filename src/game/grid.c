#include "grid.h"

#include "renderer.h"

#include <stdlib.h>

static void SetVertex(struct Vertex* v, float x, float y)
{
	v->pos[0] = x;
	v->pos[1] = y;
}

struct Vertex* CreateGrid(int n, float cellSize, float x, float y, int* vertexCount)
{
	const float size = n * cellSize;

	*vertexCount = 4 * (n+1);

	struct Vertex* vertices = malloc(sizeof(struct Vertex) * (*vertexCount));

	struct Vertex* it = vertices;
	for(int i = 0; i < n + 1; ++i)
	{
		SetVertex( it++, x + 0.0f, y + i * cellSize);
		SetVertex( it++, x + size, y + i * cellSize);

		SetVertex( it++, x + i * cellSize, y + 0.0f);
		SetVertex( it++, x + i * cellSize, y + size);
	}

	return vertices;
}

