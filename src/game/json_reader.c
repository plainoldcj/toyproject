#include "alloc.h"
#include "common.h"
#include "json_reader.h"

#include "common/reflect.h"

#include <assert.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct JsonReader;

struct ReaderContext
{
	struct ReaderContext*		next;
	struct JsonReader*			reader;

	const struct ReflectedType*	type;
	void*						object;

	uint32_t					cur;

	const char*					name;
	uint32_t					len;
};

struct JsonReader
{
	const char*					json;
	uint32_t					len;
	uint32_t					cur;

	const char*					debugName;

	struct ReaderContext*		context;
	jmp_buf						except;
};

static void ParseJsonObject(struct JsonReader* reader);

static void PushContext(struct JsonReader* reader, struct ReaderContext* context, const char* name, int len)
{
	context->reader = reader;

	context->type = reader->context->type;
	context->object = reader->context->object;

	context->next = reader->context;
	reader->context = context;

	context->cur = reader->cur;

	context->name = name;
	context->len = len;
}

static void PopContext(struct ReaderContext* context)
{
	struct JsonReader* reader = context->reader;
	assert(context == reader->context);
	reader->context = context->next;
}

static bool IsWhitespace(char c)
{
	return c == ' '
		|| c == '\n'
		|| c == '\t';
}

static void FindCursor(struct JsonReader* reader, int* outLine, int* outColumn)
{
	int line = 0;
	int col = 0;

	for(uint32_t i = 0; i <= reader->cur; ++i)
	{
		if(reader->json[i] == '\n')
		{
			col = 0;
			++line;
		}
		else
		{
			++col;
		}
	}

	*outLine = line;
	*outColumn = col;
}

static void FormatContext(struct JsonReader* reader, char* buffer, size_t size)
{
	struct ReaderContext* head = NULL;

	struct ReaderContext* context = reader->context;
	while(context)
	{
		struct ReaderContext* next = context->next;
		context->next = head;
		head = context;
		context = next;
	}

	size_t cur = 0;
	while(head)
	{
		if(cur)
		{
			if(cur + 1 >= size)
			{
				return;
			}

			buffer[cur] = '/';
			++cur;
		}

		int len = head->len;
		if(len < 0)
		{
			len = strlen(head->name);
		}

		if(cur + len >= size)
		{
			return;
		}

		snprintf(buffer + cur, len + 1, "%s", head->name);
		cur += len;

		head = head->next;
	}
}

static void ReadingError(struct JsonReader* reader, const char* format, ...)
{
	struct SScope stack;
	BigStack_Begin(&stack);

	uint32_t detailBufferSize = 1024;
	char* detailBuffer = BigStack_Alloc(detailBufferSize);
	memset(detailBuffer, 0, detailBufferSize);

	va_list args;
	va_start(args, format);
	vsprintf(detailBuffer, format, args);
	va_end(args);

	uint32_t contextBufferSize = 1024;
	char* contextBuffer = BigStack_Alloc(contextBufferSize);
	memset(contextBuffer, 0, contextBufferSize);

	FormatContext(reader, contextBuffer, contextBufferSize);

	BigStack_End(&stack);

	int line, column;
	FindCursor(reader, &line, &column);

	COM_LogPrintf("Error reading Json at position (%d,%d) in context '%s': %s",
		line, column, contextBuffer, detailBuffer);

	longjmp(reader->except, 1);
}

static void ParseJsonString(struct JsonReader* reader)
{
	while(reader->cur < reader->len)
	{
		int c = reader->json[reader->cur];
		if(c == '\"')
		{
			++reader->cur;
			return;
		}
		else if(IsWhitespace(c))
		{
			ReadingError(reader, "Whitespace in string");
		}
		else
		{
			++reader->cur;
		}
	}

	ReadingError(reader, "Unexpected end-of-input while reading string");
}

static void EatWhitespace(struct JsonReader* reader)
{
	while(reader->cur < reader->len && IsWhitespace(reader->json[reader->cur]))
	{
		++reader->cur;
	}

}

static void EatToken(struct JsonReader* reader, char tok)
{
	EatWhitespace(reader);

	if(reader->cur >= reader->len)
	{
		ReadingError(reader, "Expected token '%c', but got end-of-input.", tok);
	}
	else if(reader->json[reader->cur] != tok)
	{
		ReadingError(reader, "Expected token '%c', but '%c'.",
			tok, reader->json[reader->cur]);
	}
	else
	{
		++reader->cur;
	}
}

static float ParseJsonFloat(struct JsonReader* reader)
{
	EatWhitespace(reader);

	if(reader->cur >= reader->len)
	{
		ReadingError(reader, "Expected float, but got end-of-input.");
	}

	int firstNum = reader->cur;
	bool hasDigits = false;

	if(reader->json[reader->cur] == '-')
	{
		++reader->cur;
	}

	while(reader->cur < reader->len && isdigit(reader->json[reader->cur]))
	{
		hasDigits = true;
		++reader->cur;
	}

	if(reader->cur < reader->len && reader->json[reader->cur] == '.')
	{
		++reader->cur;
	}

	while(reader->cur < reader->len && isdigit(reader->json[reader->cur]))
	{
		hasDigits = true;
		++reader->cur;
	}

	const char* num = reader->json + firstNum;
	int len = reader->cur - firstNum;

	printf("XXXX num '%.*s'", len, num);

	if(!hasDigits)
	{
		ReadingError(reader, "Expected float, but got '%.*s'", len, num);
	}

	return (float)atof(num);
}

static int ParseJsonInt(struct JsonReader* reader)
{
	EatWhitespace(reader);

	if(reader->cur >= reader->len)
	{
		ReadingError(reader, "Expected int, but got end-of-input.");
	}

	int firstNum = reader->cur;
	bool hasDigits = false;

	while(reader->cur < reader->len && isdigit(reader->json[reader->cur]))
	{
		hasDigits = true;
		++reader->cur;
	}

	const char* num = reader->json + firstNum;
	int len = reader->cur - firstNum;

	if(!hasDigits)
	{
		ReadingError(reader, "Expected int, but got '%.*s'", len, num);
	}

	return (int)atoi(num);
}

#define FORALL_INTTYPES \
	FOR_INTTYPE(PT_INT,		int) \
	FOR_INTTYPE(PT_UINT8,	uint8_t) \
	FOR_INTTYPE(PT_UINT16,	uint16_t)

static void ParseJsonValue(struct JsonReader* reader, struct ReflectedVariable* var)
{
	if(var->isPrim && var->primType == PT_FLOAT)
	{
		float fValue = ParseJsonFloat(reader);

		float* ptr = (float*)((char*)reader->context->object + var->offset);
		*ptr = fValue;
	}
	else if(var->isPrim && !var->isArray)
	{
		int iValue = ParseJsonInt(reader);

#define FOR_INTTYPE(PRIMTYPE, TYPE) \
	if(var->primType == PRIMTYPE) \
	{ \
		TYPE* ptr = (TYPE*)((char*)reader->context->object + var->offset); \
		*ptr = (TYPE)iValue; \
	}

	FORALL_INTTYPES
	
#undef FOR_INTTYPE

	}
	else if(var->isPrim && var->primType == PT_CHAR && var->isArray)
	{
		EatToken(reader, '\"');

		uint32_t first = reader->cur;
		ParseJsonString(reader);
		uint32_t len = reader->cur - first - 1;

		if(len >= var->elementCount)
		{
			ReadingError(reader, "String is too large");
		}

		void* data = (char*)reader->context->object + var->offset;
		memset(data, 0, var->size);

		strncpy(data, reader->json + first, len);
	}
	else if(!var->isPrim && !var->isArray)
	{
		const struct ReflectedType* varType = FindReflectedType(var->typeName);
		if(!varType)
		{
			ReadingError(reader, "Cannot find reflection for variable type '%s'",
				var->typeName);
		}

		EatToken(reader, '{');

		struct ReaderContext context;
		PushContext(reader, &context, var->typeName, -1);
		context.object = (char*)reader->context->object + var->offset;
		context.type = varType;

		ParseJsonObject(reader);

		PopContext(&context);
	}
	else if(!var->isPrim && var->isArray)
	{
		const struct ReflectedType* varType = FindReflectedType(var->typeName);
		if(!varType)
		{
			ReadingError(reader, "Cannot find reflection for variable type '%s'",
				var->typeName);
		}

		EatToken(reader, '[');

		bool done = false;

		int count = 0;

		while(reader->cur < reader->len && !done)
		{
			EatWhitespace(reader);

			if(reader->cur >= reader->len)
			{
				ReadingError(reader, "Unexpected end-of-input while reading array");
			}

			char c = reader->json[reader->cur];

			if(c == '{')
			{
				++reader->cur;

				size_t elementSize = var->size / var->elementCount;

				struct ReaderContext context;
				PushContext(reader, &context, var->typeName, -1);
				context.object = (char*)reader->context->object + var->offset + count * elementSize;
				context.type = varType;

				ParseJsonObject(reader);

				PopContext(&context);


				++count;
			}
			else if(c == ',')
			{
				++reader->cur;
			}
			else if(c == ']')
			{
				++reader->cur;
				done = true;
			}
		}

		// Write element count.
		const struct ReflectedAttribute* attrib = GetReflectedAttributes() + var->attrib;
		if(attrib->flags & AF_ELEMENT_COUNT_VAR)
		{
			const struct ReflectedVariable* countVar = GetReflectedVariables() + attrib->elementCountVar;
#define FOR_INTTYPE(PRIMTYPE, TYPE) \
	if(countVar->primType == PRIMTYPE) \
	{ \
		TYPE* ptr = (TYPE*)((char*)reader->context->object + countVar->offset); \
		*ptr = (TYPE)count; \
	}

	FORALL_INTTYPES
	
#undef FOR_INTTYPE
		}
	}
	else
	{
		ReadingError(reader, "Unsupported variable type '%s' for variable '%s'",
			var->typeName, var->name);
	}
}

static void ParseJsonKeyValue(struct JsonReader* reader)
{
	uint32_t firstKey = reader->cur;
	ParseJsonString(reader);
	uint32_t keyLen = reader->cur - firstKey - 1;

	const char* key = reader->json + firstKey;

	struct ReaderContext context;
	PushContext(reader, &context, key, keyLen);

	EatToken(reader, ':');

	const struct ReflectedType* type = reader->context->type;
	for(int varIdx = 0; varIdx < type->variableCount; ++varIdx)
	{
		struct ReflectedVariable* var = type->variables + varIdx;
		if(!strncmp(var->name, key, keyLen) && var->name[keyLen] == '\0')
		{
			ParseJsonValue(reader, var);
		}
	}

	PopContext(&context);
}

static void ParseJsonObjectKeys(struct JsonReader* reader)
{
	ParseJsonKeyValue(reader);

	while(reader->cur < reader->len)
	{
		EatWhitespace(reader);

		if(reader->cur < reader->len && reader->json[reader->cur] == ',')
		{
			++reader->cur;

			EatWhitespace(reader);

			if(reader->cur < reader->len && reader->json[reader->cur] == '\"')
			{
				++reader->cur;

				ParseJsonKeyValue(reader);
			}
			else
			{
				ReadingError(reader, "Expected token '\"', but got '%c'",
					reader->json[reader->cur]);
			}
		}
		else
		{
			return;
		}
	}
}

static void ParseJsonObject(struct JsonReader* reader)
{
	while(reader->cur < reader->len)
	{
		char c = reader->json[reader->cur];
		if(c == '}')
		{
			++reader->cur;
			return;
		}
		else if(c == '\"')
		{
			++reader->cur;

			ParseJsonObjectKeys(reader);

			EatWhitespace(reader);

			if(reader->cur < reader->len && reader->json[reader->cur] == '}')
			{
				++reader->cur;
				return;
			}
			else
			{
				ReadingError(reader, "Expected token '}', but got '%c'",
					reader->json[reader->cur]);
			}
		}
		else if(IsWhitespace(c))
		{
			++reader->cur;
		}
		else
		{
			ReadingError(reader, "Unexpected character '%c' while reading object.", c);
		}
	}
}

bool ReadJson(
	const struct ReflectedType* type,
	void* object,
	const char* json,
	uint32_t len,
	const char* debugName)
{
	struct JsonReader reader;

	reader.json = json;
	reader.len = len;
	reader.cur = 0;

	reader.debugName = debugName;

	struct ReaderContext rootContext = { 0 };
	rootContext.reader = &reader;
	rootContext.name = "<root>";
	rootContext.len = ~0u;

	reader.context = &rootContext;

	if(setjmp(reader.except))
	{
		COM_LogPrintf("Unable to read json '%s'.", debugName);
		return false;
	}

	while(reader.cur < reader.len)
	{
		char c = reader.json[reader.cur];
		if(c == '{')
		{
			++reader.cur;

			struct ReaderContext context;
			PushContext(&reader, &context, type->name, -1);
			context.type = type;
			context.object = object;

			ParseJsonObject(&reader);

			PopContext(&context);

			// TODO(cj): Expect end of input.
		}
		else if(IsWhitespace(c))
		{
			++reader.cur;
		}
		else
		{
			ReadingError(&reader, "Unexpected character '%c'", c);
		}
	}

	return true;
}
