#include "common/reflect.h"

#include <stdio.h>
#include <string.h>

// Defined by automatically-generated file reflected.c
extern struct ReflectedType* g_types;
extern struct ReflectedAttribute* g_attributes;

const struct ReflectedType* FindReflectedType(const char* typeName)
{
	struct ReflectedType* type = g_types;
	while(type->name)
	{
		if(!strcmp(typeName, type->name))
		{
			return type;
		}
		++type;
	}
	return NULL;
}

const struct ReflectedAttribute* GetReflectedAttributes(void)
{
	return g_attributes;
}

void PrintReflectedType(char* buffer, const char* typeName)
{
	buffer += sprintf(buffer, "Reflected type: %s\n", typeName);

	const struct ReflectedType* type = FindReflectedType(typeName);
	if(!type)
	{
		buffer += sprintf(buffer, "NOT FOUND\n");
		return;
	}

	buffer += sprintf(buffer, "Number of variables: %d\n", type->variableCount);

	for(int i = 0; i < type->variableCount; ++i)
	{
		const struct ReflectedVariable* var = &type->variables[i];
		buffer += sprintf(buffer, "%d: name: %s, type: %s, size: %d, offset: %d, array: %d, elementCount: %d\n",
			i,
			var->name,
			var->typeName,
			var->size,
			var->offset,
			var->isArray,
			var->elementCount);
	}
}
