#include "renderer.h"

#include "common.h"

#include "GL/glew.h"

#include <stdlib.h>

// TODO(cj): Add error checking macro.
#define GL_CALL(x) x
#define GL_CHECK_ERROR

static const char* fragmentShaderSource = "void main() { gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }";

static struct
{
	GLuint fragShader;
	GLuint prog;
} s_rend;

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

void R_Init(void)
{
	InitGlew();

	COM_LogPrintf("OpenGL version: %s", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	s_rend.fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	GL_CHECK_ERROR;
	if(!s_rend.fragShader)
	{
		COM_LogPrintf("Unable to create shader");
		exit(1);
	}

	const char* source[] = { fragmentShaderSource };
	GL_CALL(glShaderSource(s_rend.fragShader, 1, source, NULL));

	GL_CALL(glCompileShader(s_rend.fragShader));
	GLint compileStatus;
	GL_CALL(glGetShaderiv(s_rend.fragShader, GL_COMPILE_STATUS, &compileStatus));
	if(compileStatus != GL_TRUE)
	{
		char logBuffer[2048] = { 0 };
		GL_CALL(glGetShaderInfoLog(s_rend.fragShader, sizeof(logBuffer) - 1, NULL, logBuffer));

		COM_LogPrintf("Failed to compile shader: %s", logBuffer);
		exit(1);
	}

	s_rend.prog = glCreateProgram();
	if(!s_rend.prog)
	{
		COM_LogPrintf("Unable to create program.");
		exit(1);
	}

	GL_CALL(glAttachShader(s_rend.prog, s_rend.fragShader));

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
}

void R_Shutdown(void)
{
	glDeleteShader(s_rend.fragShader);
	glDeleteProgram(s_rend.prog);

	s_rend.fragShader = 0;
	s_rend.prog = 0;
}

void R_Draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_TRIANGLES);
	glVertex3f(-1.0f, -1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glEnd();
}
