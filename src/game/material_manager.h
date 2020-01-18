#pragma once

enum
{
	MAT_BOMB,
	MAT_PLAYER,
	MAT_WALL,
	MAT_CHEST,

	MAT_COUNT
};

hrmat_t Materials_Get(int mat);
