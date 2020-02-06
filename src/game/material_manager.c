#include "renderer.h"

#include "assets.h"
#include "material_manager.h"
#include "tga_image.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct ManagedMaterial
{
	int				id;
	const char*		tex;
	const char*		frag;
	hrmat_t			hrmat;
	bool			ready;
};

static struct ManagedMaterial s_materials[] =
{
	{ MAT_PLAYER, "player2.tga", "frag.glsl" },
	{ MAT_WALL, "wall.tga", "frag.glsl" },
	{ MAT_BOMB, "bomb.tga", "frag.glsl" },
	{ MAT_CHEST, "chest.tga", "frag.glsl" },
	{ MAT_EXPLOSION, "explosion.tga", "frag.glsl" },
	{ MAT_FONT, "Fonts/consolas_32_0.tga", "font_frag.glsl" }
};

static hrmat_t CreateMaterial(const char* tex, const char* frag)
{
	struct Asset* asset = AcquireAsset(tex);
	struct Image image;

	(void)LoadImageFromMemoryTGA(
		&image,
		Asset_GetData(asset),
		Asset_GetSize(asset));

	hrtex_t diffuseTex = R_CreateTexture(&image);

	struct Material mat;
	memset(&mat, 0, sizeof(struct Material));
	strcpy(mat.vertShader, "vert.glsl");
	strcpy(mat.fragShader, frag);
	mat.diffuseTex = diffuseTex;

	hrmat_t hrmat = R_CreateMaterial(&mat);

	R_DestroyTexture(diffuseTex);

	ReleaseAsset(asset);

	return hrmat;
}

hrmat_t Materials_Get(int mat)
{
	struct ManagedMaterial* mmat = s_materials;
	while(mmat->id != mat)
	{
		mmat++;
	}

	if(!mmat->ready)
	{
		mmat->hrmat = CreateMaterial(mmat->tex, mmat->frag);
		mmat->ready = true;
	}
	return mmat->hrmat;
}
