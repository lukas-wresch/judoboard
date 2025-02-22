#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#ifdef LINUX
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <uchar.h>
#endif

#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "../include/core.h"


#ifdef _WIN32
#pragma comment(lib, "winmm.lib")
#endif


using namespace ZED;



int first_sec = 0;
int first_msec = 0;
int first_usec = 0;



uint32_t ZED::ColorTypeSize(ZED::ColorType ColorType)
{
	switch (ColorType)
	{
		case ColorType::R8G8B8:
		case ColorType::B8G8R8:
			return 3;
		case ColorType::R8G8B8A8:
			return 4;
	}

	return 0;
}



bool Core::RemoveFile(const char* Filename)
{
#ifdef _WIN32
	return DeleteFileA(Filename);
#else
	return remove(Filename) == 0;
#endif
}



unsigned int Core::hex2int(const std::string& hexstring)
{
	std::stringstream ss;
	ss << std::hex << hexstring;
	unsigned int value;
	ss >> value;
	return value;
}



constexpr unsigned int Core::hex2int_constexpr(char hexstring[2])
{
	unsigned int value = 0;

	for (int i = 0; hexstring[i]; i++)
	{
		const char c = hexstring[i];
		value *= 16;

		if ('0' <= c && c <= '9')
			value += c - '0';
		else if ('a' <= c && c <= 'f')
			value += c - 'a' + 10;
		else if ('A' <= c && c <= 'F')
			value += c - 'A' + 10;
	}

	return value;
}



unsigned int Core::hex2int(char hex)
{
	std::stringstream ss;
	ss << std::hex << hex;
	unsigned int value;
	ss >> value;
	return value;
}



std::string Core::int2hex(unsigned int value)
{
	std::stringstream stream;
	stream << std::hex << value;
	return stream.str();
}



std::string Core::int2hex(unsigned int value, unsigned int width)
{
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(width) << std::right << std::hex << value;
	return stream.str();
}



std::string Core::Base64Encode(const uint8_t* Data, size_t Length)
{
	const char base64_chars[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	std::string ret;
	int i = 0;
	int j = 0;
	uint8_t char_array_3[3];
	uint8_t char_array_4[4];

	for (; Length > 0; Length--)
	{
		char_array_3[i++] = *(Data++);
		if (i == 3)
		{
			char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] =   char_array_3[2] & 0x3f;

			for (i = 0; i < 4 ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i > 0)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] =   char_array_3[2] & 0x3f;

		for (j = 0; j < i + 1; j++)
			ret += base64_chars[char_array_4[j]];

		for (; i < 3; i++)
			ret += '=';
	}

	return ret;
}



Blob Core::Base64Decode(const std::string& Base64String)
{
	const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	auto is_base64 = [](unsigned char c)
	{
		return isalnum(c) || (c == '+') || (c == '/');
	};

	int in_len = Base64String.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	uint8_t char_array_4[4];
	uint8_t char_array_3[3];
	Blob ret(in_len);

	while (in_len-- && ( Base64String[in_] != '=') && is_base64(Base64String[in_]))
	{
		char_array_4[i++] = Base64String[in_];
		in_++;

		if (i == 4)
		{
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; i < 3; i++)
				ret << char_array_3[i];
			i = 0;
		}
	}

	if (i > 0)
	{
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] =  (char_array_4[0] << 2)        + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

		for (j = 0; j < i - 1; j++)
			ret << char_array_3[j];
	}

	return ret;
}



int Core::ToInt(const std::string& Number)
{
	int ret = -1;

	try
	{
		ret = std::stoi(Number);
	}
	catch (...)
	{
		return -1;
	}

	return ret;
}



std::string Core::IP2String(uint32_t IP)
{
	char address[32];
	snprintf(address, sizeof(address), "%d.%d.%d.%d", (IP & 0xFF000000) >> 24,
													  (IP & 0x00FF0000) >> 16,
													  (IP & 0x0000FF00) >> 8,
													  (IP & 0x000000FF));
	return std::string(address);
}



uint32_t Core::CurrentTimestamp()
{
	return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}



double Core::CurrentTime()
{
#ifdef _WIN32
	DWORD time = timeGetTime();

	if (!first_sec)
		first_sec = time;

	return ((double)(time - first_sec)) / 1000.0;

#elif LINUX
	//linux version
	struct timeval new_time;

	//get current time
	gettimeofday(&new_time, NULL);

	//set if not set
	if (!first_sec)
	{
		first_sec  = new_time.tv_sec;
		first_usec = new_time.tv_usec;
	}

	return (new_time.tv_sec - first_sec) + ((new_time.tv_usec - first_usec) * 0.000001);
#else
	return 0.0;
#endif
}



void Core::Pause(uint32_t Milliseconds)
{
	std::this_thread::sleep_for((std::chrono::milliseconds)Milliseconds);
/*#ifdef _WIN32
	Sleep(Milliseconds);
#else
	usleep(Milliseconds * 1000);
#endif*/
}



Core::Date Core::GetDate()
{
	Date ret;

#ifdef _WIN32
	SYSTEMTIME date;
	GetLocalTime(&date);

	ret.year  = date.wYear;
	ret.month = date.wMonth;
	ret.day   = date.wDay;

	ret.hour   = date.wHour;
	ret.minute = date.wMinute;
	ret.second = date.wSecond;
#else
	time_t t = time(NULL);
	tm date = *localtime(&t);

	ret.year  = 1900 + date.tm_year;
	ret.month = date.tm_mon + 1;
	ret.day   = date.tm_mday;

	ret.hour   = date.tm_hour;
	ret.minute = date.tm_min;
	ret.second = date.tm_sec;
#endif

	return ret;
}



bool Core::Indexer(std::function<bool(const std::string&)> onFile, const std::string& FolderName, bool Recru)
{
#ifdef _WIN32
	std::string SearchString = FolderName + "/*.*";

	WIN32_FIND_DATAA Find;
	HANDLE Handle = FindFirstFileA(SearchString.c_str(), &Find);

	while (FindNextFileA(Handle, &Find))
	{
		if (Find.cFileName[0] != '.')
		{
			if (strlen(Find.cFileName) >= 1 && !(Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string Filename;

				if (FolderName[0] != '.')
					Filename = FolderName + ZED::Core::Separator + Find.cFileName;
				else
					Filename = Find.cFileName;

				if (!onFile(Filename))
				{
					FindClose(Handle);
					return false;
				}
			}

			else if (Find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && Recru)
				Indexer(onFile, FolderName + ZED::Core::Separator + Find.cFileName);
		}
	}

	FindClose(Handle);
#elif LINUX
	DIR* dp;
	dirent* dirp;

	dp = opendir(FolderName.c_str());

	if (!dp) return false;

	while ((dirp = readdir(dp)) != NULL)
	{
		if (dirp->d_name[0] == '.')
			continue;

		if (dirp->d_type == DT_REG)
		{
			std::string Filename;

			if (FolderName[0] != '.')
				Filename = FolderName + ZED::Core::Separator + dirp->d_name;
			else
				Filename = dirp->d_name;

			if (!onFile(Filename))
			{
				closedir(dp);
				return false;
			}
		}

		else if (dirp->d_type == DT_DIR && Recru)
			Indexer(onFile, FolderName + ZED::Core::Separator + dirp->d_name);
	}

	closedir(dp);
#endif
	return true;
}