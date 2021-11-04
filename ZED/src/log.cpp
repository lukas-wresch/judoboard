#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif
#include <stdio.h>
#include <mutex>
#include "../include/log.h"
#include "../include/core.h"



using namespace ZED;


FILE* g_LogFile = nullptr;
std::mutex g_lock;



bool Log::Open(const std::string& Filename)
{
	if (!g_LogFile)
		fopen_s(&g_LogFile, Filename.c_str(), "w");

	return g_LogFile;
}



void Log::Add(const std::string& LogMessage, Type Type)
{
	Open();

	g_lock.lock();

	if (!g_LogFile)
		return;//No access to log file

	std::string type_string = "[???] ";
	if (Type == Type::Error)
		type_string = "[ERROR] ";
	else if (Type == Type::Warning)
		type_string = "[WARNING] ";
	else if (Type == Type::Info)
		type_string = "[INFO] ";
	else if (Type == Type::Debug)
		type_string = "[DEBUG] ";

	const std::string time = std::to_string(Core::CurrentTime());

	fprintf(g_LogFile, "%s", ("[" + time + "]: " + type_string + LogMessage + "\n").c_str());

	if (Type == Type::Error || Type == Type::Warning)
		fflush(g_LogFile);

	g_lock.unlock();

#ifdef _DEBUG
#ifdef _WIN32
	OutputDebugStringA((LogMessage + "\n").c_str());
#endif
#endif

	printf("%s", (LogMessage + "\n").c_str());
}



void Log::Close()
{
	if (g_LogFile)
	{
		fclose(g_LogFile);
		g_LogFile = nullptr;
	}
}