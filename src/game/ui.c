#include "renderer.h"

#include "alloc.h"
#include "common.h"
#include "font_renderer.h"
#include "material_manager.h"
#include "math.h"
#include "ui.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static struct
{
	int		screenWidth;
	int		screenHeight;

	int		mouseX;
	int		mouseY;

	bool	mouseUp;
} s_ui;

void UI_Init(int screenWidth, int screenHeight)
{
	s_ui.screenWidth = screenWidth;
	s_ui.screenHeight = screenHeight;
}

void UI_Deinit(void)
{
}

void UI_SetMousePos(int screenX, int screenY)
{
	s_ui.mouseX = screenX;
	s_ui.mouseY = screenY;
}

void UI_SetMouseButtonUp(bool up)
{
	s_ui.mouseUp = up;
}

// Origin in lower left corner.
static void DrawButtonBackground(float x, float y, float w, float h)
{
	struct Vec2 pos[] =
	{
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f }
	};

	struct Vec2 texCoords[] =
	{
		{ 0.0f, 0.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f }
	};

	int indices[] = { 0, 1, 2, 0, 2, 3 };

	for(int i = 0; i < 4; ++i)
	{
		pos[i].x = x + pos[i].x * w;
		pos[i].y = y + pos[i].y * h;
	}

	IMM_Begin(Materials_Get(MAT_BUTTON));
	{
		for(int i = 0; i < 6; ++i)
		{
			int v = indices[i];
			IMM_TexCoord(texCoords[v].x, texCoords[v].y);
			IMM_Vertex(pos[v].x, pos[v].y);
		}
	}
	IMM_End();
}

bool UI_Button(float posX, float posY, const char* format, ...)
{
	struct SScope stack;
	BigStack_Begin(&stack);

	uint32_t bufferSize = 1024;
	char* buffer = BigStack_Alloc(bufferSize);
	memset(buffer, 0, bufferSize);

	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	FNT_Printf(posX, posY, "%s", buffer);
	const struct Rect* br = FNT_GetBoundingRect();

	bool r = false;

	struct Vec2 mousePos = { s_ui.mouseX, s_ui.screenHeight - s_ui.mouseY };
	if(Rect_ContainsPoint(br, &mousePos) && s_ui.mouseUp)
	{
		r = true;
	}

	DrawButtonBackground(br->lowerLeft.x, br->lowerLeft.y, br->upperRight.x - br->lowerLeft.x, br->upperRight.y - br->lowerLeft.y);

	FNT_Printf(posX, posY, "%s", buffer);

	BigStack_End(&stack);

	return r;
}
