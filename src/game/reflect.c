#include "universal/reflect.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Defined by automatically-generated file reflected.c
extern struct ReflectedType*		g_types;
extern struct ReflectedAttribute*	g_attributes;
extern struct ReflectedVariable*	g_variables;

extern int							g_attributeCount;

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

const struct ReflectedVariable* GetReflectedVariables(void)
{
	return g_variables;
}

const struct ReflectedAttribute* GetReflectedAttributes(void)
{
	return g_attributes;
}

int GetReflectedAttributeCount(void)
{
	return g_attributeCount;
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

// TODO(cj): Similar code in json_reader.c
#define FORALL_INTTYPES \
	FOR_INTTYPE(PT_INT,		int) \
	FOR_INTTYPE(PT_UINT8,	uint8_t) \
	FOR_INTTYPE(PT_UINT16,	uint16_t)

static bool TryGetInteger(const struct ReflectedVariable* var, void* object, int* out)
{

#define FOR_INTTYPE(PRIMTYPE, TYPE) \
	if(var->primType == PRIMTYPE) \
	{ \
		TYPE* ptr = (TYPE*)((char*)object + var->offset); \
		*out = (int)*ptr;\
		return true;\
	}

	FORALL_INTTYPES
	
#undef FOR_INTTYPE

	*out = 0;
	return false;
}

int GetElementCount(const struct ReflectedType* type, const struct ReflectedVariable* var, void* object)
{
	assert(var->isArray);

	if(var->attrib != -1)
	{
		const struct ReflectedAttribute* attrib = GetReflectedAttributes() + var->attrib;
		if(attrib->flags & AF_ELEMENT_COUNT_VAR)
		{
			struct ReflectedVariable* countVar = g_variables + attrib->elementCountVar;

			int elementCount;
			bool success = TryGetInteger(countVar, object, &elementCount);
			assert(success);

			return elementCount;
		}
	}

	return var->elementCount;
}
