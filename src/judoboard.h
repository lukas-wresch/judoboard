#pragma once
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include <string>
#include <cassert>
#include "id.h"



namespace Judoboard
{
	enum class Gender
	{
		Unknown = -1,
		Male, Female
	};

	inline void operator++ (Gender& g)
	{
		g = (Gender)(((int)g) + 1);
	}

	inline void operator++ (Gender& g, int)
	{
		g = (Gender)(((int)g) + 1);
	}



	class Weight
	{
	public:
		Weight(uint32_t InKg) {
			m_InGrams = InKg * 1000;
		}
		Weight(const std::string& Input) {
			bool commaFound = false;
			int digitsAfterComma = 0;
			for (char c : Input)
			{
				if (c == ',' || c == '.')
					commaFound = true;
				else
				{
					m_InGrams *= 10;
					m_InGrams += (int)c - '0';
					if (commaFound)
						digitsAfterComma++;
				}
			}

			if (digitsAfterComma == 0)
				m_InGrams *= 1000;
			else if (digitsAfterComma == 1)
				m_InGrams *= 100;
			else if (digitsAfterComma == 2)
				m_InGrams *= 10;
		}
		Weight(const YAML::Node& Yaml) {
			m_InGrams = Yaml.as<uint32_t>();
		}

		void operator >> (YAML::Emitter& Yaml) const {
			Yaml << m_InGrams;
		}
		std::string ToString() const {
			if (m_InGrams % 1000 == 0)
				return std::to_string(m_InGrams / 1000);
			//Add space at the end so that yaml doesn't think that this is a number
			return std::to_string(m_InGrams/1000) + "," + std::to_string((m_InGrams%1000) / 100) + " ";
		}

		operator uint32_t () const {//TODO make explicit
			return m_InGrams;
		}

		Weight operator + (const Weight& rhs) {
			Weight ret;
			ret.m_InGrams = this->m_InGrams + rhs.m_InGrams;
			return ret;
		}

		Weight operator / (int Divisor) {
			Weight ret;
			ret.m_InGrams = this->m_InGrams / Divisor;
			return ret;
		}

		void operator /= (int Divisor) {
			this->m_InGrams /= Divisor;
		}
		

	private:
		Weight() = default;

		uint32_t m_InGrams = 0;
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

	inline size_t UTF8_GetLength(unsigned char c)
	{
		if (c < 0x80) return 1;
		else if (!(c & 0x20)) return 2;
		else if (!(c & 0x10)) return 3;
		else if (!(c & 0x08)) return 4;
		else if (!(c & 0x04)) return 5;
		else return 6;
	}

	inline unsigned char UTF8_GetValue(std::string::const_iterator& it)
	{
		size_t len = UTF8_GetLength(*it);

		if (len == 1)
			return *it;

		unsigned char res = (unsigned char) (*it & (0xff >> (len + 1))) << ((len - 1) * 6);

		for (--len; len; --len)
			res |= ((unsigned char) (*(++it)) - 0x80) << ((len - 1) * 6);

		return res;
	}

	inline std::string UTF8ToLatin1(const std::string& Input)
	{
		std::string ret;

		for (auto it = Input.begin(); it != Input.end(); ++it)
		{
			unsigned char value = UTF8_GetValue(it);
			ret += (char)value;
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