#include <chrono>
#include <sstream>
#include <iomanip>
#include "../include/core.h"
#include "../include/timer.h"


using namespace ZED;



std::string Timer::TimestampToString(uint32_t Timestamp)
{
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(1) << (Timestamp / 1000) / 60 << ":" << std::setw(2) << (Timestamp / 1000) % 60;
	return stream.str();
}



void Timer::Start()
{
	m_TimeElapsed = GetElapsedTime();
	m_Timestamp = Core::CurrentTimestamp();
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
	m_Timestamp = Core::CurrentTimestamp();
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
	return Core::CurrentTimestamp() - m_Timestamp + m_TimeElapsed;
}



float Timer::GetElapsedTimeInSeconds() const
{
	return (float)GetElapsedTime() / 1000.0f;
}