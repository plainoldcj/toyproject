#include "json_common.h"

#include "common/reflect.h"

#include <stdio.h>

bool ShouldReadWrite(const struct ReflectedVariable* var)
{
	const int varIndex = var - GetReflectedVariables();

	const struct ReflectedAttribute* attrib = GetReflectedAttributes();
	const struct ReflectedAttribute* const attribEnd = attrib + GetReflectedAttributeCount();

	while(attrib != attribEnd)
	{
		if(attrib->flags & AF_ELEMENT_COUNT_VAR)
		{
			if(varIndex == attrib->elementCountVar)
			{
				return false;
			}
		}
		++attrib;
	}

	return true;
}
