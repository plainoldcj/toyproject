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

typedef struct
{
	uint16_t index;
	uint16_t generation;
} hrobj_t;

struct Mesh
{
	int vertexCount;
	float* pos;
};

hrmesh_t	R_CreateMesh(const struct Mesh* mesh);
void		R_DestroyMesh(hrmesh_t handle); // TODO(cj): Rename handle argument to hrmesh to be more consistent.

hrobj_t		R_CreateObject(hrmesh_t hrmesh);
void		R_DestroyObject(hrobj_t hrobj);

void		R_DrawObject(hrobj_t hrobj);
