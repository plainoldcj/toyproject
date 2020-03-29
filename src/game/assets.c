#include "assets.h"

#include "common.h"
#include "platform.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>

#define MAX_ASSET_PATH_LENGTH 128

static struct
{
	char projectRoot[KQ_MAX_PATH];
} s_assets;

struct Asset
{
	char		name[MAX_ASSET_PATH_LENGTH];
	uint8_t*	data;
	uint32_t	size;
};

static bool FindRootDirectory(void)
{
	const char* const rootFiles[] =
	{
		".gitignore",
		".git"
	};

	char relative[256] = ".";
	int cur = 1;

	const int rootFileCount = (int)KQ_ARRAY_COUNT(rootFiles);

	bool done = false;

	for(int step = 0; step < 4; ++step)
	{
		DIR* dir = opendir(relative);
		if(!dir)
		{
			return false;
		}

		int filesFound = 0;

		struct dirent* dirEnt;
		while((dirEnt = readdir(dir)))
		{
			for(int i = 0; i < rootFileCount; ++i)
			{
				if(!strcmp(dirEnt->d_name, rootFiles[i]))
				{
					++filesFound;
					break;
				}
			}
		}

		if(filesFound == rootFileCount)
		{
			done = true;
			break;
		}

		if(cur + 4 >= sizeof(relative))
		{
			return false;
		}

		sprintf(relative + cur, "/..");
		cur += 3;
	}

	if(!done)
	{
		return false;
	}

	if(!realpath(relative, s_assets.projectRoot))
	{
		return false;
	}

	return true;
}

void InitAssets(void)
{
	if(FindRootDirectory())
	{
		COM_LogPrintf("Found project root: '%s'", s_assets.projectRoot);
	}
	else
	{
		COM_LogPrintf("Unable to find project root.");
		exit(-1);
	}
}

void DeinitAssets(void)
{
}

static bool GetFilePath(const char* assetPath, char* buffer, size_t bufferSize)
{
	snprintf(buffer, bufferSize, "%s/assets/%s", s_assets.projectRoot, assetPath);
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
	memset(asset, 0, sizeof(struct Asset));
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

const char* GetProjectRoot(void)
{
	return s_assets.projectRoot;
}
