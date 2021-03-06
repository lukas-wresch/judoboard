#pragma once
#include <string>
#include "core.h"



namespace ZED
{
	class DLLEXPORT Timer
	{
	public:
		static std::string TimestampToString(uint32_t Timestamp);

		Timer() = default;
		Timer(uint32_t Timestamp) : Timer() { m_TimeElapsed = Timestamp; }

		void Start();
		void Pause();
		void Reset();
		void Stop();//Same as Pause() followed by Reset()

		bool IsRunning() const { return m_Running; }

		std::string ToString() const;
		std::string ToStringInSeconds() const;

		uint32_t GetElapsedTime() const;//Returns elapsed time in milliseconds
		float GetElapsedTimeInSeconds() const;

		operator uint32_t () const { return GetElapsedTime(); }
		operator float () const { return (float)GetElapsedTime() / 1000.0f; }

	private:
		uint32_t m_Timestamp = 0;
		uint32_t m_TimeElapsed = 0;
		bool m_Running = false;
	};



	inline Timer operator -(int Timestamp, const Timer& rhs)
	{
		int diff = Timestamp - rhs.GetElapsedTime();
		if (diff < 0)
			diff = 0;
		return Timer(diff);
	}
}