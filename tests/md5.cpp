#include "tests.h"



TEST(MD5, ReadTestData)
{
	initialize();

	MD5 file("test-data/Test.md5");

	ASSERT_TRUE(file);

	file.Dump();

	EXPECT_EQ(file.GetAssociations().size(), file.GetNumAssociations());
	EXPECT_EQ(file.GetClubs().size(),		 file.GetNumClubs());
	EXPECT_EQ(file.GetParticipants().size(), file.GetNumParticipants());

	ASSERT_TRUE(false);//TODO
}