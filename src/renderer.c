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

	GLuint vbo;
	bool ready;
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
	
	struct Mat4 modelView = M_CreateTranslation(0.0f, 0.0f, -5.0f);

	SetUniformMat4(s_rend.prog, "uModelView", &modelView);

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
}

// TODO(cj): Rename to Deinit for consistency reasons.
void R_Shutdown(void)
{
	glDeleteShader(s_rend.fragShader);
	glDeleteShader(s_rend.vertShader);
	glDeleteProgram(s_rend.prog);

	s_rend.fragShader = 0;
	s_rend.vertShader = 0;
	s_rend.prog = 0;
}

void R_Draw(void)
{
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

	// Copy mesh vertex data.
	size_t vertSize = sizeof(float) * 2 * mesh->vertexCount;
	rmesh->posChunk = FL_Alloc(&s_rend.meshAttrAlloc, vertSize);
	memcpy(rmesh->posChunk.mem, mesh->pos, vertSize);

	// Copy mesh.
	rmesh->mesh.vertexCount = mesh->vertexCount;
	rmesh->mesh.pos = (float*)rmesh->posChunk.mem;

	hrmesh_t handle;
	handle.index = (uint16_t)(rmesh - s_rend.rendMeshes);
	handle.generation = rmesh->generation;
	return handle;
}

void R_DestroyMesh(hrmesh_t handle)
{
	struct RendMesh* rmesh = &s_rend.rendMeshes[handle.index];
	if(handle.generation != rmesh->generation)
	{
		// TODO(cj): Error output.
		return;
	}

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

	// Add to free list.
	rmesh->next = s_rend.freeMeshes;
	s_rend.freeMeshes = rmesh;

	rmesh->generation++;
	rmesh->ready = 0;
}

void R_DrawMesh(hrmesh_t handle)
{
	struct RendMesh* rmesh = &s_rend.rendMeshes[handle.index];
	if(handle.generation != rmesh->generation)
	{
		// TODO(cj): Handle error.
		return;
	}
	
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
	glDrawArrays(GL_LINES, 0, rmesh->mesh.vertexCount);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(IN_POSITION);
}
