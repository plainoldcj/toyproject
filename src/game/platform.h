#pragma once

#ifdef KQ_PLATFORM_WIN
	#include <windows.h>

	#define KQ_MAX_PATH MAX_PATH
#else
	#define KQ_MAX_PATH 1024
#endif
