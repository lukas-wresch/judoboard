#include "../ZED/include/log.h"
#include "matchtable.h"
#include "match.h"
#include "tournament.h"



using namespace Judoboard;



MatchTable::~MatchTable()
{
	for (auto match : m_Schedule)
		delete match;
}



void MatchTable::SetMatID(int32_t MatID)
{
	Schedulable::SetMatID(MatID);

	for (auto match : m_Schedule)
		if (match)
			match->SetMatID(MatID);
}



bool MatchTable::IsIncluded(const Judoka& Fighter) const
{
	for (auto& participant : m_Participants)
		if (participant && participant->GetID() == Fighter.GetID())
			return true;
	return false;
}



bool MatchTable::AddMatch(Match* NewMatch)
{
	if (!NewMatch)
		return false;

	NewMatch->SetMatchTable(this);
	m_ManualMatches.emplace_back(NewMatch);
	m_Schedule.emplace_back(NewMatch);
	return true;
}



bool MatchTable::AddParticipant(Judoka* NewParticipant, bool Force)
{
	if (!NewParticipant)
		return false;

	if (!Force)//Don't check if we are adding we are forcing the judoka in this match table
		if (!IsElgiable(*NewParticipant))//Is the judoka allowed in this match table?
			return false;

	m_Participants.push_back(NewParticipant);
	return true;
}



const std::vector<const Match*> MatchTable::FindMatches(const Judoka& Fighter1, const Judoka& Fighter2) const
{
	std::vector<const Match*> ret;

	for (auto match : m_Schedule)
	{
		if (!match->HasValidFighters())
			continue;

		if ( (*match->GetFighter(Fighter::White) == Fighter1  && *match->GetFighter(Fighter::Blue)  == Fighter2) ||
			 (*match->GetFighter(Fighter::Blue)  == Fighter1  && *match->GetFighter(Fighter::White) == Fighter2) )
			ret.push_back(match);
	}

	return ret;
}



bool MatchTable::Contains(const Judoka* Judoka) const
{
	if (!Judoka)
		return false;

	for (auto participant : m_Participants)
		if (participant && *participant == *Judoka)
			return true;

	return false;
}



const RuleSet& MatchTable::GetRuleSet() const
{
	if (m_Rules)
		return *m_Rules;
	if (GetTournament() && GetTournament()->GetDefaultRuleSet())
		return *GetTournament()->GetDefaultRuleSet();

	ZED::Log::Error("Could not find rule set");

	static RuleSet* default_rules = new RuleSet;
	return *default_rules;
}



int MatchTable::CompareFighterScore(const void* A, const void* B)
{
	const Result* a = (Result*)A;
	const Result* b = (Result*)B;

	if (a->Wins < b->Wins)
		return 1;
	if (a->Wins > b->Wins)
		return -1;

	if (a->Score < b->Score)
		return 1;
	if (a->Score > b->Score)
		return -1;

	//Direct comparision
	auto matches = a->MatchTable->FindMatches(*a->Judoka, *b->Judoka);

	int a_wins = 0,  b_wins = 0;
	int a_score = 0, b_score = 0;

	for (auto match : matches)
	{
		if (!match->HasConcluded())
			continue;

		auto result = match->GetMatchResult();

		if (result.m_Winner != Winner::Draw && match->GetWinningJudoka()->GetID() == a->Judoka->GetID())
		{
			a_wins++;
			a_score += (int)match->GetMatchResult().m_Score;
		}

		if (result.m_Winner != Winner::Draw && match->GetWinningJudoka()->GetID() == b->Judoka->GetID())
		{
			b_wins++;
			b_score += (int)match->GetMatchResult().m_Score;
		}
	}

	if (a_wins < b_wins)
		return 1;
	if (a_wins > b_wins)
		return -1;

	if (a_score < b_score)
		return 1;
	if (a_score > b_score)
		return -1;

	if (a->Time < b->Time)
		return -1;
	if (a->Time > b->Time)
		return 1;

	//This ensures that everything is well-ordered, however if this is necessary a flag is raised!
	if (a->Judoka->GetID() < b->Judoka->GetID())
	{
		a->NotSortable = b->NotSortable = true;
		return -1;
	}
	if (a->Judoka->GetID() > b->Judoka->GetID())
	{
		a->NotSortable = b->NotSortable = true;
		return 1;
	}

	return 0;
}



bool MatchTable::Result::operator < (const Result& rhs) const
{
	if (Wins < rhs.Wins)
		return false;
	if (Wins > rhs.Wins)
		return true;

	if (Score < rhs.Score)
		return false;
	if (Score > rhs.Score)
		return true;

	//Direct comparision
	auto matches = MatchTable->FindMatches(*Judoka, *rhs.Judoka);

	int a_wins  = 0, b_wins  = 0;
	int a_score = 0, b_score = 0;

	for (auto match : matches)
	{
		if (!match->HasConcluded())
			continue;

		auto result = match->GetMatchResult();

		if (result.m_Winner != Winner::Draw && match->GetWinningJudoka()->GetID() == Judoka->GetID())
		{
			a_wins++;
			a_score += (int)match->GetMatchResult().m_Score;
		}

		if (result.m_Winner != Winner::Draw && match->GetWinningJudoka()->GetID() == rhs.Judoka->GetID())
		{
			b_wins++;
			b_score += (int)match->GetMatchResult().m_Score;
		}
	}

	if (a_wins < b_wins)
		return false;
	if (a_wins > b_wins)
		return true;

	if (a_score < b_score)
		return false;
	if (a_score > b_score)
		return true;

	if (Time < rhs.Time)
		return true;
	if (Time > rhs.Time)
		return false;

	//This ensures that everything is well-ordered, however if this is necessary a flag is raised!
	if (Judoka->GetID() < rhs.Judoka->GetID())
	{
		NotSortable = rhs.NotSortable = true;
		return true;
	}
	if (Judoka->GetID() > rhs.Judoka->GetID())
	{
		NotSortable = rhs.NotSortable = true;
		return false;
	}

	return false;
}



const std::string MatchTable::ToString() const
{
	ZED::CSV ret;
	ret << GetID() << GetScheduleIndex() << GetMatID() << GetColor() << GetRuleSet().GetID() << m_Name;
	return ret;
}



MatchTable::MatchTable(ZED::CSV& Stream, const ITournament* Tournament) : Schedulable(Stream, Tournament)
{
	Stream >> m_Name;

	bool rulesAvailable;
	Stream >> rulesAvailable;
	if (rulesAvailable && Tournament)
	{
		std::string rulesUUID;
		Stream >> rulesUUID;
		m_Rules = Tournament->FindRuleSet(UUID(std::move(rulesUUID)));
	}

	//TODO import manual matches
}



void MatchTable::operator >> (ZED::CSV& Stream) const
{
	Stream << GetType();

	Schedulable::operator >>(Stream);
	Stream << m_Name;

	if (m_Rules)
	{
		Stream << true << (std::string)m_Rules->GetUUID();
		Stream.AddNewline();
	}
	else
		Stream << false;//No rule set

	//TODO export manual matches
}