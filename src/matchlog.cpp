#include <sstream>
#include "matchlog.h"



using namespace Judoboard;



const std::string MatchLog::ToString() const
{
	std::stringstream ret;

	bool comma_needed = false;
	for (auto& event : m_Events)
	{
		if (comma_needed)
			ret << ",";
		ret << event.m_Timestamp << "," << std::to_string((int)event.m_Group) << "," << std::to_string((int)event.m_Event);
		comma_needed = true;
	}

	return ret.str();
}



void MatchLog::operator << (ZED::CSV& Stream)
{
	uint32_t size;
	Stream >> size;

	for (uint32_t i = 0; i < size; i++)
	{
		Event event;
		Stream >> event.m_Group >> event.m_Event >> event.m_Timestamp;
		m_Events.emplace_back(event);
	}
}



void MatchLog::operator >> (ZED::CSV& Stream) const
{
	Stream << m_Events.size();

	for (auto& event : m_Events)
		Stream << event.m_Group << event.m_Event << event.m_Timestamp;
}