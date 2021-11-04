#include "CppUnitTest.h"
#include "../include/png.h"
#include "../include/renderer_sdl2.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace UnitTests
{
	TEST_CLASS(PNG)
	{
	public:

		TEST_METHOD(LoadFile)
		{
			ZED::PNG png("..\\..\\bin\\pngnow.png");

			Assert::IsTrue(png.IsValid());
		}


		TEST_METHOD(SaveFile)
		{
			ZED::PNG png("..\\..\\bin\\pngnow.png");

			Assert::IsTrue(png.IsValid());

			Assert::IsTrue(png.Save("..\\..\\bin\\pngnow2.png"));

			Assert::IsTrue(png.IsValid());
		}


		TEST_METHOD(CreateCircle)
		{
			ZED::TextureSDL2 tex(256, 256);

			tex.CreateCircle();

			ZED::PNG png(tex, tex.GetWidth(), tex.GetHeight(), ZED::ColorType::R8G8B8);

			Assert::IsTrue(png.IsValid());

			Assert::IsTrue(png.Save("..\\..\\bin\\circle.png"));

			Assert::IsTrue(png.IsValid());
		}		
	};
}