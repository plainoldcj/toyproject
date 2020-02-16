#include "alloc.h"
#include "common.h"
#include "json.h"

#include "common/reflect.h"

#include <assert.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct JsonReader;

struct ReaderContext
{
	struct ReaderContext*	next;
	struct JsonReader*		reader;

	uint32_t				cur;

	const char*				name;
	uint32_t				len;
};

struct JsonReader
{
	const struct ReflectedType*	type;
	void*						object;

	const char*					json;
	uint32_t					len;
	uint32_t					cur;

	const char*					debugName;

	struct ReaderContext*		context;
	jmp_buf						except;
};

static void PushContext(struct JsonReader* reader, struct ReaderContext* context, const char* name, int len)
{
	context->reader = reader;

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

	if(!hasDigits)
	{
		ReadingError(reader, "Expected float, but got '%.*s'", len, num);
	}

	return (float)atof(num);
}

static void ParseJsonValue(struct JsonReader* reader, struct ReflectedVariable* var)
{
	if(var->isPrim && var->primType == PT_FLOAT)
	{
		float fValue = ParseJsonFloat(reader);

		float* ptr = (float*)((char*)reader->object + var->offset);
		*ptr = fValue;
	}
	else
	{
		ReadingError(reader, "Unsupported variable type.");
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

	const struct ReflectedType* type = reader->type;
	for(int varIdx = 0; varIdx < type->variableCount; ++varIdx)
	{
		struct ReflectedVariable* var = type->variables + varIdx;
		if(!strncmp(var->name, key, keyLen))
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

	reader.type = type;
	reader.object = object;

	reader.json = json;
	reader.len = len;
	reader.cur = 0;

	reader.debugName = debugName;

	reader.context = NULL;

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
			struct ReaderContext context;
			PushContext(&reader, &context, "<object>", -1);

			++reader.cur;
			ParseJsonObject(&reader);

			PopContext(&context);
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