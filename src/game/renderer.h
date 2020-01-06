#pragma once

#include <stdint.h>

void R_Init(int screenWidth, int screenHeight);
void R_Shutdown(void);

void R_SetCameraPosition(float posX, float posY);

void R_Draw(void);

/*
================================================================
Render Mesh
================================================================
*/

typedef struct
{
	uint16_t index;
	uint16_t generation;
} hrmesh_t;

enum Prim
{
	ePrim_Lines,
	ePrim_Triangles
};

struct Vertex
{
	float pos[2];
	float texCoord[2];
};

#define VATT_POS		(1 << 0)
#define VATT_TEXCOORD	(1 << 1)

struct Mesh
{
	enum Prim		prim;
	int				vertexCount;
	int				attrib;
	struct Vertex*	vertices;
};

hrmesh_t	R_CreateMesh(const struct Mesh* mesh);
void		R_AcquireMesh(hrmesh_t hrmesh);
void		R_ReleaseMesh(hrmesh_t hrmesh);

/*
================================================================
Render Texture
================================================================
*/

typedef struct
{
	uint16_t index;
	uint16_t generation;
} hrtex_t;

struct Image;

hrtex_t R_CreateTexture(const struct Image* image);
void	R_DestroyTexture(hrtex_t hrtex);

/*
================================================================
Render Material
================================================================
*/

typedef struct
{
	uint16_t index;
	uint16_t generation;
} hrmat_t;

#define ASSET_PATH_LEN 64 // Put the limit in a common header somewhere.

struct Material
{
	char	vertShader[ASSET_PATH_LEN];
	char	fragShader[ASSET_PATH_LEN];

	hrtex_t diffuseTex;
};

hrmat_t	R_CreateMaterial(const struct Material* material);
void	R_DestroyMaterial(hrmat_t hrmat);

/*
================================================================
Render Object
================================================================
*/

typedef struct
{
	uint16_t index;
	uint16_t generation;
} hrobj_t;

hrobj_t	R_CreateObject(hrmesh_t hrmesh);
void	R_DestroyObject(hrobj_t hrobj);

void	R_SetObjectPos(hrobj_t hrobj, float x, float y);
void	R_SetObjectMaterial(hrobj_t hrobj, hrmat_t hrmat);

void	R_DrawObject(hrobj_t hrobj);
