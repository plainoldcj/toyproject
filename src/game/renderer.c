#include "renderer.h"

#include "alloc.h"
#include "common.h"
#include "math.h"

#include "GL/glew.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define REND_MESH_CAPACITY 2048
#define REND_OBJECT_CAPACITY 2048

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

static const char* fragmentShaderSource = "void main() { gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }";
static const char* vertexShaderSource =
"#version 150\n"
"uniform mat4 uProjection;"
"uniform mat4 uModelView;"
"in vec4 aPos;\n"
"void main() { gl_Position = uProjection * uModelView * aPos; }";

struct RendMesh
{
	uint16_t generation;

	struct RendMesh *prev, *next;
	struct Mesh mesh;

	struct Chunk posChunk;

	uint16_t refCount;

	GLuint vbo;
	bool ready;
};

struct RendObject
{
	uint16_t	generation;

	uint16_t	rmesh;

	int16_t		next;

	float		posX;
	float		posY;
};

static struct
{
	GLuint fragShader;
	GLuint vertShader;
	GLuint prog;

	GLuint gridVbo;
	int gridVertexCount;

	struct FLAlloc meshAttrAlloc;

	struct RendMesh rendMeshes[REND_MESH_CAPACITY];
	struct RendMesh* freeMeshes;
	struct RendMesh* usedMeshes;

	struct RendObject rendObjects[REND_OBJECT_CAPACITY];
	int16_t freeObjects;
} s_rend;

static const int IN_POSITION = 0;

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

static GLuint CreateShader(GLenum shaderType, const char* shaderSource)
{
	GLuint shader = glCreateShader(shaderType);
	GL_CHECK_ERROR;
	if(!shader)
	{
		COM_LogPrintf("Unable to create shader");
		exit(1);
	}

	const char* source[] = { shaderSource };
	GL_CALL(glShaderSource(shader, 1, source, NULL));

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

	s_rend.fragShader = CreateShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	s_rend.vertShader = CreateShader(GL_VERTEX_SHADER, vertexShaderSource);

	s_rend.prog = glCreateProgram();
	if(!s_rend.prog)
	{
		COM_LogPrintf("Unable to create program.");
		exit(1);
	}

	GL_CALL(glAttachShader(s_rend.prog, s_rend.fragShader));
	GL_CALL(glAttachShader(s_rend.prog, s_rend.vertShader));

	GL_CALL(glLinkProgram(s_rend.prog));
	GLint linkStatus;
	GL_CALL(glGetProgramiv(s_rend.prog, GL_LINK_STATUS, &linkStatus));
	if(linkStatus != GL_TRUE)
	{
		char logBuffer[2048] = { 0 };
		GL_CALL(glGetProgramInfoLog(s_rend.prog, sizeof(logBuffer) - 1, NULL, logBuffer));

		COM_LogPrintf("Failed to link program: %s", logBuffer);
		exit(1);
	}

	GL_CALL(glUseProgram(s_rend.prog));

	GL_CALL(glBindAttribLocation(s_rend.prog, IN_POSITION, "aPos"));

	float aspect = (float)screenWidth / (float)screenHeight;
	struct Mat4 perspective = M_CreatePerspective(DegToRad(45.0f), aspect, 0.1f, 100.0f);

	SetUniformMat4(s_rend.prog, "uProjection", &perspective);

	// Init render meshes.
	// TODO(cj): Get memory from arean/zone instead of mallocing it.
	FL_Init(&s_rend.meshAttrAlloc, malloc(4096), 4096);

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
}

// TODO(cj): Rename to Deinit for consistency reasons.
void R_Shutdown(void)
{
	DestroyMeshes();

	glDeleteShader(s_rend.fragShader);
	glDeleteShader(s_rend.vertShader);
	glDeleteProgram(s_rend.prog);

	s_rend.fragShader = 0;
	s_rend.vertShader = 0;
	s_rend.prog = 0;
}

void R_Draw(void)
{
	DestroyMeshes();

	glClear(GL_COLOR_BUFFER_BIT);
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
	size_t vertSize = sizeof(float) * 2 * mesh->vertexCount;
	rmesh->posChunk = FL_Alloc(&s_rend.meshAttrAlloc, vertSize);
	memcpy(rmesh->posChunk.mem, mesh->pos, vertSize);

	// Copy mesh.
	rmesh->mesh.prim = mesh->prim;
	rmesh->mesh.vertexCount = mesh->vertexCount;
	rmesh->mesh.pos = (float*)rmesh->posChunk.mem;

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
			rmesh->mesh.vertexCount * sizeof(float) * 2,
			rmesh->mesh.pos,
			GL_STATIC_DRAW));

		rmesh->ready = true;
	}

	// TODO(cj): Draw should just mark the mesh for rendering.
	// Draw calls should be issued in a single place.
	// TODO(cj): Restore previous binding.
	glEnableVertexAttribArray(IN_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, rmesh->vbo);
	glVertexAttribPointer(IN_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GetPrimitiveType(rmesh->mesh.prim), 0, rmesh->mesh.vertexCount);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(IN_POSITION);
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

void R_DrawObject(hrobj_t hrobj)
{
	struct RendObject* robj = &s_rend.rendObjects[hrobj.index];
	if(hrobj.generation != robj->generation)
	{
		// TODO(cj): Error output.
		return;
	}
	
	struct Mat4 modelView = M_CreateTranslation(robj->posX, robj->posY, -5.0f);

	SetUniformMat4(s_rend.prog, "uModelView", &modelView);

	struct RendMesh* rmesh = &s_rend.rendMeshes[robj->rmesh];

	R_DrawMesh(rmesh);
}