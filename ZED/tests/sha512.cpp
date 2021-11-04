#include "CppUnitTest.h"
#include "../include/sha512.h"
#include "../include/file.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace UnitTests
{
	TEST_CLASS(SHA512)
	{
	public:

		TEST_METHOD(String)
		{
			auto hash = ZED::SHA512("test");

			Assert::IsTrue(hash == "ee26b0dd4af7e749aa1a8ee3c10ae9923f618980772e473f8819a5d4940e0db27ac185f8a0e1d5f84f88bc887fd67b143732c304cc5fa9ad8e6f57f50028a8ff");
			Assert::IsTrue(hash == "EE26b0dd4af7e749aa1a8ee3c10ae9923f618980772e473f8819a5d4940e0db27ac185f8a0e1d5f84f88bc887fd67b143732c304cc5fa9ad8e6f57f50028a8ff");
			Assert::IsTrue(hash == "ee26b0dd4af7e749aa1a8ee3c10ae9923f618980772e473f8819a5d4940e0db27ac185f8a0e1d5f84f88bc887fd67b143732c304cc5fa9ad8e6f57f50028a8FF");


			auto hash2 = ZED::SHA512("");

			Assert::IsTrue(hash2 == "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
		}



		TEST_METHOD(Conversion)
		{
			auto hash = ZED::SHA512("test");
			std::string hex = hash;
			Assert::IsTrue(hex == "ee26b0dd4af7e749aa1a8ee3c10ae9923f618980772e473f8819a5d4940e0db27ac185f8a0e1d5f84f88bc887fd67b143732c304cc5fa9ad8e6f57f50028a8ff");
		}


		TEST_METHOD(ChecksumOfArrialTTF)
		{
			auto file = ZED::File("..\\..\\bin\\arial.ttf");
			Assert::IsTrue(file);

			ZED::Blob data;
			file >> data;

			Assert::IsTrue(data.GetSize() == 275572);

			auto hash = ZED::SHA512(data);

			Assert::IsTrue(hash == "DD2102535EC432F893250AD5DCACB1CE4CA08E498666ED6A1AEBF118B81467E1B3CE9516FBEE929911F3043C156EC0DBF67B9182AAD6AB31B7C38A75CCFE2E01");
		}


		TEST_METHOD(ChecksumOfArrialTTF2)
		{
			auto hash = ZED::SHA512(ZED::File("..\\..\\bin\\arial.ttf"));

			Assert::IsTrue(hash == "DD2102535EC432F893250AD5DCACB1CE4CA08E498666ED6A1AEBF118B81467E1B3CE9516FBEE929911F3043C156EC0DBF67B9182AAD6AB31B7C38A75CCFE2E01");
		}
	};
}