#include "font.h"

#include "common/unit_tests.h"

#include <string.h>

const char* fontDesc =
"info face=\"Consolas\"\n"
"common lineHeight=68.475   scaleW=512   scaleH=512   \n"
"chars count=1\n"
"char id=0     x=505   y=458   width=4     height=4     xoffset=-1.500    yoffset=43.875    xadvance=31.313      page=0  chnl=0\n";

UNIT_TEST(TestCreateFont)
{
	struct Font font;
	bool success = CreateFont(&font, fontDesc, strlen(fontDesc), "test_font");
	EXPECT_TRUE(success);

	EXPECT_EQUAL_INT(strcmp("test_font", font.debugName), 0);
	EXPECT_EQUAL_INT(strcmp("Consolas", font.faceName), 0);

	// TODO(cj): Test float values, too.

	EXPECT_EQUAL_INT(font.scaleW, 512);
	EXPECT_EQUAL_INT(font.scaleH, 512);
	EXPECT_EQUAL_INT(font.count, 1);

	EXPECT_EQUAL_INT(font.chars[0].id, 0);
	EXPECT_EQUAL_INT(font.chars[0].x, 505);
	EXPECT_EQUAL_INT(font.chars[0].y, 458);
	EXPECT_EQUAL_INT(font.chars[0].width, 4);
	EXPECT_EQUAL_INT(font.chars[0].height, 4);
	EXPECT_EQUAL_INT(font.chars[0].page, 0);
	EXPECT_EQUAL_INT(font.chars[0].chnl, 0);

	DestroyFont(&font);
}
