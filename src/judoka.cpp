#include "../ZED/include/core.h"
#include "judoka.h"
#include "database.h"



using namespace Judoboard;



Judoka::Judoka(const std::string& Firstname, const std::string& Lastname, uint16_t Weight, Gender Gender) : m_Firstname(Firstname), m_Lastname(Lastname)
{
	if (Weight < 1000)
		m_Weight = Weight;
	if (Gender == Gender::Male || Gender == Gender::Female)
		m_Gender = Gender;
}



Judoka::Judoka(ZED::CSV& Stream, const StandingData* pStandingData)
{
	std::string uuid, club_uuid;
	Stream >> m_Firstname >> m_Lastname >> m_Weight >> m_Gender >> m_Birthyear >> uuid >> club_uuid;
	SetUUID(std::move(uuid));

	if (pStandingData && club_uuid.length() > 1)
		m_pClub = pStandingData->FindClub(UUID(std::move(club_uuid)));
}



void Judoka::operator >> (ZED::CSV& Stream) const
{
	Stream << m_Firstname;
	Stream << m_Lastname;
	Stream << m_Weight << m_Gender << m_Birthyear;
	Stream << (std::string)GetUUID();

	if (m_pClub)
		Stream << (std::string)m_pClub->GetUUID();
	else
		Stream << "?";

	Stream.AddNewline();//Also needed to flush the stream
}



uint16_t Judoka::GetAge() const
{
	auto date = ZED::Core::GetDate();
	return date.year - m_Birthyear;
}



/*const std::string Judoka::GetShortName() const
{
	if (m_Firstname.length() > 20)
		return m_Firstname;
	else if (GetName().length() > 20)
		return m_Firstname + " " + m_Lastname[0] + ".";
	return GetName();
}*/



void Judoka::SetWeight(uint16_t NewWeight)
{
	if (NewWeight < 1000)
		m_Weight = NewWeight;
}



const std::string Judoka::ToString() const
{
	ZED::CSV ret;
	ret << GetID() << m_Firstname << m_Lastname << m_Weight << m_Gender << m_Birthyear;
	return ret;
}