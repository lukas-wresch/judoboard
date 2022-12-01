#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../ZED/include/log.h"
#include "matchtable.h"
#include "match.h"
#include "filter.h"
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



bool MatchTable::HasConcluded() const
{
	for (auto match : m_Schedule)
		if (match && !match->HasConcluded())
			return false;
	return true;
}



Match* MatchTable::FindMatch(const UUID& UUID) const
{
	for (auto match : m_Schedule)
		if (match && match->GetUUID() == UUID)
			return match;
	return nullptr;
}



size_t MatchTable::FindMatchIndex(const UUID& UUID) const
{
	for (size_t i = 0; i < m_Schedule.size(); ++i)
		if (m_Schedule[i] && m_Schedule[i]->GetUUID() == UUID)
			return i;
	return -1;
}



Judoka* MatchTable::FindParticipant(const UUID& UUID) const
{
	if (!m_Filter)
		return nullptr;

	for (auto participant : m_Filter->GetParticipants())
		if (participant && participant->GetUUID() == UUID)
			return participant;

	return nullptr;
}



const std::vector<Judoka*>& MatchTable::GetParticipants() const
{
	if (m_Filter)
		return m_Filter->GetParticipants();

	const std::vector<Judoka*> ret;
	return ret;
}



bool MatchTable::IsIncluded(const Judoka& Fighter) const
{
	if (!m_Filter)
		return false;

	for (auto participant : m_Filter->GetParticipants())
		if (participant && participant->GetUUID() == Fighter.GetUUID())
			return true;

	return false;
}



Status MatchTable::GetStatus() const
{
	if (m_Schedule.size() == 0)
		return Status::Scheduled;

	bool one_match_finished = false;
	bool all_matches_finished = true;

	for (auto match : m_Schedule)
	{
		if (match->IsEmptyMatch() || match->IsBestOfThree())
			continue;

		if (!match->HasConcluded())
			all_matches_finished = false;

		if (match->IsRunning() || match->HasConcluded())
			one_match_finished = true;
	}

	if (all_matches_finished)
		return Status::Concluded;
	if (one_match_finished)
		return Status::Running;
	return Status::Scheduled;
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



bool MatchTable::AddParticipant(const Judoka* NewParticipant, bool Force)
{
	if (!NewParticipant || !m_Filter)
		return false;

	if (!Force)//Don't check if we are adding we are forcing the judoka in this match table
		if (!m_Filter->IsElgiable(*NewParticipant))//Is the judoka allowed in this match table?
			return false;

	//Is the judoka already a participant?
	for (auto judoka : m_Filter->GetParticipants())
		if (judoka && *judoka == *NewParticipant)
			return false;

	m_Filter->AddParticipant(NewParticipant, Force);
	GenerateSchedule();
	return true;
}



bool MatchTable::RemoveParticipant(const Judoka* Participant)
{
	if (!Participant || !m_Filter)
		return false;

	if (!m_Filter->RemoveParticipant(Participant))
		return false;

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
	if (!Judoka || !m_Filter)
		return false;

	for (auto participant : m_Filter->GetParticipants())
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

	//ZED::Log::Warn("Could not find rule set");

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



MatchTable::MatchTable(const YAML::Node& Yaml, const ITournament* Tournament) : m_Tournament(Tournament)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["schedule_index"])
		m_ScheduleIndex = Yaml["schedule_index"].as<int>();
	if (Yaml["mat_id"])
		m_MatID = Yaml["mat_id"].as<int>();
	if (Yaml["color"])
		m_Color = Yaml["color"].as<int>();

	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();

	if (Yaml["rule_set"] && Tournament)
		m_Rules = Tournament->FindRuleSet(Yaml["rule_set"].as<std::string>());

	if (Yaml["age_group"] && Tournament)
		SetAgeGroup(Tournament->FindAgeGroup(Yaml["age_group"].as<std::string>()));

	if (Yaml["participants"] && Yaml["participants"].IsSequence() && Tournament)
	{
		for (const auto& node : Yaml["participants"])
		{
			auto participant = Tournament->FindParticipant(node.as<std::string>());
			if (participant)
				AddParticipant(participant, true);
		}
	}

	if (Yaml["matches"] && Yaml["matches"].IsSequence())
	{
		for (const auto& node : Yaml["matches"])
			m_Schedule.emplace_back(new Match(node, this, Tournament));
	}
}



size_t MatchTable::GetStartingPosition(const Judoka* Judoka) const
{
	if (m_Filter)
		m_Filter->GetStartingPosition(Judoka);
}



void MatchTable::SetStartingPosition(const Judoka* Judoka, size_t NewStartingPosition)
{
	if (m_Filter)
		m_Filter->SetStartingPosition(Judoka, NewStartingPosition);
}



const AgeGroup* MatchTable::GetAgeGroup() const
{
	if (m_Filter)
		return m_Filter->GetAgeGroup();
}



void MatchTable::SetAgeGroup(const AgeGroup* NewAgeGroup)
{
	if (m_Filter)
		return m_Filter->SetAgeGroup(NewAgeGroup);
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
	if (GetAgeGroup())
		Yaml << YAML::Key << "age_group" << YAML::Value << (std::string)GetAgeGroup()->GetUUID();

	Yaml << YAML::Key << "participants";
	Yaml << YAML::BeginSeq;

	for (auto judoka : GetParticipants())
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
	if (GetAgeGroup())
		Yaml << YAML::Key << "age_group" << YAML::Value << (std::string)GetAgeGroup()->GetUUID();

	Yaml << YAML::Key << "participants";
	Yaml << YAML::BeginSeq;

	for (auto judoka : GetParticipants())
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

	Yaml << YAML::Key << "best_of_three" << YAML::Value << m_BestOfThree;

	Yaml << YAML::EndSeq;
}



std::string MatchTable::GetHTMLForm() const
{
	std::string ret = R"(
<div>
	<label style="width:150px;float:left;margin-top:5px;" id="label_bo3">Best Of Three</label>
		<input type="checkbox" id="bo3" class="switch-input">
		<label style="padding-top:0px;padding-bottom:0px;margin-top:5px;margin-bottom:20px;" class="switch-label" for="bo3">
		<span class="toggle-on" id="bo3_enabled"></span><span class="toggle-off" id="bo3_disabled"></span>
	</label>
</div>
)";

	return ret;
}



Match* MatchTable::AddAutoMatch(size_t WhiteStartingPosition, size_t BlueStartingPosition)
{
	if (!GetJudokaByStartingPosition(WhiteStartingPosition) || !GetJudokaByStartingPosition(BlueStartingPosition))
	{
		ZED::Log::Error("Illegal participant index");
		return nullptr;
	}

	return CreateAutoMatch(GetJudokaByStartingPosition(WhiteStartingPosition), GetJudokaByStartingPosition(BlueStartingPosition));
}



Match* MatchTable::CreateAutoMatch(const Judoka* White, const Judoka* Blue)
{
	Match* new_match = new Match(White, Blue, GetTournament(), GetMatID());
	new_match->SetAutoGenerated();
	new_match->SetMatchTable(this);
	m_Schedule.emplace_back(new_match);
	return new_match;
}



Match* MatchTable::AddMatchForWinners(Match* Match1, Match* Match2)
{
	Match* new_match = CreateAutoMatch(nullptr, nullptr);

	new_match->SetDependency(Fighter::White, Match::DependencyType::TakeWinner, Match1);
	new_match->SetDependency(Fighter::Blue,  Match::DependencyType::TakeWinner, Match2);

	return new_match;
}



void MatchTable::AddMatchesForBestOfThree()
{
	auto schedule_copy = std::move(m_Schedule);

	auto length = schedule_copy.size();
	for (size_t i = 0; i < length; ++i)
	{
		auto match1 = schedule_copy[i];

		auto match2 = new Match(*match1);
		match2->SwapFighters();
		auto match3 = new Match(*match1);
		match3->SetBestOfThree(match1, match2);

		m_Schedule.push_back(match1);
		m_Schedule.emplace_back(match2);
		m_Schedule.emplace_back(match3);
	}

	//Fix references, matches should take the winner of the BO3 match not the first one
	for (auto match : m_Schedule)
	{
		if (match->GetDependencyTypeOf(Fighter::White) != Match::DependencyType::TakeWinner)
			continue;
		if (match->GetDependencyTypeOf(Fighter::Blue ) != Match::DependencyType::TakeWinner)
			continue;
		if (match->IsBestOfThree())
			continue;

		auto dependent_match1 = match->GetDependentMatchOf(Fighter::White);
		auto dependent_match2 = match->GetDependentMatchOf(Fighter::Blue);

		if (dependent_match1 && dependent_match2)
		{
			auto index1 = FindMatchIndex(dependent_match1->GetUUID());
			auto index2 = FindMatchIndex(dependent_match2->GetUUID());

			if (index1 != SIZE_MAX && index2 != SIZE_MAX)
			{
				if (index1 + 2 < m_Schedule.size())
					match->SetDependency(Fighter::White, Match::DependencyType::TakeWinner, m_Schedule[index1 + 2]);
				if (index2 + 2 < m_Schedule.size())
					match->SetDependency(Fighter::Blue,  Match::DependencyType::TakeWinner, m_Schedule[index2 + 2]);
			}
		}
	}
}



const std::string MatchTable::ResultsToHTML() const
{
	auto results = CalculateResults();

	std::string ret;

	if (results.size() == 0)
		return ret;

	ret += "</table><br/><br/><table border=\"1\" rules=\"all\">";
	ret += "<tr><th style=\"width: 0.5cm; text-align: center;\">#</th><th style=\"width: 5.0cm;\">" + Localizer::Translate("Name")
		+ "</th><th style=\"width: 1.0cm;\">" + Localizer::Translate("Wins") + "</th><th style=\"width: 1.0cm;\">"
		+ Localizer::Translate("Score") + "</th><th style=\"width: 1.3cm;\">" + Localizer::Translate("Time") + "</th></tr>";

	for (size_t i = 0; i < results.size(); i++)
	{
		const auto& score = results[i];

		ret += "<tr><td style=\"text-align: center;\">" + std::to_string(i+1) + "</td>";
		ret += "<td>" + score.Judoka->GetName(NameStyle::GivenName) + "</td>";

		ret += "<td>" + std::to_string(score.Wins)  + "</td>";
		ret += "<td>" + std::to_string(score.Score) + "</td>";
		ret += "<td>" + Timer::TimestampToString(score.Time);

		if (score.NotSortable && score.Time > 0)
			ret += " Not sortable!";//TODO make proper message

		ret += "</td>";

		ret += "</tr>";
	}

	ret += "</table>";
	return ret;
}