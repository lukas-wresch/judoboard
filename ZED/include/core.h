#pragma once
#include <stdlib.h>
#include "zed.h"
#include "blob.h"



namespace ZED
{
	enum class ColorType : uint32_t
	{
		R8G8B8,
		B8G8R8,
		R8G8B8A8
	};

	DLLEXPORT uint32_t ColorTypeSize(ColorType ColorType);//Returns the size in bytes of a give color type



	struct Color
	{
		Color() = default;
		Color(unsigned char r_, unsigned char g_, unsigned char b_, unsigned char a_ = 255)
		{
			r = r_;
			g = g_;
			b = b_;
			a = a_;
		}

		unsigned char r, g, b, a;
	};


	struct ColorF
	{
		ColorF() = default;
		ColorF(float r_, float g_, float b_)
		{
			r = r_;
			g = g_;
			b = b_;
		}

		float r, g, b;
	};



	namespace Core
	{
		struct Date
		{
			uint32_t year;
			uint32_t month;
			uint32_t day;

			uint32_t hour;
			uint32_t minute;
			uint32_t second;
		};


#ifdef _WIN32
		const std::string Separator("\\");//Should be marked as constexpr when C++ 20 is targeted
		const std::string Newline("\r\n");//Should be marked as constexpr when C++ 20 is targeted
#else
		const std::string Separator("/");
		const std::string Newline("\n");
#endif

		//Files
		DLLEXPORT bool RemoveFile(const char* Filename);
		inline bool RemoveFile(const std::string& Filename) { return RemoveFile(Filename.c_str()); }

		//Conversions
		unsigned int hex2int(const std::string& hexstring);
		constexpr unsigned int hex2int_constexpr(char hexstring[2]);//Will be obsolete once C++ 20 is targeted
		DLLEXPORT unsigned int hex2int(char hex);
		DLLEXPORT std::string  int2hex(unsigned int value);
		DLLEXPORT std::string  int2hex(unsigned int value, unsigned int width = 2);//Converts a value to a hexadecimal string with (width amount of) leading zeros

		DLLEXPORT std::string Base64Encode(const uint8_t* Data, size_t Length);
		inline std::string Base64Encode(const std::string& Data) { return Base64Encode((uint8_t*)Data.c_str(), Data.size()); }
		DLLEXPORT Blob Base64Decode(const std::string& Base64String);

		DLLEXPORT int ToInt(const std::string& Number);//Returns -1 if no number could be parsed
		DLLEXPORT std::string IP2String(uint32_t IP);

		//Timing
		DLLEXPORT uint32_t CurrentTimestamp();
		DLLEXPORT double CurrentTime();//Returns the time in seconds as double since this funtion is first called. One the first call this function returns 0.0
		DLLEXPORT void Pause(uint32_t Milliseconds);//Pauses execution for the given amount of millseconds

		inline std::chrono::time_point<std::chrono::high_resolution_clock> StartHighPrecisionTimer()
		{
			return std::chrono::high_resolution_clock::now();
		}

		inline double StopHighPrecisionTimer(std::chrono::time_point<std::chrono::high_resolution_clock>& Timer)
		{
			return (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - Timer)).count();
		}

		DLLEXPORT Date GetDate();//Returns the current date

		//File management
		DLLEXPORT bool Indexer(std::function<bool(const std::string&)> onFile, const std::string& FolderName = ".", bool Recru = true);
	}
}