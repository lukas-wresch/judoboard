#include "../ZED/include/log.h"
#include "matchtable.h"
#include "match.h"
#include "weightclass.h"
#include "fixed.h"
#include "tournament.h"
#include "round_robin.h"
#include "customtable.h"
#include "single_elimination.h"
#include "double_elimination.h"
#include "pool.h"



using namespace Judoboard;



MatchTable* MatchTable::CreateMatchTable(const YAML::Node& Yaml, const ITournament* Tournament)
{
	MatchTable* new_table = nullptr;

	if (!Yaml.IsMap() || !Yaml["type"])
		return new_table;

	switch ((MatchTable::Type)Yaml["type"].as<int>())
	{
	case MatchTable::Type::RoundRobin:
		new_table = new RoundRobin(Yaml, Tournament);
		break;
	case MatchTable::Type::Custom:
		new_table = new CustomTable(Yaml, Tournament);
		break;
	case MatchTable::Type::SingleElimination:
		new_table = new SingleElimination(Yaml, Tournament);
		break;
	case MatchTable::Type::Pool:
		new_table = new Pool(Yaml, Tournament);
		break;
	case MatchTable::Type::DoubleElimination:
		new_table = new DoubleElimination(Yaml, Tournament);
		break;
	}

	return new_table;
}



void MatchTable::SetMatID(int32_t MatID)
{
	m_MatID = MatID;

	auto schedule = GetSchedule();
	for (auto match : schedule)
		if (match)
			match->SetMatID(MatID);
}



bool MatchTable::HasConcluded() const
{
	auto schedule = GetSchedule();
	for (auto match : schedule)
		if (match && !match->HasConcluded())
			return false;
	return true;
}



Match* MatchTable::FindMatch(const UUID& UUID) const
{
	auto schedule = GetSchedule();
	for (auto match : schedule)
		if (match && match->GetUUID() == UUID)
			return match;
	return nullptr;
}



size_t MatchTable::FindMatchIndex(const UUID& UUID) const
{
	auto schedule = GetSchedule();
	for (size_t i = 0; i < schedule.size(); ++i)
		if (schedule[i] && schedule[i]->GetUUID() == UUID)
			return i;
	return -1;
}



const Judoka* MatchTable::FindParticipant(const UUID& UUID) const
{
	if (!m_Filter)
		return nullptr;

	for (auto [pos, judoka] : m_Filter->GetParticipants())
		if (judoka.GetJudoka() && *judoka.GetJudoka() == UUID)
			return judoka.GetJudoka();

	return nullptr;
}



bool MatchTable::IsElgiable(const Judoka& Fighter) const
{
	if (m_Filter)
		return m_Filter->IsElgiable(Fighter);
	return false;
}



const std::vector<const Judoka*> MatchTable::GetParticipants() const
{
	std::vector<const Judoka*> ret;

	if (m_Filter)
	{
		for (auto [pos, judoka] : m_Filter->GetParticipants())
			if (judoka.GetJudoka())
				ret.emplace_back(judoka.GetJudoka());
	}

	return ret;
}



bool MatchTable::IsIncluded(const Judoka& Fighter) const
{
	return FindParticipant(Fighter.GetUUID()) != nullptr;
}



Status MatchTable::GetStatus() const
{
	auto schedule = GetSchedule();

	if (schedule.size() == 0)
		return Status::Scheduled;

	bool one_match_finished = false;
	bool all_matches_finished = true;

	for (auto match : schedule)
	{
		if (match->IsEmptyMatch() || match->IsBestOfThree())
			continue;

		if (match->IsRunning())
			return Status::Running;

		if (match->HasConcluded())
			one_match_finished = true;
		else
			all_matches_finished = false;
	}

	if (all_matches_finished && one_match_finished)
		return Status::Concluded;

	return Status::Scheduled;
}



bool MatchTable::AddMatch(Match* NewMatch)
{
	if (!NewMatch)
		return false;

	if (NewMatch->GetFighter(Fighter::White))
		AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::White)), true);
	if (NewMatch->GetFighter(Fighter::Blue))
		AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::Blue)),  true);

	if (IsSubMatchTable())
		NewMatch->SetMatchTable(GetParent());
	else
		NewMatch->SetMatchTable(this);

	m_Schedule.emplace_back(NewMatch);
	return true;
}



bool MatchTable::AddParticipant(Judoka* NewParticipant, bool Force)
{
	if (!NewParticipant || !m_Filter)
		return false;

	if (FindParticipant(*NewParticipant))
		return false;

	if (!m_Filter->AddParticipant(NewParticipant, Force))
		return false;

	if (GetTournament())//Add to tournament?
		((ITournament*)GetTournament())->AddParticipant(NewParticipant);//Const cast

	if (!IsSubMatchTable())
		GenerateSchedule();
	return true;
}



bool MatchTable::RemoveParticipant(const Judoka* Participant)
{
	if (!Participant || !m_Filter)
		return false;

	auto old_max = GetMaxStartPositions();

	if (m_Filter->RemoveParticipant(Participant))
	{
		auto new_max = GetMaxStartPositions();

		if (new_max < old_max)
		{
			for (size_t i = new_max; i < old_max; ++i)
			{
				if (m_Filter->IsStartPositionTaken(i))
				{
					auto judoka = m_Filter->GetJudokaByStartPosition(i);
					m_Filter->FindFreeStartPos(judoka);
				}
			}
		}

		GenerateSchedule();
		return true;
	}

	return false;
}



bool MatchTable::DeleteMatch(const UUID& UUID)
{
	for (auto it = m_Schedule.begin(); it != m_Schedule.end(); ++it)
	{
		if ((*it)->GetUUID() == UUID)
		{
			if (!IsSubMatchTable())
				delete *it;
			m_Schedule.erase(it);
			return true;
		}
	}

	return false;
}



std::string MatchTable::GetDescription() const
{
	auto name = GetName();

	if (name.length() > 0 && GetAgeGroup())
		return GetAgeGroup()->GetName() + " " + name;

	else if (m_Filter)
	{
		if (name.length() > 0)
			return m_Filter->GetDescription() + " " + name;
		else
			return m_Filter->GetDescription();
	}

	return name;
}



const std::vector<const Match*> MatchTable::FindMatches(const Judoka& Fighter1, const Judoka& Fighter2) const
{
	std::vector<const Match*> ret;

	auto schedule = GetSchedule();

	for (auto match : schedule)
	{
		if (!match->HasValidFighters())
			continue;

		if ( (*match->GetFighter(Fighter::White) == Fighter1  && *match->GetFighter(Fighter::Blue)  == Fighter2) ||
			 (*match->GetFighter(Fighter::Blue)  == Fighter1  && *match->GetFighter(Fighter::White) == Fighter2) )
			ret.push_back(match);
	}

	return ret;
}



const RuleSet& MatchTable::GetRuleSet() const
{
	if (m_Rules)
		return *m_Rules;
	if (GetFilter() && GetFilter()->GetAgeGroup() && GetFilter()->GetAgeGroup()->GetRuleSet())
		return *GetFilter()->GetAgeGroup()->GetRuleSet();
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



MatchTable::MatchTable(const YAML::Node& Yaml, const ITournament* Tournament, const MatchTable* Parent)
	: m_Tournament(Tournament), m_Parent(Parent)
{
	assert(Yaml.IsMap());

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

	if (Yaml["matches"] && Yaml["matches"].IsSequence())
	{
		for (const auto& node : Yaml["matches"])
			m_Schedule.emplace_back(new Match(node, this, Tournament));
	}

	if (Yaml["filter"] && Yaml["filter"].IsMap())
	{
		switch ((IFilter::Type)Yaml["filter"]["type"].as<int>())
		{
			case IFilter::Type::Weightclass:
				SetFilter(new Weightclass(Yaml["filter"], m_Parent ? m_Parent : this));
				break;
			case IFilter::Type::Fixed:
			case IFilter::Type::Standard://Deprecated
				SetFilter(new Fixed(Yaml["filter"], m_Parent ? m_Parent : this));
				break;
			default:
				ZED::Log::Error("Unknown filter in match table");
				assert(false);
		}

		assert(GetFilter());
	}
}



size_t MatchTable::GetStartPosition(const Judoka* Judoka) const
{
	if (m_Filter)
		return m_Filter->GetStartPosition(Judoka);
	return SIZE_MAX;
}



const Judoka* MatchTable::GetJudokaByStartPosition(size_t StartPosition) const
{
	if (m_Filter)
		return m_Filter->GetJudokaByStartPosition(StartPosition).GetJudoka();
	return nullptr;
}



void MatchTable::SetStartPosition(const Judoka* Judoka, size_t NewStartPosition)
{
	if (NewStartPosition >= GetMaxStartPositions())
		return;

	if (m_Filter)
	{
		m_Filter->SetStartPosition(Judoka, NewStartPosition);
		GenerateSchedule();//Re-generate schedule
	}
}



const AgeGroup* MatchTable::GetAgeGroup() const
{
	if (m_Filter)
		return m_Filter->GetAgeGroup();
	return nullptr;
}



void MatchTable::SetAgeGroup(const AgeGroup* NewAgeGroup)
{
	if (m_Filter)
		m_Filter->SetAgeGroup(NewAgeGroup);
}



void MatchTable::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)GetUUID();

	if (!IsSubMatchTable())
	{
		Yaml << YAML::Key << "schedule_index" << YAML::Value << m_ScheduleIndex;
		Yaml << YAML::Key << "color"  << YAML::Value << (int)m_Color;
	}

	if (!IsSubMatchTable() || GetMatID() != 0)
		Yaml << YAML::Key << "mat_id" << YAML::Value << m_MatID;	

	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();

	if (m_Name.length() > 0)
		Yaml << YAML::Key << "name" << YAML::Value << m_Name;

	if (m_Rules)
		Yaml << YAML::Key << "rule_set" << YAML::Value << (std::string)m_Rules->GetUUID();

	if (GetFilter())
	{
		Yaml << YAML::Key << "filter" << YAML::Value;
		if (GetFilter()->GetType() == IFilter::Type::Weightclass)
			*GetFilter() >> Yaml;
		else//Export as fixed participants
		{//TODO this should be improved for double elimination filter is losersof
			Fixed temp(*GetFilter());
			temp >> Yaml;
		}
	}

	auto schedule = GetSchedule();

	if (!schedule.empty())
	{
		Yaml << YAML::Key << "matches";
		Yaml << YAML::BeginSeq;

		for (auto match : schedule)
			*match >> Yaml;

		Yaml << YAML::EndSeq;
	}
}



void MatchTable::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "schedule_index" << YAML::Value << m_ScheduleIndex;
	Yaml << YAML::Key << "mat_id" << YAML::Value << m_MatID;
	Yaml << YAML::Key << "color"  << YAML::Value << (int)m_Color;

	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();
	if (GetFilter())
		GetFilter()->ToString(Yaml);

	Yaml << YAML::Key << "name" << YAML::Value << m_Name;
	Yaml << YAML::Key << "description" << YAML::Value << GetDescription();

	Yaml << YAML::Key << "best_of_three" << YAML::Value << IsBestOfThree();

	if (IsSubMatchTable())
		Yaml << YAML::Key << "is_sub_match_table" << YAML::Value << IsSubMatchTable();

	if (m_Rules)
		Yaml << YAML::Key << "rule_set" << YAML::Value << (std::string)m_Rules->GetUUID();

	auto age_group = GetAgeGroup();
	if (age_group)
		Yaml << YAML::Key << "age_group" << YAML::Value << (std::string)age_group->GetUUID();

	Yaml << YAML::Key << "participants";
	Yaml << YAML::BeginSeq;

	auto max = GetMaxStartPositions();
	for (size_t start_pos = 0; start_pos <= max; ++start_pos)
	{
		auto judoka = GetJudokaByStartPosition(start_pos);
		
		if (judoka)
		{
			Yaml << YAML::BeginMap;
			Yaml << YAML::Key << "uuid"      << YAML::Value << (std::string)judoka->GetUUID();
			Yaml << YAML::Key << "firstname" << YAML::Value << judoka->GetFirstname();
			Yaml << YAML::Key << "lastname"  << YAML::Value << judoka->GetLastname();
			Yaml << YAML::Key << "weight"    << YAML::Value << judoka->GetWeight().ToString();

			if (judoka->GetClub())
			{
				Yaml << YAML::Key << "club_uuid" << YAML::Value << (std::string)judoka->GetClub()->GetUUID();
				Yaml << YAML::Key << "club_name" << YAML::Value << judoka->GetClub()->GetName();
			}

			Yaml << YAML::Key << "start_pos" << YAML::Value << start_pos;
			Yaml << YAML::EndMap;
		}
	}

	Yaml << YAML::EndSeq;
}



void MatchTable::operator >>(nlohmann::json& Json) const
{
	Json["uuid"] = (std::string)GetUUID();
	Json["mat_id"] = m_MatID;
	Json["html"] = ToHTML();	
}



void MatchTable::OnLotteryPerformed()
{
	if (!m_Filter || !GetTournament())
		return;

	auto participants = m_Filter->GetParticipants();
	auto lots = GetTournament()->GetLots();

	size_t current_pos = 0;

	for (auto [club_uuid, lot] : lots)//For every lot
		for (auto [pos, participant] : participants)//Find participants for this lot
	{
		auto judoka = participant.GetJudoka();

		if (!judoka)
			continue;

		auto club = judoka->GetClub();

		if (!club)
			continue;

		if (club->IsChildOf(club_uuid))//Does the club belong to this lot?
			m_Filter->SetStartPosition(participant, current_pos++);//Assign start position
	}
}



std::string MatchTable::GetHTMLForm()
{
	std::string ret = R"(
<div>
	<label style="width:150px;float:left;margin-top:5px;" id="label_bo3">Best of Three</label>
		<input type="checkbox" id="bo3" class="switch-input">
		<label style="padding-top:0px;padding-bottom:0px;margin-top:5px;margin-bottom:20px;" class="switch-label" for="bo3">
		<span class="toggle-on" id="bo3_enabled"></span><span class="toggle-off" id="bo3_disabled"></span>
	</label>
</div>
)";

	return ret;
}



const std::string MatchTable::GetHTMLTop() const
{
	std::string ret;

	if (!IsSubMatchTable() && GetFilter())
		ret += "<div style=\"border-style: dashed; border-width: 2px; border-color: #ccc; padding: 0.3cm; margin-bottom: 0.3cm;\">";

	if (!IsSubMatchTable())
		ret += "<h3>";

	ret += "<a href=\"#matchtable_add.html?id=" + (std::string)GetUUID() + "\">" + GetDescription() + "</a>";
	if (GetMatID() != 0)
		ret += " / " + Localizer::Translate("Mat") + " " + std::to_string(GetMatID()) + " / " + GetRuleSet().GetName();

	if (!IsSubMatchTable())
		ret += "</h3>";

	ret += "<br/><br/>";
	return ret;
}



Match* MatchTable::AddAutoMatch(size_t WhiteStartPosition, size_t BlueStartPosition)
{
	if (!GetJudokaByStartPosition(WhiteStartPosition) || !GetJudokaByStartPosition(BlueStartPosition))
	{
		ZED::Log::Error("Illegal participant index");
		return nullptr;
	}

	return CreateAutoMatch(GetJudokaByStartPosition(WhiteStartPosition), GetJudokaByStartPosition(BlueStartPosition));
}



Match* MatchTable::CreateAutoMatch(const DependentJudoka& White, const DependentJudoka& Blue)
{
	Match* new_match = new Match(White, Blue, GetTournament(), GetMatID());
	new_match->SetMatchTable(this);
	m_Schedule.emplace_back(new_match);
	return new_match;
}



Match* MatchTable::AddMatchForWinners(Match* Match1, Match* Match2)
{
	Match* new_match = CreateAutoMatch(nullptr, nullptr);

	if (!new_match)
		return nullptr;

	new_match->SetDependency(Fighter::White, DependencyType::TakeWinner, Match1);
	new_match->SetDependency(Fighter::Blue,  DependencyType::TakeWinner, Match2);

	return new_match;
}



void MatchTable::AddMatchesForBestOfThree()
{
	auto schedule_copy = GetSchedule();

	m_Schedule.clear();

	auto length = schedule_copy.size();
	for (size_t i = 0; i < length; ++i)
	{
		auto match1 = schedule_copy[i];

		auto match2 = new Match(*match1);
		match2->SwapFighters();
		auto match3 = new Match(*match1);
		match3->SetBestOfThree(match1, match2);

		m_Schedule.emplace_back(match1);
		m_Schedule.emplace_back(match2);
		m_Schedule.emplace_back(match3);
	}

	//Fix references, matches should take the winner of the BO3 match not the first one
	for (auto match : m_Schedule)
	{
		if (match->GetDependencyTypeOf(Fighter::White) != DependencyType::TakeWinner)
			continue;
		if (match->GetDependencyTypeOf(Fighter::Blue ) != DependencyType::TakeWinner)
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
					match->SetDependency(Fighter::White, DependencyType::TakeWinner, m_Schedule[index1 + 2]);
				if (index2 + 2 < m_Schedule.size())
					match->SetDependency(Fighter::Blue,  DependencyType::TakeWinner, m_Schedule[index2 + 2]);
			}
		}
	}
}



const std::string MatchTable::ResultsToHTML() const
{
	auto results = CalculateResults();

	std::string ret;

	if (results.GetSize() == 0)
		return ret;

	ret += "<br/><br/><table border=\"1\" rules=\"all\">";
	ret += "<tr><th style=\"width: 0.5cm; text-align: center;\">#</th><th style=\"width: 5.0cm;\">" + Localizer::Translate("Name")
		+ "</th><th style=\"width: 1.0cm;\">" + Localizer::Translate("Wins") + "</th><th style=\"width: 1.0cm;\">"
		+ Localizer::Translate("Score") + "</th><th style=\"width: 1.3cm;\">" + Localizer::Translate("Time") + "</th></tr>";

	for (size_t i = 0; i < results.GetSize(); i++)
	{
		const auto& score = results[i];

		ret += "<tr><td style=\"text-align: center;\">" + std::to_string(i+1) + "</td>";
		if (score.Judoka)
			ret += "<td>" + score.Judoka->GetName(NameStyle::GivenName) + "</td>";
		else
			ret += "<td>- - -</td>";

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