#include "common/reflect.h"
#include "common/unit_tests.h"

#include "json.h"
#include "json_tests.h"

#include <string.h>

UNIT_TEST(TestReadJson)
{
	const char* json =
	"{\n"
	"	\"fvalue0\": 5.0,\n"
	"	\"fvalue1\": 0.25\n"
	"}";

	const struct ReflectedType* type = FindReflectedType("JsonTest");

	struct JsonTest object;

	bool success = ReadJson(type, &object, json, strlen(json), "test json");
	EXPECT_TRUE(success);

	EXPECT_EQUAL_FLOAT(object.fvalue0, 5.0f);
	EXPECT_EQUAL_FLOAT(object.fvalue1, 0.25f);
}
