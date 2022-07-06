#pragma once
#include <string>
#include "id.h"



namespace Judoboard
{
	enum class Gender
	{
		Unknown = -1,
		Male, Female
	};



	inline std::string Latin1ToUTF8(const std::string& Input)
	{
		std::string ret;
		
		for (unsigned char c : Input)
		{
			if (c < 128)
				ret += c;
			else
			{
				ret += 0xc2 + (c > 0xbf);
				ret += (c & 0x3f) + 0x80;
			}
		}

		return ret;
	}



	inline void ReplaceAll(std::string& Haystack, const std::string& Needle, const std::string& ReplaceBy)
	{
		size_t start_pos = 0;

		while ((start_pos = Haystack.find(Needle, start_pos)) != std::string::npos)
		{
			Haystack.replace(start_pos, Needle.length(), ReplaceBy);
			start_pos += ReplaceBy.length();
		}
	}
}