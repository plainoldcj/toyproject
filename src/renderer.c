#include "renderer.h"

#include "common.h"
#include "math.h"

#include "GL/glew.h"

#include <stdbool.h>
#include <stdlib.h>

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

static struct
{
	GLuint fragShader;
	GLuint vertShader;
	GLuint prog;

	GLuint gridVbo;
	int gridVertexCount;
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

struct Vertex
{
	float x;
	float y;
	float z;
};

static void SetVertex(struct Vertex* v, float x, float y, float z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

// Draws a nxn-grid with origin in the lower left corner.
static struct Vertex* CreateGrid(int n, float cellSize, float x, float y, float z, int* vertexCount)
{
	const float size = n * cellSize;

	*vertexCount = 4 * (n+1);

	struct Vertex* vertices = malloc(sizeof(struct Vertex) * (*vertexCount));

	struct Vertex* it = vertices;
	for(int i = 0; i < n + 1; ++i)
	{
		SetVertex( it++, x + 0.0f, y + i * cellSize, z);
		SetVertex( it++, x + size, y + i * cellSize, z);

		SetVertex( it++, x + i * cellSize, y + 0.0f, z);
		SetVertex( it++, x + i * cellSize, y + size, z);
	}

	return vertices;
}

#define TILE_SIZE 1.0f

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

	const int n = 16;

	const float gridX = -(n/2) * TILE_SIZE;
	const float gridY = -(n/2) * TILE_SIZE;

	struct Vertex* vertices = CreateGrid(n, TILE_SIZE, gridX, gridY, 0.0f, &s_rend.gridVertexCount);

	// TODO(cj): Restore previous binding.
	GL_CALL(glGenBuffers(1, &s_rend.gridVbo));
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, s_rend.gridVbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, s_rend.gridVertexCount * sizeof(struct Vertex), vertices, GL_STATIC_DRAW));

	free(vertices);
}

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

	// TODO(cj): Restore previous binding.
	glEnableVertexAttribArray(IN_POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, s_rend.gridVbo);
	glVertexAttribPointer(IN_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glDrawArrays(GL_LINES, 0, s_rend.gridVertexCount);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(IN_POSITION);

#if 0
	const float z = -5.0f;
	glBegin(GL_TRIANGLES);
	glVertexAttrib3f(IN_POSITION, -1.0f, -1.0f, z);
	glVertexAttrib3f(IN_POSITION, 1.0f, -1.0f, z);
	glVertexAttrib3f(IN_POSITION, 0.0f, 1.0f, z);
	glEnd();
#endif
}
