#include "common.h"
#include "strsplit.h"

#include "universal/universal.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

struct StrSplitImpl
{
	const char* begin;
	const char* end;
	const char* sep;
};

// TODO(cj): Make a version that takes a str size parameter.
void StrSplit_Init(struct StrSplit* strSplit, const char* str, const char* sep)
{
	KQ_STATIC_ASSERT(sizeof(struct StrSplitImpl) <= sizeof(struct StrSplit));

	struct StrSplitImpl* impl = (struct StrSplitImpl*)strSplit;

	impl->begin = str;
	impl->end = str;
	impl->sep = sep;

	if(!impl->sep)
	{
		impl->sep = " \t\r\n";
	}
}

static bool IsSeparator(char c, const char* sep)
{
	while(*sep != '\0')
	{
		if(c == *sep)
		{
			return true;
		}
		++sep;
	}
	return false;
}

int StrSplit_Next(struct StrSplit* strSplit)
{
	struct StrSplitImpl* impl = (struct StrSplitImpl*)strSplit;

	if(!impl->end)
	{
		return 0;
	}

	char c = *impl->end;

	while(IsSeparator(c, impl->sep))
	{
		++impl->end;
		c = *impl->end;
	}

	if(c == '\0')
	{
		return 0;
	}

	impl->begin = impl->end;

	while(!IsSeparator(c, impl->sep) && c != '\0')
	{
		++impl->end;
		c = *impl->end;
	}

	return impl->begin != impl->end;
}

const char* StrSplit_String(struct StrSplit* strSplit)
{
	struct StrSplitImpl* impl = (struct StrSplitImpl*)strSplit;
	return impl->begin;
}

int StrSplit_Size(struct StrSplit* strSplit)
{
	struct StrSplitImpl* impl = (struct StrSplitImpl*)strSplit;
	return impl->end - impl->begin;;
}
