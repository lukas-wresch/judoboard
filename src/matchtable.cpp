#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
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
		if (participant && participant->GetUUID() == Fighter.GetUUID())
			return true;
	return false;
}



bool MatchTable::AddMatch(Match* NewMatch)
{
	if (!NewMatch)
		return false;

	NewMatch->SetMatchTable(this);
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

		if (result.m_Winner != Winner::Draw && match->GetWinner()->GetUUID() == Judoka->GetUUID())
		{
			a_wins++;
			a_score += (int)match->GetMatchResult().m_Score;
		}

		if (result.m_Winner != Winner::Draw && match->GetWinner()->GetUUID() == rhs.Judoka->GetUUID())
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
	if ((std::string)Judoka->GetUUID() < (std::string)rhs.Judoka->GetUUID())
	{
		NotSortable = rhs.NotSortable = true;
		return true;
	}
	if ((std::string)Judoka->GetUUID() > (std::string)rhs.Judoka->GetUUID())
	{
		NotSortable = rhs.NotSortable = true;
		return false;
	}

	return false;
}



const std::string MatchTable::ToString() const
{
	ZED::CSV ret;
	ret << (std::string)GetUUID() << GetType() << GetScheduleIndex() << GetMatID() << GetColor() << (std::string)GetRuleSet().GetUUID() << m_Name;
	return ret;
}



MatchTable::MatchTable(ZED::CSV& Stream, ITournament* Tournament) : Schedulable(Stream, Tournament)
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

	size_t participants;
	Stream >> participants;
	for (size_t i = 0; i < participants; i++)
	{
		std::string uuid;
		Stream >> uuid;

		if (Tournament)
		{
			auto judoka = Tournament->FindParticipant(UUID(std::move(uuid)));

			if (judoka)
				m_Participants.emplace_back(judoka);
			else
				ZED::Log::Error("Participant could not be found");
		}
	}
}



MatchTable::MatchTable(const YAML::Node& Yaml, ITournament* Tournament) : Schedulable(Yaml, Tournament)
{
	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();

	if (Yaml["rule_set"])
		m_Rules = Tournament->FindRuleSet(Yaml["rule_set"].as<std::string>());

	if (Yaml["age_group"])
		m_pAgeGroup = Tournament->FindAgeGroup(Yaml["age_group"].as<std::string>());

	if (Yaml["participants"] && Yaml["participants"].IsSequence())
	{
		for (const auto& node : Yaml["participants"])
		{
			auto participant = Tournament->FindParticipant(node.as<std::string>());
			if (participant)
				m_Participants.emplace_back(participant);
		}
	}
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

	Stream << m_Participants.size();
	for (auto judoka : m_Participants)
		Stream << (std::string)judoka->GetUUID();
}



void MatchTable::operator >> (YAML::Emitter& Yaml) const
{
	Schedulable::operator >>(Yaml);

	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();
	if (m_Name.length() > 0)
		Yaml << YAML::Key << "name" << YAML::Value << m_Name;

	if (m_Rules)
		Yaml << YAML::Key << "rule_set" << YAML::Value << (std::string)m_Rules->GetUUID();
	if (m_pAgeGroup)
		Yaml << YAML::Key << "age_group" << YAML::Value << (std::string)m_pAgeGroup->GetUUID();

	Yaml << YAML::Key << "participants";
	Yaml << YAML::BeginSeq;

	for (auto judoka : m_Participants)
		Yaml << (std::string)judoka->GetUUID();

	Yaml << YAML::EndSeq;
}



void MatchTable::ToString(YAML::Emitter& Yaml) const
{
	Schedulable::operator >>(Yaml);

	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();
	Yaml << YAML::Key << "name" << YAML::Value << m_Name;
	Yaml << YAML::Key << "description" << YAML::Value << GetDescription();

	if (m_Rules)
		Yaml << YAML::Key << "rule_set" << YAML::Value << (std::string)m_Rules->GetUUID();
	if (m_pAgeGroup)
		Yaml << YAML::Key << "age_group" << YAML::Value << (std::string)m_pAgeGroup->GetUUID();

	Yaml << YAML::Key << "participants";
	Yaml << YAML::BeginSeq;

	for (auto judoka : m_Participants)
		judoka->ToString(Yaml);

	Yaml << YAML::EndSeq;
}



Match* MatchTable::AddAutoMatch(size_t WhiteIndex, size_t BlueIndex)
{
	if (!GetParticipant(WhiteIndex) || !GetParticipant(BlueIndex))
	{
		ZED::Log::Error("Illegal participant index");
		return nullptr;
	}

	return CreateAutoMatch(GetParticipant(WhiteIndex), GetParticipant(BlueIndex));
}



Match* MatchTable::CreateAutoMatch(Judoka* White, Judoka* Blue)
{
	Match* new_match = new Match(GetTournament(), White, Blue, GetMatID());
	new_match->SetAutoGenerated();
	new_match->SetMatchTable(this);
	m_Schedule.emplace_back(new_match);
	return new_match;
}



Match* MatchTable::AddMatchForWinners(Match* Match1, Match* Match2)
{
	Match* new_match = CreateAutoMatch(nullptr, nullptr);

	//Is this an 'empty' match?
	if (!Match1->HasDependentMatches() && Match1->GetSingleValidFighters())
		new_match->SetFighter(Fighter::White, Match1->GetSingleValidFighters());
	else
		new_match->SetDependency(Fighter::White, Match::DependencyType::TakeWinner, Match1);

	if (!Match2->HasDependentMatches() && Match2->GetSingleValidFighters())
		new_match->SetFighter(Fighter::Blue, Match2->GetSingleValidFighters());
	else
		new_match->SetDependency(Fighter::Blue,  Match::DependencyType::TakeWinner, Match2);

	return new_match;
}



std::pair<size_t, size_t> MatchTable::GetIndicesOfMatch(const Match* Match) const
{
	std::pair ret(0, 0);

	if (!Match || !Match->HasValidFighters())
	{
		ZED::Log::Error("Illegal match");
		return ret;
	}

	for (int i = 0; i < m_Participants.size(); ++i)
	{
		if (m_Participants[i]->GetUUID() == Match->GetFighter(Fighter::White)->GetUUID())
			ret.first = i;
		if (m_Participants[i]->GetUUID() == Match->GetFighter(Fighter::Blue)->GetUUID())
			ret.second = i;
	}	

	return ret;
}



size_t MatchTable::GetIndexOfParticipant(const Judoka* Participant) const
{
	if (!Participant)
		return 0;

	for (size_t i = 0; i < m_Participants.size(); ++i)
	{
		if (Participant && Participant->GetUUID() == m_Participants[i]->GetUUID())
			return i;
	}

	return 0;
}