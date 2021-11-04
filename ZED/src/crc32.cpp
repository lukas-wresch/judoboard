#include <sstream>
#include <iomanip>
#include "../include/crc32.h"


using namespace ZED;



constexpr std::array<uint32_t, 256> g_crc32_table = CRC32::InitCrcTable();



CRC32::CRC32(const char* String)
{
	m_Hash = 0xFFFFFFFF;
	
	for (size_t i = 0; String[i]; i++)
		m_Hash = (m_Hash >> 8) ^ g_crc32_table[(m_Hash & 0x000000FF) ^ String[i]];

	m_Hash = ~m_Hash;
}



CRC32::CRC32(const uint8_t* Data, size_t Size)
{
	m_Hash = 0xFFFFFFFF;

	for (size_t i = 0; i < Size; i++)
		m_Hash = (m_Hash >> 8) ^ g_crc32_table[(m_Hash & 0x000000FF) ^ Data[i]];

	m_Hash = ~m_Hash;
}



bool CRC32::operator ==(const char* HexString) const
{
	std::stringstream stream;
	stream << std::hex << std::setw(8) << std::setfill('0') << m_Hash;
#ifdef _WIN32
	return _stricmp(stream.str().c_str(), HexString) == 0;
#else
	return strcasecmp(stream.str().c_str(), HexString) == 0;
#endif
}