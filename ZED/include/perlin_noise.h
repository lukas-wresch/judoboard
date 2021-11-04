#pragma once
#include "core.h"
#include "twister.h"



namespace ZED
{
	class PerlinNoise
	{
	public:
		PerlinNoise(uint32_t Seed) : m_Seed(Seed) {}

		DLLEXPORT double Noise2D(double X, double Y);

	private:
		double Noise(int X, int Y);
		double SmoothedNoise(int X, int Y);
		double Interpolate(double A, double B, double X);

		uint32_t m_Seed;
	};
}