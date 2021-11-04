#include "CppUnitTest.h"
#include "../include/crypto.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace UnitTests
{
	TEST_CLASS(Crypto)
	{
	public:

		TEST_METHOD(SimpleTest)
		{
			ZED::Crypto::KeyPair pair;

			auto sign = pair.Sign("Hello");
			Assert::IsTrue(pair.Verify("Hello", sign));
			Assert::IsFalse(pair.Verify("NotHello", sign));

			ZED::Crypto::KeyPair pair2(pair);
			Assert::IsTrue(pair2.Verify("Hello", sign));
			Assert::IsFalse(pair2.Verify("NotHello", sign));

			ZED::Crypto::PublicKey pk(pair);
			Assert::IsTrue(pk.Verify("Hello", sign));
			Assert::IsFalse(pk.Verify("NotHello", sign));
		}


		
	};
}