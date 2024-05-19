#include "scheduler.h"



using namespace Judoboard;



MatchParcel::MatchParcel(const MatchTable& Table, size_t Count)
	: m_Table(Table)
{
	assert(Count > 0);

	auto schedule = Table.GetSchedule();
	for (auto match : schedule)
	{
		m_Matches.emplace_back(*match);
		Count--;
		if (Count <= 0)
			break;
	}
	m_StartIndex = GetSize();
}



void MatchParcel::operator << (size_t Count)
{
	auto schedule = m_Table.GetSchedule();
	for (size_t i = 0; i < Count; ++i)
	{
		assert(schedule[m_StartIndex + i]);
		m_Matches.emplace_back(*schedule[m_StartIndex + i]);
	}

	m_StartIndex += Count;
	//m_Count      += Count;
}



void MatchParcel::operator = (MatchParcel& rhs)
{
	assert(m_Table.GetUUID() == rhs.m_Table.GetUUID());
	m_Matches = rhs.m_Matches;
}



void MatchParcel::operator = (MatchParcel&& rhs) noexcept
{
	assert(m_Table.GetUUID() == rhs.m_Table.GetUUID());
	m_Matches = std::move(rhs.m_Matches);
}



void MatchParcel::operator += (MatchParcel& rhs)
{
	assert(m_Table == rhs.m_Table);
	assert(m_StartIndex + GetSize() == rhs.m_StartIndex);
	m_StartIndex += rhs.GetSize();
}



MatchParcel MatchParcel::Split()
{
	MatchParcel ret(m_Table);
	ret.m_StartIndex = m_StartIndex;

	auto count = GetSize();

	for (size_t i = 0; i < count/2; ++i)
	{
		ret.m_Matches.emplace_back(m_Matches[0]);
		m_Matches.erase(m_Matches.begin());
	}

	//right half
	m_StartIndex = ret.m_StartIndex + ret.GetSize();

	assert(m_Table == ret.m_Table);
	assert(m_StartIndex == ret.m_StartIndex + ret.GetSize());
	return ret;
}



Delivery& Delivery::AddRemainder()
{
	return Append(m_Table.GetSchedule().size() - GetSize());
}



void Scheduler::Start(std::vector<const MatchTable*> Plan)
{
	//Generate parcels
	std::vector<MatchParcel> parcels;

	for (auto table : Plan)
	{
		auto new_parcels = table->GetMatchParcels();
		//Append new_parcels
		//parcels.insert(parcels.end(), new_parcels.begin(), new_parcels.end());
	}
}