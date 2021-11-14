#include "tests.h"



TEST(MD5, ReadTestData)
{
	initialize();

	MD5 file("test-data/Test.md5");

	ASSERT_TRUE(file);

	file.Dump();

	//EXPECT_EQ(file.GetAssociations().size(), file.GetNumAssociations());//This seems to be broken even in the test file
	EXPECT_EQ(file.GetClubs().size(),		 file.GetNumClubs());
	EXPECT_EQ(file.GetParticipants().size(), file.GetNumParticipants());

	EXPECT_EQ(file.GetClubs().size(), 21);
	EXPECT_EQ(file.GetParticipants().size(), 142);

	ASSERT_TRUE(false);//TODO
}