#include "json_writer.h"

#include "universal/reflect.h"

#include "common.h"
#include "json_common.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>

struct JsonWriter
{
	char*		buffer;
	uint32_t	bufferSize;

	uint32_t	cur;

	const char*	debugName;

	jmp_buf		except;

	int			indent;
};

static void WriteJsonObject(
	struct JsonWriter* writer,
	const struct ReflectedType* type,
	void* object);

static void WritingError(struct JsonWriter* writer, const char* error)
{
	COM_LogPrintf("Error writing json '%s': %s",
			writer->debugName, error);
	longjmp(writer->except, 1);
}

static void Write(struct JsonWriter* writer, const char* format, ...)
{
	char* dst;
	int size, count;

	size = writer->bufferSize - writer->cur;
	dst = writer->buffer + writer->cur;

	va_list args;
	va_start(args, format);
	count = vsnprintf(dst, size, format, args);
	va_end(args);

	if(count >= size)
	{
		WritingError(writer, "Buffer too small");
	}

	writer->cur += count;
}

static void WriteIndent(struct JsonWriter* writer)
{
	Write(writer, "%.*s", writer->indent, "\t\t\t\t\t\t\t\t\t\t\t");
}

#define FORALL_PRIMTYPES\
	FOR_PRIMTYPE(PT_FLOAT,	float, "%.2f") \
	FOR_PRIMTYPE(PT_INT,	int, "%d") \
	FOR_PRIMTYPE(PT_UINT8,	uint8_t, "%" PRIu8) \
	FOR_PRIMTYPE(PT_UINT16, uint16_t, "%" PRIu16)

static void WriteVariable(
	struct JsonWriter* writer,
	const struct ReflectedType* type,
	const struct ReflectedVariable* var,
	void* object)
{
	if(var->isPrim && !var->isArray)
	{
#define FOR_PRIMTYPE(PRIMTYPE, TYPE, FORMAT) \
	if(var->primType == PRIMTYPE) \
	{ \
		TYPE* value = (TYPE*)((char*)object + var->offset); \
		Write(writer, FORMAT, *value); \
	}

FORALL_PRIMTYPES

#undef FOR_PRIMTYPE
	}
	else if(var->isPrim && var->primType == PT_CHAR && var->isArray)
	{
		const char* str = (const char*)object + var->offset;
		Write(writer, "\"%.*s\"", var->elementCount, str);
	}
	else if(!var->isPrim && !var->isArray)
	{
		const struct ReflectedType* nestedType = FindReflectedType(var->typeName);
		if(!nestedType)
		{
			WritingError(writer, "Cannot find reflected type");
		}
		void* nestedObject = (char*)object + var->offset;

		WriteJsonObject(writer, nestedType, nestedObject);
	}
	else if(!var->isPrim && var->isArray)
	{
		const struct ReflectedType* nestedType = FindReflectedType(var->typeName);
		if(!nestedType)
		{
			WritingError(writer, "Cannot find reflected type");
		}

		Write(writer, "[\n");
		++writer->indent;

		const int elementCount = GetElementCount(type, var, object);

		for(int i = 0; i < elementCount; ++i)
		{
			if(i != 0)
			{
				Write(writer, ",\n");
			}

			size_t nestedSize = var->size / var->elementCount;

			void* nestedObject = (char*)object + var->offset + nestedSize * i;

			WriteIndent(writer);
			WriteJsonObject(writer, nestedType, nestedObject);
		}

		--writer->indent;

		Write(writer, "\n");
		WriteIndent(writer);
		Write(writer, "]");
	}
	else
	{
		WritingError(writer, "Unsupported variable type");
	}
}

static void WriteJsonObject(
	struct JsonWriter* writer,
	const struct ReflectedType* type,
	void* object)
{
	Write(writer, "{");
	++writer->indent;

	for(int varIdx = 0; varIdx < type->variableCount; ++varIdx)
	{
		const struct ReflectedVariable* var = type->variables + varIdx;

		if(!ShouldReadWrite(var))
		{
			continue;
		}

		if(varIdx != 0)
		{
			Write(writer, ",");
		}

		Write(writer, "\n");
		WriteIndent(writer);
		Write(writer, "\"%s\": ", var->name);
		WriteVariable(writer, type, var, object);
	}

	--writer->indent;
	Write(writer, "\n");
	WriteIndent(writer);
	Write(writer, "}");
}

bool WriteJson(
	const struct ReflectedType* type,
	void* object,
	char* buffer,
	uint32_t bufferSize,
	const char* debugName)
{
	struct JsonWriter writer;

	writer.buffer = buffer;
	writer.bufferSize = bufferSize;
	writer.cur = 0;
	writer.debugName = debugName;
	writer.indent = 0;

	if(setjmp(writer.except))
	{
		COM_LogPrintf("Unable to write json '%s'.", debugName);
		return false;
	}

	WriteJsonObject(&writer, type, object);

	return true;
}
