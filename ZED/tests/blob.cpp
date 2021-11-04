#include "CppUnitTest.h"
#include "../include/blob.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace UnitTests
{
	TEST_CLASS(Blob)
	{
	public:

		TEST_METHOD(SimpleTest)
		{
			ZED::Blob blob;

			Assert::IsTrue(blob.GetSize() == 0);

			char data[16];
			blob.Append(data, 10);

			Assert::IsTrue(blob.GetSize() == 10);
		}


		TEST_METHOD(Read)
		{
			ZED::Blob blob;

			Assert::IsTrue(blob.GetSize() == 0);

			char data[16] = "Hello";
			blob.Append(data, 5);

			Assert::IsTrue(blob.GetSize() == 5);

			char data2[16];
			blob >> data2;

			Assert::IsTrue(data2[0] == 'H');
			Assert::IsTrue(data2[1] == 'e');
			Assert::IsTrue(data2[2] == 'l');
			Assert::IsTrue(data2[3] == 'l');
			Assert::IsTrue(data2[4] == 'o');
		}


		TEST_METHOD(Copy)
		{
			ZED::Blob blob;

			Assert::IsTrue(blob.GetSize() == 0);

			char data[16] = "Hello";
			blob.Append(data, 5);

			Assert::IsTrue(blob.GetSize() == 5);

			ZED::Blob copy(blob);

			Assert::IsTrue(copy.GetSize() == 5);
			char data2[16];
			copy >> data2;

			Assert::IsTrue(data2[0] == 'H');
			Assert::IsTrue(data2[1] == 'e');
			Assert::IsTrue(data2[2] == 'l');
			Assert::IsTrue(data2[3] == 'l');
			Assert::IsTrue(data2[4] == 'o');
		}


		TEST_METHOD(DeepCopy)
		{
			ZED::Blob* blob = new ZED::Blob(0);

			Assert::IsTrue(blob->GetSize() == 0);

			char data[16] = "Hello";
			blob->Append(data, 5);

			Assert::IsTrue(blob->GetSize() == 5);

			ZED::Blob copy(*blob);

			Assert::IsTrue(copy.GetSize() == 5);
			char data2[16];
			copy >> data2;
			
			delete blob;

			Assert::IsTrue(data2[0] == 'H');
			Assert::IsTrue(data2[1] == 'e');
			Assert::IsTrue(data2[2] == 'l');
			Assert::IsTrue(data2[3] == 'l');
			Assert::IsTrue(data2[4] == 'o');
		}



		TEST_METHOD(SharedCopy)
		{
			ZED::Blob blob;

			Assert::IsTrue(blob.GetSize() == 0);

			char data[16] = "Hello";
			blob.Append(data, 5);

			Assert::IsTrue(blob.GetSize() == 5);

			{
				ZED::Blob copy(blob);
				copy.MakeShared();

				Assert::IsTrue(copy.GetSize() == 5);
				char data2[16];
				copy >> data2;

				Assert::IsTrue(data2[0] == 'H');
				Assert::IsTrue(data2[1] == 'e');
				Assert::IsTrue(data2[2] == 'l');
				Assert::IsTrue(data2[3] == 'l');
				Assert::IsTrue(data2[4] == 'o');
			}

			Assert::IsTrue(blob.GetSize() == 5);
			char data2[16];
			blob >> data2;

			Assert::IsTrue(data2[0] == 'H');
			Assert::IsTrue(data2[1] == 'e');
			Assert::IsTrue(data2[2] == 'l');
			Assert::IsTrue(data2[3] == 'l');
			Assert::IsTrue(data2[4] == 'o');
		}
	};
}