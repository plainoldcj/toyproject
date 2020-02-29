#pragma once

enum PrimitiveType
{
	PT_CHAR,
	PT_FLOAT,
	PT_INT,
	PT_UINT8,
	PT_UINT16
};

enum AttributeFlags
{
	AF_ELEMENT_COUNT_VAR = (1 << 0)
};

struct ReflectedAttribute
{
	int			flags;

	// Element count attribute.
	const char* elementCountVar;
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
	int					attrib;
};

struct ReflectedType
{
	const char*					name;
	int							variableCount;
	struct ReflectedVariable*	variables;
};

const struct ReflectedType*	FindReflectedType(const char* typeName);

const struct ReflectedAttribute* GetReflectedAttributes(void);

void						PrintReflectedType(char* buffer, const char* typeName);
