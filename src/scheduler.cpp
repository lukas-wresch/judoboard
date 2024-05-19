#include "scheduler.h"



using namespace Judoboard;



MatchParcel::MatchParcel(const MatchTable& Table, size_t Count)
	: m_Table(Table)
{
	auto schedule = Table.GetSchedule();
	for (auto match : schedule)
	{
		m_Matches.emplace_back(match);
		m_Count++;
		if (m_Count >= Count)
			break;
	}
	m_StartIndex = m_Count;
}



void MatchParcel::operator << (size_t Count)
{
	auto schedule = m_Table.GetSchedule();
	for (size_t i = 0; i < Count; ++i)
	{
		assert(schedule[m_StartIndex + i]);
		m_Matches.emplace_back(schedule[m_StartIndex + i]);
	}

	m_StartIndex += Count;
	m_Count      += Count;
}



void MatchParcel::operator += (MatchParcel& rhs)
{
	assert(m_Table == rhs.m_Table);
	assert(m_StartIndex + m_Count == rhs.m_StartIndex);
	m_StartIndex += rhs.m_Count;
}



MatchParcel MatchParcel::Split()
{
	MatchParcel ret(m_Table, m_StartIndex, m_Count/2);

	//right half
	m_StartIndex = ret.m_StartIndex + ret.m_Count;
	m_Count      = m_Count - ret.m_Count;//Remainder

	assert(m_Table == ret.m_Table);
	assert(m_StartIndex == ret.m_StartIndex + ret.m_Count);
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
		parcels.insert(parcels.end(), new_parcels.begin(), new_parcels.end());
	}
}