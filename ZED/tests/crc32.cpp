#include "CppUnitTest.h"
#include "../include/crc32.h"
#include "../include/file.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace UnitTests
{
	TEST_CLASS(CRC32)
	{
	public:

		TEST_METHOD(String)
		{
			auto hash = ZED::CRC32("test");

			Assert::IsTrue(hash == "d87f7e0c");
			Assert::IsTrue(hash == "D87F7e0c");
			Assert::IsTrue(hash == "D87F7E0C");

			auto hash2 = ZED::CRC32("");

			Assert::IsTrue(hash2 == "00000000");
		}


		TEST_METHOD(ChecksumOfArrialTTF)
		{
			auto file = ZED::File("..\\..\\bin\\arial.ttf");
			Assert::IsTrue(file);

			ZED::Blob data;
			file >> data;

			Assert::IsTrue(data.GetSize() == 275572);

			auto hash = ZED::CRC32(data);

			Assert::IsTrue(hash == "44E1DF76");
		}
	};
}