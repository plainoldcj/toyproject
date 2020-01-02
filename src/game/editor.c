#include "editor.h"

#include "assets.h"
#include "entity.h"
#include "grid.h"
#include "math.h"
#include "renderer.h"
#include "shared_game.h"
#include "tga_image.h"

#include <stdlib.h>
#include <string.h>

#define TILE_SIZE 1.0f

#define TILEMAP_SIZE 128

static struct
{
	hrobj_t grid;
	hrmesh_t tile;
	hrmat_t mat;

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
	gridMesh.attrib = VATT_POS;
	gridMesh.vertexCount = gridVertexCount;
	gridMesh.vertices = vertices;

	hrmesh_t rmesh = R_CreateMesh(&gridMesh);
	s_ed.grid = R_CreateObject(rmesh);

	R_SetObjectMaterial(s_ed.grid, s_ed.mat);

	R_ReleaseMesh(rmesh);

	free(vertices); // TODO(cj): Remove malloc.
}

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

	s_ed.tile = R_CreateMesh(&mesh);

	free(vertices); // TODO(cj): Remove malloc.
}

static void CreateTileEntity(float posX, float posY)
{
	EntityId_t entId = CreateEntity();

	struct Transform* transform = AddEntityComponent(&s_transforms, entId);
	struct Drawable* drawable = AddEntityComponent(&s_drawables, entId);

	transform->posX = posX;
	transform->posY = posY;

	drawable->hrobj = R_CreateObject(s_ed.tile);

	R_SetObjectMaterial(drawable->hrobj, s_ed.mat);
}

static void CreateMaterial()
{
	struct Asset* asset = AcquireAsset("wall.tga");
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

	s_ed.mat = R_CreateMaterial(&mat);

	R_DestroyTexture(diffuseTex);

	ReleaseAsset(asset);
}

void Ed_Init(void)
{
	CreateMaterial();
	Ed_CreateGrid();
	CreateTile();

	Sh_Init();

	CreateTileEntity(0.0f, 0.0f);
	CreateTileEntity(-1.0f, -1.0f);
}

void Ed_Shutdown(void)
{
	Sh_Shutdown();

	R_DestroyMaterial(s_ed.mat);
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
