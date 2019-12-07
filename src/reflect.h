#pragma once

enum PrimitiveType
{
	PT_FLOAT
};

struct ReflectedVariable
{
	const char*			name;
	const char*			typeName;
	int					size;
	int					offset;
	enum PrimitiveType	primType;
	int					isPrim;
};

struct ReflectedType
{
	const char*					name;
	int							variableCount;
	struct ReflectedVariable*	variables;
};

struct ReflectedType* FindReflectedType(const char* name);
