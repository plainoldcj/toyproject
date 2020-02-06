#include "common.h"
#include "font.h"
#include "strsplit.h"

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#define FONT_READER_STRING_SIZE	256

struct FontReader
{
	struct Font*	font;

	struct StrSplit	strSplit;
	jmp_buf			except;

	// Always 0-terminated.
	char			token[FONT_READER_STRING_SIZE];

	const char*		valueTok;
	int				valueLen;

	char			stringValue[FONT_READER_STRING_SIZE];
	int				intValue;
};

static void NextToken(struct FontReader* reader)
{
	if(!StrSplit_Next(&reader->strSplit))
	{
		COM_LogPrintf("Parsing font '%s' failed: Unexpected end of input.",
				reader->font->debugName);
		longjmp(reader->except, 1);
	}

	const char* tok = StrSplit_String(&reader->strSplit);
	int tokLen = StrSplit_Size(&reader->strSplit);

	if(tokLen >= FONT_READER_STRING_SIZE)
	{
		COM_LogPrintf("Parsing font '%s' failed: Token exceeds size %d",
				reader->font->debugName,
				FONT_READER_STRING_SIZE);
		longjmp(reader->except, 1);
	}

	memset(reader->token, 0, FONT_READER_STRING_SIZE);
	strncpy(reader->token, tok, tokLen);
	reader->token[FONT_READER_STRING_SIZE - 1] = '\0';
}

static void ExpectToken(struct FontReader* reader, const char* token)
{
	NextToken(reader);

	if(strcmp(token, reader->token))
	{
		COM_LogPrintf("Parsing font '%s' failed: Expected token '%s', but got '%s'.",
				reader->font->debugName,
				token,
				reader->token);

		longjmp(reader->except, 1);
	}
}

static void ExpectKeyValue(struct FontReader* reader, const char* key)
{
	NextToken(reader);

	struct StrSplit kvs; // key=value splitter.
	StrSplit_Init(&kvs, reader->token, "=");

	int success = StrSplit_Next(&kvs);
	if(!success)
	{
		COM_LogPrintf("Parsing font '%s' failed: Unexpected end of input.",
				reader->font->debugName);
		longjmp(reader->except, 1);
	}

	const char* tok = StrSplit_String(&kvs);
	int tokLen = StrSplit_Size(&kvs);

	if(strncmp(key, tok, tokLen))
	{
		char buf[FONT_READER_STRING_SIZE] = { '\0' };
		strncpy(buf, tok, tokLen);
		buf[FONT_READER_STRING_SIZE - 1] = '\0';

		COM_LogPrintf("Parsing font '%s' failed: Expected key '%s' but got '%s'.",
			reader->font->debugName, key, buf);
		longjmp(reader->except, 1);
	}

	success = StrSplit_Next(&kvs);
	if(!success)
	{
		COM_LogPrintf("Parsing font '%s' failed: Unexpected end of input.",
				reader->font->debugName);
		longjmp(reader->except, 1);
	}

	reader->valueTok = StrSplit_String(&kvs);
	reader->valueLen = StrSplit_Size(&kvs);
}

static void ExpectString(struct FontReader* reader, const char* key)
{
	ExpectKeyValue(reader, key);

	if(reader->valueTok[0] != '"' || reader->valueTok[reader->valueLen - 1] != '"')
	{
		COM_LogPrintf("Parsing font '%s' failed: Expected string, but got '%s'.",
			reader->font->debugName, reader->valueTok);
		longjmp(reader->except, 1);
	}

	memset(reader->stringValue, 0, sizeof(reader->stringValue));
	strncpy(reader->stringValue, reader->valueTok + 1, reader->valueLen - 2);
}

static int ExpectInt(struct FontReader* reader, const char* key)
{
	ExpectKeyValue(reader, key);
	return atoi(reader->valueTok);
}

static float ExpectFloat(struct FontReader* reader, const char* key)
{
	ExpectKeyValue(reader, key);
	return atof(reader->valueTok);
}

bool InitFont(
	struct Font*	font,
	const char*		desc,
	int				descLen,
	const char*		debugName)
{
	memset(font->chars, 0, sizeof(struct FontChar) * FONT_CHAR_COUNT);

	struct FontReader reader;

	reader.font = font;
	strncpy(font->debugName, debugName, FONT_DEBUG_NAME_SIZE);

	if(setjmp(reader.except))
	{
		COM_LogPrintf("Unable to create font '%s'", debugName);
		return false;
	}

	StrSplit_Init(&reader.strSplit, desc, 0);

	ExpectToken(&reader, "info");

	ExpectString(&reader, "face");
	strcpy(font->faceName, reader.stringValue);

	ExpectToken(&reader, "common");

	font->lineHeight = ExpectFloat(&reader, "lineHeight");

	font->scaleW = (uint16_t)ExpectInt(&reader, "scaleW");
	font->scaleH = (uint16_t)ExpectInt(&reader, "scaleH");

	ExpectToken(&reader, "chars");

	font->count = (uint16_t)ExpectInt(&reader, "count");

	for(int i = 0; i < font->count; ++i)
	{
		struct FontChar fontChar;

		ExpectToken(&reader, "char");

		fontChar.id			= (uint16_t)ExpectInt(&reader, "id");
		fontChar.x			= (uint16_t)ExpectInt(&reader, "x");
		fontChar.y			= (uint16_t)ExpectInt(&reader, "y");
		fontChar.width		= (uint8_t)ExpectInt(&reader, "width");
		fontChar.height		= (uint8_t)ExpectInt(&reader, "height");
		fontChar.xoffset	= ExpectFloat(&reader, "xoffset");
		fontChar.yoffset	= ExpectFloat(&reader, "yoffset");
		fontChar.xadvance	= ExpectFloat(&reader, "xadvance");
		fontChar.page		= (uint8_t)ExpectInt(&reader, "page");
		fontChar.chnl		= (uint8_t)ExpectInt(&reader, "chnl");

		if(fontChar.id >= FONT_CHAR_COUNT)
		{
			COM_LogPrintf("Parsing font '%s' failed: Character id %d is out of range.",
					reader.font->debugName,
					fontChar.id);

			longjmp(reader.except, 1);
		}

		font->chars[fontChar.id] = fontChar;
	}

	return true;
}
