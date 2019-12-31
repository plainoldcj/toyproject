#include "assets.h"

#include "common.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ASSET_PATH_LENGTH 128

struct Asset
{
	char		name[MAX_ASSET_PATH_LENGTH];
	uint8_t*	data;
	uint32_t	size;
};

static bool GetFilePath(const char* assetPath, char* buffer, size_t bufferSize)
{
	const char* assetBasePath = "/home/christianj/Projects/toyproject/assets";
	snprintf(buffer, bufferSize, "%s/%s", assetBasePath, assetPath);
	return true; // TODO(cj): Return false when buffer is too small.
}

static bool ReadAssetFile(const char* assetPath, struct Asset* asset)
{
	char filePath[MAX_ASSET_PATH_LENGTH];
	if (!GetFilePath(assetPath, filePath, MAX_ASSET_PATH_LENGTH))
	{
		COM_LogPrintf("ERROR: Cannot get file path for asset '%s'.", assetPath);
		return false;
	}

	FILE* file = fopen(filePath, "rb");
	if (file == 0)
	{
		COM_LogPrintf("Unable to asset file %s", filePath);
		return false;
	}

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	assert(!asset->data);
	asset->data = malloc(size);

	if (!asset->data)
	{
		COM_LogPrintf("Cannot allocate buffer of size %d for asset %s", size, filePath);

		fclose(file);
		return false;
	}

	asset->size = size;

	fread(asset->data, size, 1, file);

	fclose(file);

	return true;
}

struct Asset* AcquireAsset(const char* assetPath)
{
	// TODO
	struct Asset* asset = malloc(sizeof(struct Asset));
	(void)ReadAssetFile(assetPath, asset);
	return asset;
}

void ReleaseAsset(struct Asset* asset)
{
	// TODO
}

uint8_t* Asset_GetData(struct Asset* asset)
{
	return asset->data;
}

int32_t Asset_GetSize(struct Asset* asset)
{
	return asset->size;
}
