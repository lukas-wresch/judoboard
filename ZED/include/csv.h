#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include "core.h"



namespace ZED
{
	class CSV
	{
	public:
		CSV() = default;
		//CSV(CSV& rhs) : m_line(rhs.m_line), m_stream(rhs.m_stream.str()) {}
		CSV(CSV& rhs)// : m_line(rhs.m_line), m_stream(rhs.m_stream.str())
		{
			rhs.Flush();
			m_stream.str(rhs.m_stream.str());

			m_stream.seekg(rhs.m_stream.tellg());
			//m_stream.seekg(0, std::ios_base::end);
			m_stream.seekp(rhs.m_stream.tellp());
			//m_stream.seekp(0, std::ios_base::end);
			//m_stream << rhs.m_stream.rdbuf();
			if (!m_stream.str().empty() && !m_line.empty())
				m_stream << ',';
			Flush();
		}

		CSV(const CSV& rhs)// : m_line(rhs.m_line), m_stream(rhs.m_stream.str())
		{
			m_stream.str((std::string)rhs);

			m_stream.seekg(rhs.m_stream.tellg());
			//m_stream.seekg(0, std::ios_base::end);
			m_stream.seekp(rhs.m_stream.tellp());
			//m_stream.seekp(0, std::ios_base::beg);
			//m_stream << rhs.m_stream.rdbuf();
			if (!m_stream.str().empty() && !m_line.empty())
				m_stream << ',';
			Flush();
		}

		//CSV(const CSV& rhs) = delete;
		/*CSV(const CSV& rhs) : m_line(rhs.m_line)
		{
			m_stream.str(rhs.m_stream.str());
			//m_stream.seekg(rhs.m_stream.tellg());
			if (!rhs.m_stream.str().empty())
			{
				//m_stream << rhs.m_stream.rdbuf();
				Flush();
			}
			else
			{
				m_stream = std::stringstream(m_line);
				m_line.clear();
			}
		}*/

		/*CSV(const CSV& rhs) : m_line(rhs.m_line)
		{
			m_stream << rhs.m_stream.str();
		}*/

		void operator = (CSV& rhs)
		{
			m_line   = rhs.m_line;
			m_stream = std::stringstream(rhs.m_stream.str());
		}

		//CSV(const char* Input) : m_line(Input), m_stream(m_line) {}
		CSV(const char* Input) : m_line(Input) {}
		CSV(const std::string& Input) : m_line(Input), m_stream(m_line) {}
		CSV(const std::ifstream& Input)
		{
			if (Input.is_open())
				m_stream << Input.rdbuf();
		}

		/*~CSV()
		{
			//m_stream.~basic_stringstream();
			//m_line.~basic_string();
		}*/

		template<typename T>
		CSV& operator << (const T& Value)
		{
			if constexpr (std::is_enum<T>::value)
				Append((std::underlying_type_t<T> &)Value);
			else
				Append(Value);
			return *this;
		}

		CSV& operator >> (std::ofstream& Output)
		{
			Flush();
			Output << m_stream.rdbuf();
			return *this;
		}

		template<typename T>
		CSV& operator >> (T& Value)
		{
			if constexpr (std::is_enum<T>::value)
				Read((std::underlying_type_t<T> &)Value);
			else
				Read(Value);
			return *this;
		}

		template<typename T>
		void Add(const T& Value) { Append(Value); }

		template <typename T, typename... Ts>
		void Add(T First, Ts... Rest)
		{
			Append(First);
			Add(Rest...);
		}

		const std::string get() { Flush(); return m_stream.str(); }
		operator const std::string () const { Flush(); return m_stream.str(); }

		DLLEXPORT void AddNewline();//Adds \n or \r\n depending on the platform

		DLLEXPORT void Ignore();

	private:
		DLLEXPORT void AppendImpl(const char* Value);

		void Append(const std::string& Value) { AppendImpl(Value.c_str()); }
		//void Append(uint32_t Value) { AppendImpl(std::to_string(Value).c_str()); }
		void Append(int Value)  { AppendImpl(std::to_string(Value).c_str()); }
		void Append(char* Value) { AppendImpl(Value); }
		void Append(const char Value[]) { AppendImpl(Value); }
		void Append(bool Value) { AppendImpl(Value ? std::to_string(1).c_str() : std::to_string(0).c_str()); }

		void Read(std::string& Value)
		{
			Ignore();
			/*std::getline(m_stream, Value, '\n');
			if (Value.length() >= 1 && Value[Value.length()-1] == '\r')
			{
				Value.pop_back();
				Ignore();
			}*/
			Value.clear();
			while (m_stream.peek() != EOF)
			{
				char c = m_stream.get();
				if (c == ',' || c == '\r' || c == '\n' || c == '\0')
					break;
				Value += c;
			}
		}

		template<typename T>
		void Read(T& Value)
		{
			Ignore();
			m_stream >> Value;
		}

		DLLEXPORT void Flush() const;

		mutable std::string m_line;
		mutable std::stringstream m_stream;
	};
}