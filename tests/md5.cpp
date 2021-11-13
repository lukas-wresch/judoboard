#include "tests.h"



TEST(MD5, ReadTestData)
{
	initialize();

	MD5 file("test-data/Test.md5");

	ASSERT_TRUE(file);

	ASSERT_TRUE(false);//TODO
}