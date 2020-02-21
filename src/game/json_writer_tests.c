#include "common/reflect.h"
#include "common/unit_tests.h"

#include "json_writer.h"
#include "json_writer_tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 2048

static const char* expected =
"{\n"
"\t\"fValue\": 54.20,\n"
"\t\"iValue\": 11,\n"
"\t\"str\": \"hello, json\",\n"
"\t\"nested\": {\n"
"\t\t\"fValue\": 32.23\n"
"\t}\n"
"}";

UNIT_TEST(TestWriteJson)
{
	char* buffer = malloc(BUFFER_SIZE);
	memset(buffer, 0, BUFFER_SIZE);

	const struct ReflectedType* type = FindReflectedType("JsonWriterTest");
	struct JsonWriterTest object =
	{
		.fValue = 54.2f,
		.iValue = 11,
		.str = "hello, json",
		.nested = {
			.fValue = 32.23f
		}
	};

	bool success = WriteJson(type, &object, buffer, BUFFER_SIZE, "json test");

#if 0
	printf("json writer output:\n%s", buffer);
	printf("json writer expected:\n%s", expected);
#endif

	EXPECT_TRUE(success);
	EXPECT_EQUAL_INT(0, strcmp(expected, buffer));

	free(buffer);
}
