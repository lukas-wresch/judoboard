#include "timer.h"
#include <chrono>
#include <sstream>
#include <iomanip>

/*#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "winmm.lib")
#else
#include <sys/time.h>
#endif



#ifndef _WIN32
uint32_t timeGetTime()
{
	timeval new_time;

	gettimeofday(&new_time, NULL);//get the current time

	return new_time.tv_sec*1000 + new_time.tv_usec/1000;
}
#endif*/



uint32_t Timer::GetTimestamp()
{
	return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	//return timeGetTime();
}



std::string Timer::TimestampToString(uint32_t Timestamp)
{
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(1) << (Timestamp / 1000) / 60 << ":" << std::setw(2) << (Timestamp / 1000) % 60;
	return stream.str();
}



void Timer::Start()
{
	m_TimeElapsed = GetElapsedTime();
	m_Timestamp = GetTimestamp();
	m_Running = true;
}



void Timer::Pause()
{
	m_TimeElapsed = GetElapsedTime();
	m_Running = false;
}



void Timer::Reset()
{
	m_TimeElapsed = 0;
}



void Timer::Stop()
{
	m_TimeElapsed = 0;
	m_Running = false;
}



std::string Timer::ToString() const
{
	auto time = GetElapsedTime();

	std::stringstream stream;
	//stream << std::setfill('0') << std::setw(1) << (time/1000)/60 << ":" << std::setw(2) << (time/1000)%60 << "." << std::setw(3) << time%1000;
	stream << std::setfill('0') << std::setw(1) << (time / 1000) / 60 << ":" << std::setw(2) << (time / 1000) % 60 << "." << std::setw(1) << (time % 1000)/100;
	return stream.str();
}



std::string Timer::ToStringInSeconds() const
{
	return std::to_string(GetElapsedTime() / 1000);
}



uint32_t Timer::GetElapsedTime() const
{
	if (!m_Running)
		return m_TimeElapsed;
	return GetTimestamp() - m_Timestamp + m_TimeElapsed;
}