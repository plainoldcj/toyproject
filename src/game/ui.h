#pragma once

void UI_Init(int screenWidth, int screenHeight);
void UI_Deinit(void);

void UI_SetMousePos(int screenX, int screenY);

void UI_Button(float posX, float posY, const char* format, ...);
