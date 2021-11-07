#include <sstream>
#include <fstream>
#include <ZED/include/csv.h>
#include <ZED/include/log.h>
#include "match.h"
#include "tournament.h"



using namespace Judoboard;



Match::Match(const ITournament* Tournament, Judoka* White, Judoka* Blue, uint32_t MatID) : Schedulable(Tournament)
{
	m_White.m_Judoka = White;
	m_Blue.m_Judoka  = Blue;
	SetMatID(MatID);
}



Match::Match(ZED::CSV& Stream, ITournament* Tournament) : Schedulable(Stream, Tournament)
{
	std::string whiteUUID, blueUUID;

	Stream >> whiteUUID >> blueUUID;
	Stream >> m_State;
	Stream >> m_Result.m_Winner >> m_Result.m_Score >> m_Result.m_Time;

	std::string matchtableUUID;
	Stream >> matchtableUUID;

	if (matchtableUUID.length() > 1)
		m_Table = Tournament->FindMatchTable(UUID(std::move(matchtableUUID)));

	m_White.m_Judoka = Tournament->FindParticipant(UUID(std::move(whiteUUID)));
	m_Blue.m_Judoka  = Tournament->FindParticipant(UUID(std::move(blueUUID)));

	if (HasConcluded() || IsRunning())
		m_Log << Stream;
}



void Match::operator >>(ZED::CSV& Stream) const
{
	Schedulable::operator >>(Stream);

	if (!GetFighter(Fighter::White) || !GetFighter(Fighter::Blue))
		ZED::Log::Error("NOT IMPLEMENTED");//DEBUG NOT IMPLEMENTED
	else
	{
		Stream << GetFighter(Fighter::White)->GetUUID() << GetFighter(Fighter::Blue)->GetUUID() << m_State;
		Stream << m_Result.m_Winner << m_Result.m_Score << m_Result.m_Time;

		if (m_Table)
			Stream << m_Table->GetUUID();
		else
			Stream << "-";
	}

	if (HasConcluded() || IsRunning())
		m_Log >> Stream;

	Stream.AddNewline();//Also needed to flush the stream
}



Judoka* Match::GetFighter(Fighter Fighter)
{
	if (Fighter == Fighter::White)
		return m_White.m_Judoka;

	return m_Blue.m_Judoka;
}



const Judoka* Match::GetFighter(Fighter Fighter) const
{
	if (Fighter == Fighter::White)
		return m_White.m_Judoka;

	return m_Blue.m_Judoka;
}



bool Match::Contains(const Judoka& Judoka) const
{
	if (m_White.m_Judoka && m_White.m_Judoka->GetID() == Judoka.GetID())
		return true;

	if (m_Blue.m_Judoka  && m_Blue.m_Judoka->GetID()  == Judoka.GetID())
		return true;

	return false;
}



const Judoka* Match::GetWinningJudoka() const
{
	if (!HasConcluded())
		return nullptr;

	auto result = GetMatchResult();

	if (result.m_Winner == Winner::Draw)
		return nullptr;

	if (result.m_Winner == Winner::White)
		return m_White.m_Judoka;
	else
		return m_Blue.m_Judoka;
}



const std::vector<Match*> Match::GetDependentMatches()
{
	std::vector<Match*> ret;

	if (m_White.m_PreviousMatch)
		ret.emplace_back(m_White.m_PreviousMatch);
	if (m_Blue.m_PreviousMatch)
		ret.emplace_back(m_Blue.m_PreviousMatch);

	return ret;
}



const std::vector<const Match*> Match::GetDependentMatches() const
{
	std::vector<const Match*> ret;

	if (m_White.m_PreviousMatch)
		ret.emplace_back(m_White.m_PreviousMatch);
	if (m_Blue.m_PreviousMatch)
		ret.emplace_back(m_Blue.m_PreviousMatch);

	return ret;
}



const RuleSet& Match::GetRuleSet() const
{
	if (m_Rules)
		return *m_Rules;
	if (m_Table)
		return m_Table->GetRuleSet();
	if (GetTournament() && GetTournament()->GetDefaultRuleSet())
		return *GetTournament()->GetDefaultRuleSet();

	ZED::Log::Error("Could not find rule set");

	static RuleSet* default_rules = new RuleSet;
	return *default_rules;
}



ZED::CSV Match::ToString() const
{
	ZED::CSV ret;

	ret << GetID();

	if (!GetFighter(Fighter::White) || !GetFighter(Fighter::Blue))
		ret << "???,???";
	else
		ret << GetFighter(Fighter::White)->GetName() << GetFighter(Fighter::Blue)->GetName();

	ret << GetMatID() << m_State << GetColor().ToHexString();

	if (GetMatchTable())
		ret << GetMatchTable()->GetID() << GetMatchTable()->GetName();
	else
		ret << "0,- - -";

	return ret;
}



ZED::CSV Match::AllToString() const
{
	return ToString() << m_Result.m_Winner << m_Result.m_Score << m_Result.m_Time << GetRuleSet().GetID();
}