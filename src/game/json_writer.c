#include "json_writer.h"

#include "common/reflect.h"

#include "common.h"

#include <stdarg.h>
#include <stdbool.h>
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

static void WriteVariable(
	struct JsonWriter* writer,
	const struct ReflectedVariable* var,
	void* object)
{
	if(var->isPrim && var->primType == PT_FLOAT)
	{
		float* fValue = (float*)((char*)object + var->offset);
		Write(writer, "%.2f", *fValue);
	}
	else if(var->isPrim && var->primType == PT_CHAR && var->isArray)
	{
		const char* str = (const char*)object + var->offset;
		Write(writer, "\"%.*s\"", var->elementCount, str);
	}
	else if(!var->isPrim)
	{
		const struct ReflectedType* nestedType = FindReflectedType(var->typeName);
		if(!nestedType)
		{
			WritingError(writer, "Cannot find reflected type");
		}
		void* nestedObject = (char*)object + var->offset;

		WriteJsonObject(writer, nestedType, nestedObject);
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
		if(varIdx != 0)
		{
			Write(writer, ",");
		}

		const struct ReflectedVariable* var = type->variables + varIdx;
		Write(writer, "\n");
		WriteIndent(writer);
		Write(writer, "\"%s\": ", var->name);
		WriteVariable(writer, var, object);
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
