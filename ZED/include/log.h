#pragma once
#include <string>
#include "core.h"



namespace ZED
{
	class DLLEXPORT Log
	{
	public:
		enum class Type
		{
			Error,
			Warning,
			Info,
			Debug
		};

		static void Info( const std::string& LogMessage) { Add(LogMessage, Type::Info); }
		static void Warn( const std::string& LogMessage) { Add(LogMessage, Type::Warning); }
		static void Error(const std::string& LogMessage) { Add(LogMessage, Type::Error); }
#ifdef _DEBUG
		static void Debug(const std::string& LogMessage) { Add(LogMessage, Type::Debug); }
#else
		static void Debug(const std::string& LogMessage) {}
#endif

		static bool Open(const std::string& Filename = "zed_log.txt");
		static void Close();

	private:
#ifdef ZED_NO_LOG
		static void Add(const std::string& LogMessage, Type Type = Type::Info) {}
#else
		static void Add(const std::string& LogMessage, Type Type = Type::Info);
#endif
	};
}