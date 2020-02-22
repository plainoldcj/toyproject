#include "common/reflect.h"
#include "common/unit_tests.h"

#include "json_reader.h"
#include "json_tests.h"
#include "json_writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WRITE_BUFFER_SIZE 2048

UNIT_TEST(TestReadJson)
{
	const char* json =
	"{\n"
	"	\"nested\":\n"
	"	{\n"
	"		\"fValue\": 1.24\n"
	"	},\n"
	"	\"fValue\": 5.0,\n"
	"	\"iValue\": 11,\n"
	"	\"u8Value\": 55,\n"
	"	\"u16Value\": 0,\n"
	"	\"str\": \"hello\",\n"
	"	\"arr\": [\n"
	"		{\n"
	"			\"iValue\": 17\n"
	"		},\n"
	"		{\n"
	"			\"iValue\": 18\n"
	"		}\n"
	"	]\n"
	"}";

	const struct ReflectedType* type = FindReflectedType("JsonTest");

	struct JsonTest object;

	bool success = ReadJson(type, &object, json, strlen(json), "json read test");
	EXPECT_TRUE(success);

	EXPECT_EQUAL_FLOAT(object.fValue, 5.0f);
	EXPECT_EQUAL_FLOAT(object.iValue, 11);
	EXPECT_EQUAL_FLOAT(object.u8Value, 55);
	EXPECT_EQUAL_FLOAT(object.u16Value, 0);
	EXPECT_EQUAL_FLOAT(object.nested.fValue, 1.24f);
	EXPECT_EQUAL_INT(0, strcmp(object.str, "hello"));
	EXPECT_EQUAL_INT(17, object.arr[0].iValue);
	EXPECT_EQUAL_INT(18, object.arr[1].iValue);
}

UNIT_TEST(TestWriteJson)
{
	const char* expected =
		"{\n"
		"\t\"fValue\": 54.20,\n"
		"\t\"iValue\": 11,\n"
		"\t\"u8Value\": 0,\n"
		"\t\"u16Value\": 132,\n"
		"\t\"str\": \"hello, json\",\n"
		"\t\"nested\": {\n"
		"\t\t\"fValue\": 32.23\n"
		"\t},\n"
		"\t\"arr\": [\n"
		"\t\t{\n"
		"\t\t\t\"iValue\": -17\n"
		"\t\t},\n"
		"\t\t{\n"
		"\t\t\t\"iValue\": -18\n"
		"\t\t}\n"
		"\t]\n"
		"}";

	char* buffer = malloc(WRITE_BUFFER_SIZE);
	memset(buffer, 0, WRITE_BUFFER_SIZE);

	const struct ReflectedType* type = FindReflectedType("JsonTest");
	struct JsonTest object =
	{
		.fValue = 54.2f,
		.iValue = 11,
		.u8Value = 0,
		.u16Value = 132,
		.str = "hello, json",
		.nested = {
			.fValue = 32.23f
		},
		.arr[0] =
		{
			.iValue = -17
		},
		.arr[1] =
		{
			.iValue = -18
		}
	};

	bool success = WriteJson(type, &object, buffer, WRITE_BUFFER_SIZE, "json write test");

#if 0
	printf("json writer output:\n%s", buffer);
	printf("json writer expected:\n%s", expected);
#endif

	EXPECT_TRUE(success);
	EXPECT_EQUAL_INT(0, strcmp(expected, buffer));

	free(buffer);
}
