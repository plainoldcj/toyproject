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
	int	flags;

	// Element count attribute.
	int	elementCountVar;
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

const struct ReflectedVariable* GetReflectedVariables(void);

const struct ReflectedAttribute* GetReflectedAttributes(void);
int GetReflectedAttributeCount(void);

void						PrintReflectedType(char* buffer, const char* typeName);

int GetElementCount(const struct ReflectedType* type, const struct ReflectedVariable* var, void* object);
