#pragma once

#include <stdint.h>

struct Asset;

struct Asset*	AcquireAsset(const char* path);
void			ReleaseAsset(struct Asset* asset);

uint8_t*		Asset_GetData(struct Asset* asset);
int32_t			Asset_GetSize(struct Asset* asset);
