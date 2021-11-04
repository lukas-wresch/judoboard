#pragma once
#include <array>
#include <string>
#include "core.h"
#include "blob.h"



namespace ZED
{
	class SHA512
	{
	public:
		SHA512() = default;
		DLLEXPORT SHA512(const uint8_t* String, size_t Length);
		SHA512(const char* String) : SHA512((uint8_t*)String, strlen(String)) {}
		SHA512(const std::string& String) : SHA512((uint8_t*)String.c_str(), String.length()) {}
		SHA512(const Blob& Data) : SHA512((const uint8_t*)Data, Data.GetSize()) {}

		DLLEXPORT bool operator == (const std::string& HexString) const;
		bool operator != (const char* HexString) const { return !(*this == HexString); }

		DLLEXPORT operator std::string () const;
		DLLEXPORT operator Blob () const;
		
	private:
		void Transform(const uint8_t* String, uint32_t NumberOfBlocks);

		uint64_t m_Hash[8] = { 0x6a09e667f3bcc908ULL,
							   0xbb67ae8584caa73bULL,
							   0x3c6ef372fe94f82bULL,
							   0xa54ff53a5f1d36f1ULL,
							   0x510e527fade682d1ULL,
							   0x9b05688c2b3e6c1fULL,
							   0x1f83d9abfb41bd6bULL,
							   0x5be0cd19137e2179ULL };
	};
}