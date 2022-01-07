#pragma once
#include <string>



class Timer
{
public:
	static uint32_t GetTimestamp();
	static std::string TimestampToString(uint32_t Timestamp);

	Timer() = default;
	Timer(uint32_t Timestamp) : Timer() { m_TimeElapsed = Timestamp; }

	void Start();
	void Pause();
	void Reset();
	void Stop();//Same as Pause() followed by Reset()

	bool IsRunning() const { return m_Running; }

	std::string ToString() const;
	std::string ToStringWithHundreds() const;
	std::string ToStringInSeconds() const;
	std::string ToStringOnlySeconds() const;

	uint32_t GetElapsedTime() const;//Returns elapsed time in milliseconds

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