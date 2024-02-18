#pragma once
#include "core.h"
#include "blob.h"
#include <stdio.h>



namespace ZED
{
	class File
	{
	public:
		File() = delete;
		DLLEXPORT File(const char* Filename, bool WriteAccess = false);
		File(const std::string& Filename, bool WriteAccess = false) : File(Filename.c_str(), WriteAccess) {}

		~File() { Close(); }

		static bool Exists(const char* Filename);
		static bool Exists(const std::string& Filename) { return Exists(Filename.c_str()); }
		static size_t GetSize(const char* Filename);
		static size_t GetSize(const std::string& Filename) { return GetSize(Filename.c_str()); }
		
		bool IsOpen() const { return m_File; }

		DLLEXPORT void Close();
		DLLEXPORT size_t GetSize() const;

		DLLEXPORT void Seek(int32_t RelativeOffset);

		DLLEXPORT bool Write(const uint8_t Value);//Write single byte
		DLLEXPORT bool Write(const char* String);//Write string without \0
		DLLEXPORT bool Write(const std::string& String) { return Write(String.c_str()); }//Write string without \0
		DLLEXPORT bool Write(const void* Data, size_t Size);
		DLLEXPORT bool Read(void* Data, size_t Size) const;

		DLLEXPORT void operator >> (Blob& Data);
		DLLEXPORT void operator << (const Blob& Data);

		template<typename T>
		void operator << (const T& Data)
		{
			Write(&Data[0], sizeof(T));
		}

		template<typename T>
		void operator >> (T& Data) const
		{
			Read(&Data[0], sizeof(T));
		}

		operator bool () const { return IsOpen(); }
		operator FILE* () { return m_File; }

		FILE* GetFileHandle() { return m_File; }

	private:
		FILE* m_File = nullptr;
	};
}