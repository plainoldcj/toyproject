#include "common/reflect.h"
#include "common/unit_tests.h"

#include "json.h"
#include "json_tests.h"

#include <string.h>

UNIT_TEST(TestReadJson)
{
	const char* json =
	"{\n"
	"	\"nested\":\n"
	"	{\n"
	"		\"fvalue\": 1.24\n"
	"	},\n"
	"	\"fvalue0\": 5.0,\n"
	"	\"fvalue1\": 0.25,\n"
	"	\"str\": \"hello\"\n"
	"}";

	const struct ReflectedType* type = FindReflectedType("JsonTest");

	struct JsonTest object;

	bool success = ReadJson(type, &object, json, strlen(json), "test json");
	EXPECT_TRUE(success);

	EXPECT_EQUAL_FLOAT(object.fvalue0, 5.0f);
	EXPECT_EQUAL_FLOAT(object.fvalue1, 0.25f);
	EXPECT_EQUAL_FLOAT(object.nested.fvalue, 1.24f);
	EXPECT_EQUAL_INT(0, strcmp(object.str, "hello"));
}
