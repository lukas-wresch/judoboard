#pragma once
#include <array>
#include <string>
#include "core.h"
#include "blob.h"



namespace ZED
{
	class SHA256
	{
	public:
		SHA256() = default;
		DLLEXPORT SHA256(const uint8_t* String, size_t Length);
		SHA256(const char* String) : SHA256((uint8_t*)String, strlen(String)) {}
		SHA256(const std::string& String) : SHA256((uint8_t*)String.c_str(), String.length()) {}
		SHA256(const Blob& Data) : SHA256((const uint8_t*)Data, Data.GetSize()) {}

		DLLEXPORT bool operator == (const std::string& HexString) const;
		bool operator != (const char* HexString) const { return !(*this == HexString); }

		DLLEXPORT operator std::string () const;
		DLLEXPORT operator Blob () const;
		
	private:
		void Transform(const uint8_t* String, uint32_t NumberOfBlocks);

		uint32_t m_Hash[8] = { 0x6a09e667,
							   0xbb67ae85,
							   0x3c6ef372,
							   0xa54ff53a,
							   0x510e527f,
							   0x9b05688c,
							   0x1f83d9ab,
							   0x5be0cd19 };
	};
}