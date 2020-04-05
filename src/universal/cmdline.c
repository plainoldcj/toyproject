#include "cmdline.h"
#include "strsplit.h"

#include <string.h>

// TODO(cj): This must be unit tested!
const char* GetCommandLineOption(int argc, const char* argv[], const char* option)
{
	struct StrSplit strSplit;

	for(int i = 0; i < argc; ++i)
	{
		StrSplit_Init(&strSplit, argv[i], "=");

		if(StrSplit_Next(&strSplit)
			&& !strncasecmp(option, StrSplit_String(&strSplit), StrSplit_Size(&strSplit))
			&& StrSplit_Next(&strSplit))
		{
			return StrSplit_String(&strSplit);
		}
	}

	return NULL;
}
