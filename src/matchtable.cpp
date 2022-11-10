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
	m_MatID = MatID;

	for (auto match : m_Schedule)
		if (match)
			match->SetMatID(MatID);
}



Match* MatchTable::FindMatch(const UUID& UUID) const
{
	for (auto match : m_Schedule)
		if (match && match->GetUUID() == UUID)
			return match;
	return nullptr;
}



Judoka* MatchTable::FindParticipant(const UUID& UUID) const
{
	for (auto participant : m_Participants)
		if (participant && participant->GetUUID() == UUID)
			return participant;
	return nullptr;
}



bool MatchTable::IsIncluded(const Judoka& Fighter) const
{
	for (auto participant : m_Participants)
		if (participant && participant->GetUUID() == Fighter.GetUUID())
			return true;
	return false;
}



bool MatchTable::AddMatch(Match* NewMatch)
{
	if (!NewMatch)
		return false;

	AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::White)), true);
	AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::Blue)),  true);

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

	//Is the judoka alreay a participant?
	for (auto judoka : m_Participants)
		if (judoka && *judoka == *NewParticipant)
			return false;

	m_Participants.push_back(NewParticipant);
	GenerateSchedule();
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

		auto result = match->GetResult();

		if (result.m_Winner != Winner::Draw && match->GetWinner()->GetUUID() == Judoka->GetUUID())
		{
			a_wins++;
			a_score += (int)match->GetResult().m_Score;
		}

		if (result.m_Winner != Winner::Draw && match->GetWinner()->GetUUID() == rhs.Judoka->GetUUID())
		{
			b_wins++;
			b_score += (int)match->GetResult().m_Score;
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



MatchTable::MatchTable(const YAML::Node& Yaml, ITournament* Tournament) : m_Tournament(Tournament)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["mat_id"])
		m_MatID = Yaml["mat_id"].as<int>();

	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();

	if (Yaml["rule_set"] && Tournament)
		m_Rules = Tournament->FindRuleSet(Yaml["rule_set"].as<std::string>());

	if (Yaml["age_group"] && Tournament)
		m_pAgeGroup = Tournament->FindAgeGroup(Yaml["age_group"].as<std::string>());

	if (Yaml["participants"] && Yaml["participants"].IsSequence() && Tournament)
	{
		for (const auto& node : Yaml["participants"])
		{
			auto participant = Tournament->FindParticipant(node.as<std::string>());
			if (participant)
				m_Participants.emplace_back(participant);
		}
	}

	if (Yaml["matches"] && Yaml["matches"].IsSequence())
	{
		for (const auto& node : Yaml["matches"])
			m_Schedule.emplace_back(new Match(node, this, Tournament));
	}
}



void MatchTable::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "schedule_index" << YAML::Value << m_ScheduleIndex;
	Yaml << YAML::Key << "mat_id" << YAML::Value << m_MatID;
	Yaml << YAML::Key << "color"  << YAML::Value << (int)m_Color;

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

	Yaml << YAML::Key << "matches";
	Yaml << YAML::BeginSeq;

	for (auto match : m_Schedule)
		*match >> Yaml;

	Yaml << YAML::EndSeq;
}



void MatchTable::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "schedule_index" << YAML::Value << m_ScheduleIndex;
	Yaml << YAML::Key << "mat_id" << YAML::Value << m_MatID;
	Yaml << YAML::Key << "color"  << YAML::Value << (int)m_Color;

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
	{
		Yaml << YAML::BeginMap;
		Yaml << YAML::Key << "uuid"      << YAML::Value << (std::string)judoka->GetUUID();
		Yaml << YAML::Key << "firstname" << YAML::Value << judoka->GetFirstname();
		Yaml << YAML::Key << "lastname"  << YAML::Value << judoka->GetLastname();
		Yaml << YAML::Key << "weight"    << YAML::Value << judoka->GetWeight().ToString();
		if (GetStartingPosition(judoka) != -1)
			Yaml << YAML::Key << "starting_pos" << YAML::Value << GetStartingPosition(judoka);
		Yaml << YAML::EndMap;
	}

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



Match* MatchTable::CreateAutoMatch(const Judoka* White, const Judoka* Blue)
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
	//if (!Match1->HasDependentMatches() && Match1->GetSingleValidFighters())
		//new_match->SetFighter(Fighter::White, Match1->GetSingleValidFighters());
	//else if (!Match1->IsCompletelyEmptyMatch())
		new_match->SetDependency(Fighter::White, Match::DependencyType::TakeWinner, Match1);

	//if (!Match2->HasDependentMatches() && Match2->GetSingleValidFighters())
		//new_match->SetFighter(Fighter::Blue, Match2->GetSingleValidFighters());
	//else if (!Match2->IsCompletelyEmptyMatch())
		new_match->SetDependency(Fighter::Blue,  Match::DependencyType::TakeWinner, Match2);

	return new_match;
}



size_t MatchTable::FindMatchIndex(const UUID& UUID) const
{
	for (size_t i = 0; m_Schedule.size(); ++i)
		if (m_Schedule[i] && m_Schedule[i]->GetUUID() == UUID)
			return i;
	return -1;
}



std::pair<size_t, size_t> MatchTable::GetParticipantIndicesOfMatch(const Match* Match) const
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
		if (Participant->GetUUID() == m_Participants[i]->GetUUID())
			return i;
	}

	return 0;
}