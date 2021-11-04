#include "CppUnitTest.h"
#include "../include/png.h"
#include "../include/perlin_noise.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace UnitTests
{
	TEST_CLASS(PerlinNoise)
	{
	public:

		TEST_METHOD(WriteToPNG)
		{
			const int width  = 256;
			const int height = 256;

			ZED::PerlinNoise noise(1337423141);
			const double scale = 0.17;
			const int octaves  = 4;

			double* data = new double[width * height];
			for (int x = 0; x < width; x++)
			{
				for (int y = 0; y < height; y++)
				{
					double freq = scale;
					double amp  = 1.0;

					double v = 0.0;
					for (int i = 0; i < octaves; i++)
					{
						v += amp * noise.Noise2D(freq * x, freq * y);
						freq *= 2.0;
						amp  *= 0.5;
					}

					const int index = y * width + x;
					data[index] = v;
				}
			}

			//Normalize
			double min = data[0];
			double max = data[0];

			for (int i = 0; i < width * height; i++)
			{
				const double v = data[i];
				if (v > max)
					max = v;
				if (v < min)
					min = v;
			}

			for (int i = 0; i < width * height; i++)
				data[i] = (data[i] - min)/(max-min);


			ZED::Image img(width, height, ZED::ColorType::R8G8B8);

			for (int x = 0; x < width; x++)
			{
				for (int y = 0; y < height; y++)
				{
					const int index = y * width + x;
					const double v = data[index];

					const int color = (int)(v * 255.0);

					img.SetPixel(x, y, ZED::Color(color, color, color));
				}
			}

			ZED::PNG png(std::move(img));

			Assert::IsTrue(png.IsValid());

			Assert::IsTrue(png.Save("..\\..\\bin\\noise.png"));

			Assert::IsTrue(png.IsValid());
		}		
	};
}