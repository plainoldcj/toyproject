#pragma once

enum PrimitiveType
{
	PT_CHAR,
	PT_FLOAT,
	PT_INT
};

struct ReflectedVariable
{
	const char*			name;
	const char*			typeName;
	int					size;
	int					offset;
	enum PrimitiveType	primType;
	int					isPrim;
	int					isArray;
	int					elementCount;
};

struct ReflectedType
{
	const char*					name;
	int							variableCount;
	struct ReflectedVariable*	variables;
};

const struct ReflectedType*	FindReflectedType(const char* typeName);

void						PrintReflectedType(char* buffer, const char* typeName);
