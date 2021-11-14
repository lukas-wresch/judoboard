#pragma once
#include <string>
#include "id.h"



namespace Judoboard
{
	enum class Gender
	{
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
}