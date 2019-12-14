#pragma once

struct Vertex
{
	float x;
	float y;
};

// Draws a nxn-grid with origin in the lower left corner.
struct Vertex* CreateGrid(int n, float cellSize, float x, float y, int* vertexCount);

