#include "tests.h"



TEST(MD5, ReadTestData)
{
	initialize();

	MD5 file("test-data/Test.md5");

	ASSERT_TRUE(file);

	file.Dump();

	ASSERT_TRUE(false);//TODO
}