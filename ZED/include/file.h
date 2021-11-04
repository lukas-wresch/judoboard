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
		
		bool IsOpen() const { return m_File; }

		DLLEXPORT void Close();
		DLLEXPORT size_t GetSize() const;

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