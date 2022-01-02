#include "tests.h"



TEST(ID, SHA256)
{
	initialize();

	ZED::SHA256 empty("");
	ZED::SHA256 test("test");
	ZED::SHA256 test2("thisisaverylongteststringtotesthashfunctions");

	EXPECT_TRUE(empty == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	EXPECT_TRUE(test  == "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08");
	EXPECT_TRUE(test2 == "5bfd8507c0fceb6bef290c25591f760285c5271b8fff6bb333c7d8dde1bd1737");
}



TEST(ID, SHA512)
{
	initialize();

	ZED::SHA512 empty("");
	ZED::SHA512 test("test");
	ZED::SHA512 test2("thisisaverylongteststringtotesthashfunctions");

	EXPECT_TRUE(empty == "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
	EXPECT_TRUE(test  == "ee26b0dd4af7e749aa1a8ee3c10ae9923f618980772e473f8819a5d4940e0db27ac185f8a0e1d5f84f88bc887fd67b143732c304cc5fa9ad8e6f57f50028a8ff");
	EXPECT_TRUE(test2 == "640c2a1b1a1943b61d0cfead761bd733f7a5a8fe15f8a5d86cebedad739c5ef724a3cc11e77338dd174b49aa0e45c2ca9b30e5a5e21729ba3345ede347a245d6");
}



TEST(ID, CheckUniqueness)
{
	initialize();

	const int size = 5000;
	ID* ids[size] = {};

	for (int i = 0; i < size; i++)
	{
		ids[i] = new ID;
	}

	for (int i = 0; i < size; i++)
		for (int j = i+1; j < size; j++)
	{
		ASSERT_NE(ids[i]->GetID(),   ids[j]->GetID());
		ASSERT_NE(ids[i]->GetUUID(), ids[j]->GetUUID());
	}

	for (int i = 0; i < size; i++)
		delete ids[i];
}