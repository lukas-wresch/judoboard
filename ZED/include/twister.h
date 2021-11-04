#pragma once
#include "core.h"



#define mtRand_N      624



namespace ZED
{
	class Twister
	{
	public:
		DLLEXPORT Twister(uint32_t Seed);
		DLLEXPORT float Rand();
		DLLEXPORT uint32_t RandL();

	private:
		uint32_t mt[mtRand_N];
		int mti;
	};
}