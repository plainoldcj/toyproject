#include "renderer.h"

#include "common.h"

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
"in vec4 aPos;\n"
"void main() { gl_Position = aPos; }";

static struct
{
	GLuint fragShader;
	GLuint vertShader;
	GLuint prog;
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

void R_Init(void)
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
	glBegin(GL_TRIANGLES);
	glVertexAttrib3f(IN_POSITION, -1.0f, -1.0f, 0.0f);
	glVertexAttrib3f(IN_POSITION, 1.0f, -1.0f, 0.0f);
	glVertexAttrib3f(IN_POSITION, 0.0f, 1.0f, 0.0f);
	glEnd();
}
