#pragma once

struct Rect;

void FNT_Init(void);
void FNT_Deinit(void);

// Offset in the upper-left hand corner.
void FNT_Printf(float posX, float posY, const char* format, ...);

const struct Rect* FNT_GetBoundingRect(void);


