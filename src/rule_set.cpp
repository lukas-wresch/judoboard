#include "rule_set.h"



using namespace Judoboard;



RuleSet::RuleSet(const std::string& Name, uint32_t MatchTime, int GoldenScoreTime, uint32_t OsaeKomiTime, uint32_t OsaeKomiTime_with_Wazaari, bool Yuko, bool Koka, bool Draw, uint32_t BreakTime) : m_Name(Name)
{
	m_Yuko = Yuko;
	m_Koka = Koka;
	m_Draw = Draw;

	m_MatchTime = MatchTime;
	m_GoldenScoreTime = GoldenScoreTime;
	if (GoldenScoreTime < 0)
		m_GoldenScoreTime = -1;

	m_OsaeKomiTime = OsaeKomiTime;
	m_OsaeKomiTime_With_WazaAri = OsaeKomiTime_with_Wazaari;

	m_BreakTime = BreakTime;
}



RuleSet::RuleSet(ZED::CSV& Stream)
{
	Stream >> m_Name;

	std::string uuid;
	Stream >> uuid;
	SetUUID(std::move(uuid));

	Stream >> m_MatchTime >> m_GoldenScoreTime >> m_OsaeKomiTime >> m_OsaeKomiTime_With_WazaAri >> m_BreakTime;
	Stream >> m_Yuko >> m_Koka >> m_Draw;
}



void RuleSet::operator >>(ZED::CSV& Stream) const
{
	Stream << m_Name;

	Stream << GetUUID();

	Stream << m_MatchTime << m_GoldenScoreTime << m_OsaeKomiTime << m_OsaeKomiTime_With_WazaAri << m_BreakTime;
	Stream << m_Yuko << m_Koka << m_Draw;
	Stream.AddNewline();//Also needed to flush the stream
}



bool RuleSet::IsOutOfTime(int MatchTime_Millseconds, bool GoldenScore) const
{
	if (!GoldenScore)
	{
		if (m_MatchTime < 0)//Infinite match time?
			return false;
		return MatchTime_Millseconds > m_MatchTime * 1000;
	}

	if (m_GoldenScoreTime < 0)//Infinite golden score time?
		return false;
	return MatchTime_Millseconds > m_GoldenScoreTime * 1000;
}



const std::string RuleSet::GetDescription() const
{
	std::string ret;
	if (m_MatchTime < 0)
		ret += "&infin; ";
	else
		ret += Timer::TimestampToString(m_MatchTime * 1000) + " ";

	if (IsGoldenScoreEnabled())
	{
		if (m_GoldenScoreTime < 0)
			ret += "(+&infin;) ";
		else
			ret += "(+" + Timer::TimestampToString(m_GoldenScoreTime * 1000) + ") ";
	}

	return ret + std::to_string(m_OsaeKomiTime) + "|" + std::to_string(m_OsaeKomiTime_With_WazaAri);
}