#pragma once
#include <array>
#include <sstream>
#include <iomanip>
#include "core.h"
#include "blob.h"



namespace ZED
{
	class CRC32
	{
	public:
		DLLEXPORT CRC32(const char* String);
		CRC32(const std::string& String) : CRC32(String.c_str()) {}

		DLLEXPORT CRC32(const uint8_t* Data, size_t Size);
		CRC32(const Blob& Data) : CRC32(Data, Data.GetSize()) {}

		DLLEXPORT bool operator ==(const char* HexString) const;
		bool operator ==(const std::string& HexString) const { return *this == HexString.c_str(); }
		bool operator ==(const uint32_t Hash) const { return m_Hash == Hash; }
		bool operator !=(const char* HexString) const { return !(*this == HexString); }
		bool operator !=(const std::string& HexString) const { return !(*this == HexString.c_str()); }
		bool operator !=(const uint32_t Hash) const { return m_Hash != Hash; }

		operator std::string ()
		{
			std::stringstream stream;
			stream << std::hex << std::setw(8) << std::setfill('0') << m_Hash;
			return stream.str();
		}

		operator uint32_t ()
		{
			return m_Hash;
		}

		static constexpr uint32_t Reflect(uint32_t ref, char ch)
		{
			uint32_t value = 0;
			for (int i = 1; i < ch + 1; i++, ref >>= 1)
			{
				if (ref & 1)
					value |= 1 << (ch - i);
			}
			return value;
		}


		static constexpr std::array<uint32_t, 256> InitCrcTable()
		{
			std::array<uint32_t, 256> crc32_table = {};

			const uint32_t polynomial = 0x04c11db7;

			for (int i = 0; i < 256; i++)
			{
				crc32_table[i] = Reflect(i, 8) << 24;
				for (int j = 0; j < 8; j++)
					crc32_table[i] = (crc32_table[i] << 1) ^ ((crc32_table[i] & 0x80000000) ? polynomial : 0);
				crc32_table[i] = Reflect(crc32_table[i], 32);
			}

			return crc32_table;
		}

	private:
		uint32_t m_Hash = 0;
	};
}