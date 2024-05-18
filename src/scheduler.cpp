#include "scheduler.h"



using namespace Judoboard;



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



void MatchParcel::operator += (MatchParcel& rhs)
{
	assert(m_Table == rhs.m_Table);
	assert(m_StartIndex + m_Count == rhs.m_StartIndex);
	m_StartIndex += rhs.m_Count;
}



Delivery Delivery::AddRemainder()
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