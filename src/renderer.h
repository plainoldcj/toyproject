#pragma once

#include <stdint.h>

void R_Init(int screenWidth, int screenHeight);
void R_Shutdown(void);

void R_Draw(void);

typedef struct
{
	uint16_t index;
	uint16_t generation;
} hrmesh_t;

struct Mesh
{
	int vertexCount;
	float* pos;
};

hrmesh_t	R_CreateMesh(const struct Mesh* mesh);
void		R_DestroyMesh(hrmesh_t handle);

void		R_DrawMesh(hrmesh_t handle);
