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

enum Prim
{
	ePrim_Lines,
	ePrim_Triangles
};

struct Mesh
{
	enum Prim prim;
	int vertexCount;
	float* pos;
	float* texCoord;
};

hrmesh_t	R_CreateMesh(const struct Mesh* mesh);
void		R_AcquireMesh(hrmesh_t hrmesh);
void		R_ReleaseMesh(hrmesh_t hrmesh);

hrobj_t		R_CreateObject(hrmesh_t hrmesh);
void		R_DestroyObject(hrobj_t hrobj);

void		R_SetObjectPos(hrobj_t hrobj, float x, float y);

void		R_DrawObject(hrobj_t hrobj);
