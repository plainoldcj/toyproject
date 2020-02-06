#include "renderer.h"

#include "alloc.h"
#include "assets.h"
#include "common.h"
#include "math.h"
#include "tga_image.h"

#include "GL/glew.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REND_MESH_CAPACITY		2048
#define REND_TEXTURE_CAPACITY	2048
#define REND_MATERIAL_CAPACITY	2048
#define REND_OBJECT_CAPACITY	2048

#define REND_IMMEDIATE_BUFFER_CAPACITY 2048
#define REND_IMMEDIATE_BUFFER_DRAW_CALL_CAPACITY 128

// TODO(cj): Renderer should not need to know about asset system.
// Just feed shader sources directly.
// Some other system can read materials + shaders from disk.

#define GL_CALL(x) \
    do { \
        x; \
        GLint error; \
        while((error = glGetError())) { \
            COM_LogPrintf("ERROR - in %s:%d: calling %s failed with error 0x%X, '%s'.\n", \
                __FILE__, __LINE__, #x, error, gluErrorString(error)); \
            exit(1); \
        } \
    } while(false)

#define GL_CHECK_ERROR \
    do { \
        GLint error; \
        if((error = glGetError())) { \
            COM_LogPrintf("ERROR - in %s:%d: glGetError() == 0x%X, '%s'.\n", \
                __FILE__, __LINE__, error, gluErrorString(error)); \
            exit(1); \
        } \
    } while(false)

struct RendMesh
{
	uint16_t generation;

	struct RendMesh *prev, *next;
	struct Mesh mesh;

	struct Chunk vertChunk;

	uint16_t refCount;

	GLuint vbo;
	bool ready;
};

struct RendMaterial
{
	uint16_t	generation;
	uint16_t	next;

	char		vertShaderAsset[ASSET_PATH_LEN];
	char		fragShaderAsset[ASSET_PATH_LEN];

	int			alphaTestEnabled;
	int			alphaTestFunc;
	float		alphaTestRef;

	uint16_t	refCount;

	bool 		ready;

	GLuint		fragShader;
	GLuint		vertShader;
	GLuint		prog;

	uint16_t	diffuseTex;
};

// TODO(cj): Add debug name.
struct RendTexture
{
	uint16_t 		generation;
	uint16_t 		next;
	uint16_t		refCount;

	struct Chunk	imageChunk;

	struct Image	image;

	bool 			ready;

	GLuint			id;
	GLenum			internalFormat;
	GLenum			format;
};

// TODO(cj): Add debug name.

struct RendObject
{
	uint16_t	generation;

	uint16_t	rmesh;
	uint16_t	rmat;

	int16_t		next;

	float		posX;
	float		posY;
};

struct ImmBatch
{
	float	texCoordS;
	float	texCoordT;
	int16_t	count;
	bool	recording;
};

struct ImmDrawCall
{
	uint16_t irmat; // TODO(cj): Release materials.
	uint16_t first;
	uint16_t count;
};

struct ImmBuffer
{
	struct Vertex		vertices[REND_IMMEDIATE_BUFFER_CAPACITY];
	int16_t				vertexCount;

	struct ImmDrawCall	drawCalls[REND_IMMEDIATE_BUFFER_DRAW_CALL_CAPACITY];
	uint16_t			drawCallCount;
};

static struct
{
	uint16_t immMatDefault;
} s_config;

static struct
{
	struct Mat4 perspective;
	struct Mat4 orthographic;

	struct Vec2 cameraPos;

	GLuint gridVbo;
	int gridVertexCount;

	struct FLAlloc meshAttrAlloc;

	struct RendMesh rendMeshes[REND_MESH_CAPACITY];
	struct RendMesh* freeMeshes;
	struct RendMesh* usedMeshes;

	struct FLAlloc imageAlloc;

	struct RendTexture rendTextures[REND_TEXTURE_CAPACITY];
	int16_t freeTextures;

	struct RendMaterial rendMaterials[REND_MATERIAL_CAPACITY];
	int16_t freeMaterials;

	struct RendObject rendObjects[REND_OBJECT_CAPACITY];
	int16_t freeObjects;

	struct ImmBuffer	immBuf;
	struct ImmBatch		immBatch;
} s_rend;

static const int IN_POSITION = 0;
static const int IN_TEXCOORD = 1;

static void InitImmBatch(void)
{
	s_rend.immBatch.texCoordS = 0.0f;
	s_rend.immBatch.texCoordT = 0.0f;
	s_rend.immBatch.count = 0;
	s_rend.immBatch.recording = false;
}

static void InitGlew(void)
{
	GLenum error = glewInit();
	if(error != GLEW_OK)
	{
		COM_LogPrintf("Unable to initialize GLEW: %s", glewGetErrorString(error));
		exit(1);
	}

	if(!GLEW_VERSION_3_2)
	{
		COM_LogPrintf("OpenGL version 3.2 is not supported.");
		exit(1);
	}
}

static void Texture_CreateInternal(struct RendTexture* tex, void* pixelData) {
	GL_CALL(glGenTextures(1, &tex->id));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, tex->id));

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, tex->internalFormat, tex->image.width, tex->image.height, 0, tex->format,
				GL_UNSIGNED_BYTE, pixelData));

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR));

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
}

void Texture_Create(struct RendTexture* tex, uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, void* pixelData)
{
	tex->internalFormat = format;
	tex->format = format;

	Texture_CreateInternal(tex, pixelData);
}

void Texture_CreateFromImage(struct RendTexture* tex, const struct Image* image)
{
	GLenum format;

	switch (image->format)
	{
		case PixelFormat_RGB8:
			format = GL_RGB;
		case PixelFormat_RGBA8:
			format = GL_RGBA;
	}
	// TODO: error handling.
	
	Texture_Create(tex, image->width, image->height, format, format, image->pixelData);
}

void Texture_Bind(struct RendTexture* tex, uint32_t level)
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + level));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, tex->id));
}

static void InitTexture(struct RendTexture* rtex)
{
	Texture_CreateFromImage(rtex, &rtex->image);
}

static GLuint CreateShader(GLenum shaderType, const char* shaderSource, int length)
{
	GLuint shader = glCreateShader(shaderType);
	GL_CHECK_ERROR;
	if(!shader)
	{
		COM_LogPrintf("Unable to create shader");
		exit(1);
	}

	const char* source[] = { shaderSource };
	GL_CALL(glShaderSource(shader, 1, source, &length));

	GL_CALL(glCompileShader(shader));
	GLint compileStatus;
	GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus));
	if(compileStatus != GL_TRUE)
	{
		char logBuffer[2048] = { 0 };
		GL_CALL(glGetShaderInfoLog(shader, sizeof(logBuffer) - 1, NULL, logBuffer));

		COM_LogPrintf("Failed to compile shader: %s", logBuffer);
		exit(1);
	}

	return shader;
}

static void SetUniformInt(GLuint prog, const char* name, int val)
{
	GLint loc = glGetUniformLocation(prog, name);
	GL_CHECK_ERROR;
	if(loc < 0)
	{
		COM_LogPrintf("Cannot find uniform \"%s\"", name);
		exit(1);
	}
	GL_CALL(glUniform1i(loc, val));
}

static void SetUniformMat4(GLuint prog, const char* name, struct Mat4* m)
{
	GLint loc = glGetUniformLocation(prog, name);
	GL_CHECK_ERROR;
	if(loc < 0)
	{
		COM_LogPrintf("Cannot find uniform \"%s\"", name);
		exit(1);
	}
	GL_CALL(glUniformMatrix4fv(loc, 1, GL_TRUE /* Matrix is stored row-major */, &m->m00));
}

static void DestroyMesh(struct RendMesh* rmesh)
{
	// Remove from used list.
	// TODO(cj): Can we get rid of branching here?
	if(rmesh->next)
	{
		rmesh->next->prev = rmesh->prev;
	}
	if(rmesh->prev)
	{
		rmesh->prev->next = rmesh->next;
	}
	if(rmesh == s_rend.usedMeshes)
	{
		if(s_rend.usedMeshes->next)
		{
			s_rend.usedMeshes = s_rend.usedMeshes->next;
		}
		else
		{
			s_rend.usedMeshes = s_rend.usedMeshes->prev;
		}
	}

	// TODO(cj): Destroy vertex buffers.
	if(rmesh->ready)
	{
		GL_CALL(glDeleteBuffers(1, &rmesh->vbo));
		rmesh->ready = false;
	}

	// Add to free list.
	rmesh->next = s_rend.freeMeshes;
	s_rend.freeMeshes = rmesh;

	rmesh->generation++;
}

static void DestroyMeshes(void)
{
	for(uint16_t i = 0; i < REND_MESH_CAPACITY; ++i)
	{
		struct RendMesh* rmesh = &s_rend.rendMeshes[i];
		if(rmesh->refCount == 0)
		{
			DestroyMesh(rmesh);
		}
	}
}

void R_Init(int screenWidth, int screenHeight)
{
	InitGlew();

	COM_LogPrintf("OpenGL version: %s", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	float aspect = (float)screenWidth / (float)screenHeight;
	s_rend.perspective = M_CreatePerspective(DegToRad(45.0f), aspect, 0.1f, 100.0f);

	s_rend.orthographic = M_CreateOrthographic(0.0f, screenWidth, 0.0f, screenHeight, -1.0f, 1.0f);

	// Init render meshes.
	// TODO(cj): Get memory from arean/zone instead of mallocing it.
	FL_Init(&s_rend.meshAttrAlloc, malloc(4096), 4096);
	FL_Init(&s_rend.imageAlloc, malloc(8000000), 8000000);

	// Create initial list for render meshes.
	// TODO(cj): This is a dumb loop.
	for(uint16_t i = 0; i < REND_MESH_CAPACITY; ++i)
	{
		s_rend.rendMeshes[i].next = s_rend.freeMeshes;
		s_rend.freeMeshes = &s_rend.rendMeshes[i];
	}

	// TODO(cj): This is also a dumb loop.
	s_rend.rendObjects[0].next = -1;
	for(int16_t i = 1; i < REND_OBJECT_CAPACITY; ++i)
	{
		s_rend.rendObjects[i].next = i - 1;
	}
	s_rend.freeObjects = REND_OBJECT_CAPACITY - 1;

	// TODO(cj): Another dump loop.
	s_rend.rendMaterials[0].next = -1;
	for(int16_t i = 1; i < REND_MATERIAL_CAPACITY; ++i)
	{
		s_rend.rendMaterials[i].next = i - 1;
	}
	s_rend.freeMaterials = REND_MATERIAL_CAPACITY - 1;

	// TODO(cj): Another dump loop.
	s_rend.rendTextures[0].next = -1;
	for(int16_t i = 1; i < REND_TEXTURE_CAPACITY; ++i)
	{
		s_rend.rendTextures[i].next = i - 1;
	}
	s_rend.freeTextures = REND_TEXTURE_CAPACITY - 1;

	s_rend.immBuf.vertexCount = 0;
	s_rend.immBuf.drawCallCount = 0;

	InitImmBatch();
}

// TODO(cj): Rename to Deinit for consistency reasons.
void R_Shutdown(void)
{
	DestroyMeshes();
}

void R_SetCameraPosition(float posX, float posY)
{
	s_rend.cameraPos.x = posX;
	s_rend.cameraPos.y = posY;
}

hrmesh_t R_CreateMesh(const struct Mesh* mesh)
{
	if(!s_rend.freeMeshes)
	{
		hrmesh_t handle;
		handle.index = 0;
		handle.generation = 0;
		return handle;
	}

	struct RendMesh* rmesh = s_rend.freeMeshes;

	// Remove from free list.
	s_rend.freeMeshes = s_rend.freeMeshes->next;

	// Add to used list.
	// TODO(cj): Can we get rid of branching here?
	rmesh->prev = NULL;
	rmesh->next = s_rend.usedMeshes;
	if(rmesh->next)
	{
		rmesh->next->prev = rmesh;
	}
	s_rend.usedMeshes = rmesh;

	rmesh->generation++;
	rmesh->ready = false;
	rmesh->refCount = 1;

	// Copy mesh vertex data.

	size_t vertSize = sizeof(struct Vertex) * mesh->vertexCount;
	rmesh->vertChunk = FL_Alloc(&s_rend.meshAttrAlloc, vertSize);
	memcpy(rmesh->vertChunk.mem, mesh->vertices, vertSize);

	// Copy mesh.
	rmesh->mesh.prim = mesh->prim;
	rmesh->mesh.vertexCount = mesh->vertexCount;
	rmesh->mesh.vertices = (struct Vertex*)rmesh->vertChunk.mem;

	hrmesh_t handle;
	handle.index = (uint16_t)(rmesh - s_rend.rendMeshes);
	handle.generation = rmesh->generation;
	return handle;
}

void R_AcquireMesh(hrmesh_t hrmesh)
{
	struct RendMesh* rmesh = &s_rend.rendMeshes[hrmesh.index];
	if(hrmesh.generation != rmesh->generation)
	{
		// TODO(cj): Error output.
		return;
	}

	++rmesh->refCount;
}

void R_ReleaseMesh(hrmesh_t hrmesh)
{
	struct RendMesh* rmesh = &s_rend.rendMeshes[hrmesh.index];
	if(hrmesh.generation != rmesh->generation)
	{
		// TODO(cj): Error output.
		return;
	}

	--rmesh->refCount;
}

static GLenum GetPrimitiveType(enum Prim prim)
{
	switch(prim)
	{
	case ePrim_Lines:
		return GL_LINES;
	case ePrim_Triangles:
		return GL_TRIANGLES;
	default:
		// TODO(cj): Error
		return GL_TRIANGLES;
	};
}

void R_DrawMesh(struct RendMesh* rmesh)
{
	if(!rmesh->ready)
	{
		// TODO(cj): Restore previous binding.
		GL_CALL(glGenBuffers(1, &rmesh->vbo));
		GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, rmesh->vbo));
		GL_CALL(glBufferData(GL_ARRAY_BUFFER,
			rmesh->mesh.vertexCount * sizeof(struct Vertex),
			rmesh->mesh.vertices,
			GL_STATIC_DRAW));

		rmesh->ready = true;
	}

	// TODO(cj): Draw should just mark the mesh for rendering.
	// Draw calls should be issued in a single place.
	// TODO(cj): Restore previous binding.
	glEnableVertexAttribArray(IN_POSITION);
	glEnableVertexAttribArray(IN_TEXCOORD);

	glBindBuffer(GL_ARRAY_BUFFER, rmesh->vbo);

	glVertexAttribPointer(IN_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, pos));
	glVertexAttribPointer(IN_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, texCoord));

	glDrawArrays(GetPrimitiveType(rmesh->mesh.prim), 0, rmesh->mesh.vertexCount);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(IN_TEXCOORD);
	glDisableVertexAttribArray(IN_POSITION);
}

hrtex_t R_CreateTexture(const struct Image* image)
{
	if(s_rend.freeTextures == -1)
	{
		hrtex_t handle;
		handle.index = 0;
		handle.generation = 0;
		return handle;
	}

	struct RendTexture* rtex = &s_rend.rendTextures[s_rend.freeTextures];

	// Update free list.
	s_rend.freeTextures = rtex->next;
	rtex->next = -1;

	rtex->generation++;
	rtex->refCount = 1;
	rtex->ready = false;

	// Copy image.

	int bytesPerPixel = -1;
	switch(image->format)
	{
	case PixelFormat_RGB8:
		bytesPerPixel = 3;
		break;
	case PixelFormat_RGBA8:
		bytesPerPixel = 4;
		break;
	default:
		COM_LogPrintf("Unknown image format %d", image->format);
		exit(1);
		break;
	}

	size_t imageSize = image->width * image->height * bytesPerPixel;
	rtex->imageChunk = FL_Alloc(&s_rend.imageAlloc, imageSize);

	memcpy(rtex->imageChunk.mem, image->pixelData, imageSize);

	rtex->image.width = image->width;
	rtex->image.height = image->height;
	rtex->image.format = image->format;
	rtex->image.pixelData = rtex->imageChunk.mem;

	hrtex_t hrtex;
	hrtex.index = (uint16_t)(rtex - s_rend.rendTextures);
	hrtex.generation = rtex->generation;
	return hrtex;
}

void R_DestroyTexture(hrtex_t hrtex)
{
	// TODO(cj)
}

hrmat_t R_CreateMaterial(const struct Material* material)
{
	if(s_rend.freeMaterials == -1)
	{
		hrmat_t handle;
		handle.index = 0;
		handle.generation = 0;
		return handle;
	}

	struct RendTexture* rtex = &s_rend.rendTextures[material->diffuseTex.index];
	if(material->diffuseTex.generation != rtex->generation)
	{
		// TODO(cj): Error output.
		hrmat_t handle;
		handle.index = 0;
		handle.generation = 0;
		return handle;
	}

	struct RendMaterial* rmat = &s_rend.rendMaterials[s_rend.freeMaterials];

	// Update free list.
	s_rend.freeMaterials = rmat->next;
	rmat->next = -1;

	rmat->generation++;
	rmat->refCount = 1;
	rmat->ready = false;

	rmat->diffuseTex = material->diffuseTex.index;

	// TODO(cj): Trailing 0?
	memset(rmat->vertShaderAsset, 0, ASSET_PATH_LEN);
	memset(rmat->fragShaderAsset, 0, ASSET_PATH_LEN);
	snprintf(rmat->vertShaderAsset, ASSET_PATH_LEN, "%s", material->vertShader);
	snprintf(rmat->fragShaderAsset, ASSET_PATH_LEN, "%s", material->fragShader);

	rmat->alphaTestEnabled = material->alphaTestEnabled;
	rmat->alphaTestFunc = material->alphaTestFunc;
	rmat->alphaTestRef = material->alphaTestRef;

	hrmat_t hrmat;
	hrmat.index = (uint16_t)(rmat - s_rend.rendMaterials);
	hrmat.generation = rmat->generation;
	return hrmat;
}

void R_DestroyMaterial(hrmat_t hrmat)
{
	struct RendMaterial* rmat = &s_rend.rendMaterials[hrmat.index];
	if(hrmat.generation != rmat->generation)
	{
		// TODO(cj): Error output.
		return;
	}

// TODO(cj): Release GL resources.
#if 0
	glDeleteShader(s_rend.fragShader);
	glDeleteShader(s_rend.vertShader);
	glDeleteProgram(s_rend.prog);

	s_rend.fragShader = 0;
	s_rend.vertShader = 0;
	s_rend.prog = 0;
#endif

	rmat->next = s_rend.freeMaterials;
	s_rend.freeMaterials = (int16_t)(rmat - s_rend.rendMaterials);
}

hrobj_t R_CreateObject(hrmesh_t hrmesh)
{
	if(s_rend.freeObjects == -1)
	{
		hrobj_t handle;
		handle.index = 0;
		handle.generation = 0;
		return handle;
	}

	struct RendMesh* rmesh = &s_rend.rendMeshes[hrmesh.index];
	if(hrmesh.generation != rmesh->generation)
	{
		// TODO(cj): Error output.
		hrobj_t handle;
		handle.index = 0;
		handle.generation = 0;
		return handle;
	}

	struct RendObject* robj = &s_rend.rendObjects[s_rend.freeObjects];

	// Update free list.
	s_rend.freeObjects = robj->next;
	robj->next = -1;

	robj->generation++;
	robj->rmesh = hrmesh.index;
	robj->rmat = (uint16_t)-1;
	robj->posX = 0.0f;
	robj->posY = 0.0f;

	rmesh->refCount++;

	hrobj_t hrobj;
	hrobj.index = (uint16_t)(robj - s_rend.rendObjects);
	hrobj.generation = robj->generation;
	return hrobj;
}

void R_DestroyObject(hrobj_t hrobj)
{
	struct RendObject* robj = &s_rend.rendObjects[hrobj.index];
	if(hrobj.generation != robj->generation)
	{
		// TODO(cj): Error output.
		return;
	}

	struct RendMesh* rmesh = &s_rend.rendMeshes[robj->rmesh];
	rmesh->refCount--;

	robj->next = s_rend.freeObjects;
	s_rend.freeObjects = (int16_t)(robj - s_rend.rendObjects);
}

void R_SetObjectPos(hrobj_t hrobj, float x, float y)
{
	struct RendObject* robj = &s_rend.rendObjects[hrobj.index];
	if(hrobj.generation != robj->generation)
	{
		// TODO(cj): Error output.
		return;
	}

	robj->posX = x;
	robj->posY = y;
}

static uint16_t AcquireMaterial(hrmat_t hrmat)
{
	struct RendMaterial* rmat = &s_rend.rendMaterials[hrmat.index];
	if(hrmat.generation != rmat->generation)
	{
		// TODO(cj): Error output.
		return 0;
	}

	++rmat->refCount;

	return hrmat.index;
}

#if 0
static void ReleaseMaterial(uint16_t irmat) // TODO(cj): Make irmat for index to render material a convention?
{
	struct RendMaterial* rmat = &s_rend.rendMaterials[irmat];
	--rmat->refCount;
}
#endif

void R_SetObjectMaterial(hrobj_t hrobj, hrmat_t hrmat)
{
	struct RendObject* robj = &s_rend.rendObjects[hrobj.index];
	if(hrobj.generation != robj->generation)
	{
		// TODO(cj): Error output.
		return;
	}

	// TODO(cj): Release previously assigned material.

	robj->rmat = AcquireMaterial(hrmat);
}

static void InitMaterial(struct RendMaterial* rmat)
{
	struct RendTexture* rtex = &s_rend.rendTextures[rmat->diffuseTex];
	if(!rtex->ready)
	{
		InitTexture(rtex);
		rtex->ready = true;
	}

	struct Asset* fragShader = AcquireAsset(rmat->fragShaderAsset);

	rmat->fragShader = CreateShader(
		GL_FRAGMENT_SHADER,
		(const char*)Asset_GetData(fragShader),
		Asset_GetSize(fragShader));

	ReleaseAsset(fragShader);

	struct Asset* vertShader = AcquireAsset(rmat->vertShaderAsset);

	rmat->vertShader = CreateShader(
		GL_VERTEX_SHADER,
		(const char*)Asset_GetData(vertShader),
		Asset_GetSize(vertShader));

	ReleaseAsset(vertShader);

	rmat->prog = glCreateProgram();
	if(!rmat->prog)
	{
		COM_LogPrintf("Unable to create program.");
		exit(1);
	}

	GL_CALL(glAttachShader(rmat->prog, rmat->fragShader));
	GL_CALL(glAttachShader(rmat->prog, rmat->vertShader));

	GL_CALL(glLinkProgram(rmat->prog));
	GLint linkStatus;
	GL_CALL(glGetProgramiv(rmat->prog, GL_LINK_STATUS, &linkStatus));
	if(linkStatus != GL_TRUE)
	{
		char logBuffer[2048] = { 0 };
		GL_CALL(glGetProgramInfoLog(rmat->prog, sizeof(logBuffer) - 1, NULL, logBuffer));

		COM_LogPrintf("Failed to link program: %s", logBuffer);
		exit(1);
	}
}

void R_DrawObject(hrobj_t hrobj)
{
	struct RendObject* robj = &s_rend.rendObjects[hrobj.index];
	if(hrobj.generation != robj->generation)
	{
		// TODO(cj): Error output.
		return;
	}

	if(robj->rmat == (uint16_t)-1)
	{
		// TODO(cj): Error output.
		return;
	}

	struct RendMaterial* rmat = &s_rend.rendMaterials[robj->rmat];
	if(!rmat->ready)
	{
		InitMaterial(rmat);
		rmat->ready = true;
	}
	else
	{
		struct RendTexture* rtex = &s_rend.rendTextures[rmat->diffuseTex];
		Texture_Bind(rtex, 0);
	}

	GL_CALL(glUseProgram(rmat->prog));

	GL_CALL(glBindAttribLocation(rmat->prog, IN_POSITION, "aPos"));
	GL_CALL(glBindAttribLocation(rmat->prog, IN_TEXCOORD, "aTexCoord"));

	SetUniformMat4(rmat->prog, "uProjection", &s_rend.perspective);
	
	struct Mat4 modelView = M_CreateTranslation(
		robj->posX - s_rend.cameraPos.x,
		robj->posY - s_rend.cameraPos.y,
		-10.0f);

	SetUniformMat4(rmat->prog, "uModelView", &modelView);
	SetUniformInt(rmat->prog, "uDiffuseTex", 0);

	struct RendMesh* rmesh = &s_rend.rendMeshes[robj->rmesh];

	R_DrawMesh(rmesh);
}

void IMM_Begin(hrmat_t hrmat)
{
	assert(!s_rend.immBatch.recording);
	s_rend.immBatch.recording = true;

	uint16_t irmat = AcquireMaterial(hrmat);
	assert(irmat != 0);

	struct ImmBuffer* immBuf = &s_rend.immBuf;

	if(!immBuf->drawCallCount || immBuf->drawCalls[ immBuf->drawCallCount - 1 ].irmat != irmat)
	{
		if(immBuf->drawCallCount == REND_IMMEDIATE_BUFFER_DRAW_CALL_CAPACITY)
		{
			// TODO(cj): Print error.
			assert(false);
		}

		struct ImmDrawCall* drawCall = &immBuf->drawCalls[ immBuf->drawCallCount++ ];
		drawCall->irmat = irmat;
		drawCall->first = immBuf->vertexCount;
		drawCall->count = 0;
	}
}

void IMM_End(void)
{
	assert(s_rend.immBatch.recording);
	assert((s_rend.immBatch.count % 3) == 0);

	struct ImmBuffer* immBuf = &s_rend.immBuf;

	struct ImmDrawCall* drawCall = &immBuf->drawCalls[ immBuf->drawCallCount - 1 ];
	drawCall->count = s_rend.immBatch.count;

	InitImmBatch();
}

void IMM_Vertex(float x, float y)
{
	assert(s_rend.immBuf.vertexCount < REND_IMMEDIATE_BUFFER_CAPACITY);
	struct Vertex* vert = &s_rend.immBuf.vertices[s_rend.immBuf.vertexCount];
	++s_rend.immBuf.vertexCount;

	vert->pos[0] = x;
	vert->pos[1] = y;
	vert->texCoord[0] = s_rend.immBatch.texCoordS;
	vert->texCoord[1] = s_rend.immBatch.texCoordT;

	++s_rend.immBatch.count; // TODO(cj): I don't think we need this.
}

void IMM_TexCoord(float s, float t)
{
	assert(s_rend.immBatch.recording);
	s_rend.immBatch.texCoordS = s;
	s_rend.immBatch.texCoordT = t;
}

void R_SetConfig(const struct R_Config* conf)
{
	s_config.immMatDefault = AcquireMaterial(conf->immMatDefault);
}

static GLenum GetAlphaTestFunc(int alphaTestFunc)
{
	switch(alphaTestFunc)
	{
		case ALPHA_TEST_GEQUAL:
			return GL_GEQUAL;
		default:
			// TODO(cj): Error handling
			assert(false);
			return 0;
	};
}

static void ImmDraw(void)
{
	struct ImmBuffer* immBuf = &s_rend.immBuf;

	if(!immBuf->vertexCount) return;

	GLuint vbo;

	GL_CALL(glGenBuffers(1, &vbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER,
				immBuf->vertexCount * sizeof(struct Vertex),
				immBuf->vertices,
				GL_STATIC_DRAW));

	for(uint16_t drawCallIndex = 0; drawCallIndex < immBuf->drawCallCount; ++drawCallIndex)
	{
		struct ImmDrawCall* drawCall = &immBuf->drawCalls[ drawCallIndex ];

		// TODO(cj): Duplicated code.
		struct RendMaterial* rmat = &s_rend.rendMaterials[drawCall->irmat];
		if(!rmat->ready)
		{
			InitMaterial(rmat);
			rmat->ready = true;
		}
		else
		{
			struct RendTexture* rtex = &s_rend.rendTextures[rmat->diffuseTex];
			Texture_Bind(rtex, 0);
		}

		GL_CALL(glUseProgram(rmat->prog));

		GL_CALL(glBindAttribLocation(rmat->prog, IN_POSITION, "aPos"));
		GL_CALL(glBindAttribLocation(rmat->prog, IN_TEXCOORD, "aTexCoord"));


		SetUniformMat4(rmat->prog, "uProjection", &s_rend.orthographic);

		struct Mat4 ident = Mat4_CreateIdentity();
		SetUniformMat4(rmat->prog, "uModelView", &ident);
		SetUniformInt(rmat->prog, "uDiffuseTex", 0);

		if(rmat->alphaTestEnabled)
		{
			GL_CALL(glEnable(GL_ALPHA_TEST));
			GL_CALL(glAlphaFunc(GetAlphaTestFunc(rmat->alphaTestFunc), rmat->alphaTestRef));
		}

		// TODO(cj): Duplicated code.

		// TODO(cj): Draw should just mark the mesh for rendering.
		// Draw calls should be issued in a single place.
		// TODO(cj): Restore previous binding.
		glEnableVertexAttribArray(IN_POSITION);
		glEnableVertexAttribArray(IN_TEXCOORD);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glVertexAttribPointer(IN_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, pos));
		glVertexAttribPointer(IN_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, texCoord));

		glDrawArrays(GL_TRIANGLES, 0, immBuf->vertexCount);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(IN_TEXCOORD);
		glDisableVertexAttribArray(IN_POSITION);

		if(rmat->alphaTestEnabled)
		{
			GL_CALL(glDisable(GL_ALPHA_TEST));
		}
	}

	GL_CALL(glDeleteBuffers(1, &vbo));

	immBuf->vertexCount = 0;
	immBuf->drawCallCount = 0;
}

void R_BeginFrame(void)
{
	DestroyMeshes();

	glClear(GL_COLOR_BUFFER_BIT);
}

void R_EndFrame(void)
{
	ImmDraw();
}
