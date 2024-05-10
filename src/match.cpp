#include <sstream>
#include <fstream>
#include "../ZED/include/csv.h"
#include "../ZED/include/log.h"
#include "match.h"
#include "tournament.h"



using namespace Judoboard;



RuleSet s_default_rules;



Match::Match(const Judoka* White, const Judoka* Blue, const ITournament* Tournament, uint32_t MatID) : m_Tournament(Tournament)
{
	m_White.m_Judoka = White;
	m_Blue.m_Judoka  = Blue;
	SetMatID(MatID);
}



Match::Match(const DependentJudoka& White, const DependentJudoka& Blue, const ITournament* Tournament, uint32_t MatID)
{
	m_White = White;
	m_Blue  = Blue;
	SetMatID(MatID);
}



Match::Match(const YAML::Node& Yaml, MatchTable* MatchTable, const ITournament* Tournament) : m_Tournament(Tournament)
{
	if (!Yaml.IsMap())
		return;

	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["mat_id"])
		m_MatID = Yaml["mat_id"].as<int>();

	if (Yaml["white"] && Tournament)
	{
		m_White.m_Judoka = Tournament->FindParticipant(Yaml["white"].as<std::string>());
		assert(m_White.m_Judoka);
	}
	if (Yaml["blue"] && Tournament)
	{
		m_Blue.m_Judoka  = Tournament->FindParticipant(Yaml["blue"].as<std::string>());
		assert(m_Blue.m_Judoka);
	}

	if (Yaml["state"])
		m_State = (Status)Yaml["state"].as<int>();
	if (Yaml["winner"])
		m_Result.m_Winner = (Winner)Yaml["winner"].as<int>();
	if (Yaml["score"])
		m_Result.m_Score = (Score)Yaml["score"].as<int>();
	if (Yaml["time"])
		m_Result.m_Time = Yaml["time"].as<int>();

	if (Yaml["rule_set"] && Tournament)
		m_Rules = Tournament->FindRuleSet(Yaml["rule_set"].as<std::string>());

	if (MatchTable)
		m_Table = MatchTable;
	else if (Yaml["match_table"] && Tournament)
		m_Table = Tournament->FindMatchTable(Yaml["match_table"].as<std::string>());

	if (Yaml["dependency_white"])
		m_White.m_Type = (DependencyType)Yaml["dependency_white"].as<int>();
	if (Yaml["dependency_blue"])
		m_Blue.m_Type = (DependencyType)Yaml["dependency_blue"].as<int>();

	if (Yaml["dependent_match_white"])
	{
		if (m_Table)
		{
			if (m_Table->GetParent())
				m_White.m_DependentMatch = m_Table->GetParent()->FindMatch(Yaml["dependent_match_white"].as<std::string>());
			if (!m_White.m_DependentMatch)
				m_White.m_DependentMatch = m_Table->FindMatch(Yaml["dependent_match_white"].as<std::string>());
		}

		if (!m_White.m_DependentMatch && Tournament)
			m_White.m_DependentMatch = Tournament->FindMatch(Yaml["dependent_match_white"].as<std::string>());
		assert(m_White.m_DependentMatch);
	}
	if (Yaml["dependent_match_blue"])
	{
		if (m_Table)
		{
			if (m_Table->GetParent())
				m_Blue.m_DependentMatch = m_Table->GetParent()->FindMatch(Yaml["dependent_match_blue"].as<std::string>());
			if (!m_Blue.m_DependentMatch)
				m_Blue.m_DependentMatch = m_Table->FindMatch(Yaml["dependent_match_blue"].as<std::string>());
		}

		if (!m_Blue.m_DependentMatch && Tournament)
			m_Blue.m_DependentMatch = Tournament->FindMatch(Yaml["dependent_match_blue"].as<std::string>());
		assert(m_Blue.m_DependentMatch);
	}

	if (Yaml["dependent_match_table_white"])
	{
		if (m_Table && m_Table->GetParent())
			m_White.m_DependentMatchTable = m_Table->GetParent()->FindMatchTable(Yaml["dependent_match_table_white"].as<std::string>());
		if (!m_White.m_DependentMatchTable && Tournament)
			m_White.m_DependentMatchTable = Tournament->FindMatchTable(Yaml["dependent_match_table_white"].as<std::string>());
		assert(m_White.m_DependentMatchTable);
	}
	if (Yaml["dependent_match_table_blue"])
	{
		if (m_Table && m_Table->GetParent())
			m_Blue.m_DependentMatchTable = m_Table->GetParent()->FindMatchTable(Yaml["dependent_match_table_blue"].as<std::string>());
		if (!m_Blue.m_DependentMatchTable && Tournament)
			m_Blue.m_DependentMatchTable = Tournament->FindMatchTable(Yaml["dependent_match_table_blue"].as<std::string>());
		assert(m_Blue.m_DependentMatchTable);
	}

	if (Yaml["tag"])
		m_Tag.value = Yaml["tag"].as<uint32_t>();

	if (Yaml["log"])
		m_Log << Yaml["log"];
}



void Match::operator >>(YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)GetUUID();
	if (m_MatID != 0)
		Yaml << YAML::Key << "mat_id" << YAML::Value << m_MatID;

	if (GetFighter(Fighter::White))
		Yaml << YAML::Key << "white" << YAML::Value << (std::string)GetFighter(Fighter::White)->GetUUID();
	if (GetFighter(Fighter::Blue))
		Yaml << YAML::Key << "blue"  << YAML::Value << (std::string)GetFighter(Fighter::Blue )->GetUUID();

	Yaml << YAML::Key << "state"  << YAML::Value << (int)m_State;

	if (HasConcluded())
	{
		Yaml << YAML::Key << "winner" << YAML::Value << (int)m_Result.m_Winner;
		Yaml << YAML::Key << "score" << YAML::Value << (int)m_Result.m_Score;
		Yaml << YAML::Key << "time" << YAML::Value << m_Result.m_Time;
	}

	if (m_Rules)
		Yaml << YAML::Key << "rule_set" << YAML::Value << (std::string)m_Rules->GetUUID();

	if (m_Table)
		Yaml << YAML::Key << "match_table" << YAML::Value << (std::string)m_Table->GetUUID();

	if (m_White.m_Type != DependencyType::None)
		Yaml << YAML::Key << "dependency_white" << YAML::Value << (int)m_White.m_Type;
	if (m_Blue.m_Type != DependencyType::None)
		Yaml << YAML::Key << "dependency_blue"  << YAML::Value << (int)m_Blue.m_Type;

	if (m_White.m_DependentMatch)
		Yaml << YAML::Key << "dependent_match_white" << YAML::Value << (std::string)m_White.m_DependentMatch->GetUUID();
	if (m_Blue.m_DependentMatch)
		Yaml << YAML::Key << "dependent_match_blue"  << YAML::Value << (std::string)m_Blue.m_DependentMatch->GetUUID();

	if (m_White.m_DependentMatchTable)
		Yaml << YAML::Key << "dependent_match_table_white" << YAML::Value << (std::string)m_White.m_DependentMatchTable->GetUUID();
	if (m_Blue.m_DependentMatchTable)
		Yaml << YAML::Key << "dependent_match_table_blue"  << YAML::Value << (std::string)m_Blue.m_DependentMatchTable->GetUUID();

	if (!m_Tag.IsNormal())
		Yaml << YAML::Key << "tag" << YAML::Value << (uint32_t)m_Tag.value;

	if (HasConcluded() || IsRunning())
	{
		Yaml << YAML::Key << "log" << YAML::Value;
		m_Log >> Yaml;
	}

	Yaml << YAML::EndMap;
}



void Match::operator >>(nlohmann::json& Json) const
{
	Json["uuid"] = (std::string)GetUUID();

	Json["mat_id"] = GetMatID();

	if (GetFighter(Fighter::White))
		Json["white"] = (std::string)GetFighter(Fighter::White)->GetName(NameStyle::GivenName);
	else
		Json["white"] = "???";
	if (GetFighter(Fighter::Blue))
		Json["blue"]  = (std::string)GetFighter(Fighter::Blue)->GetName(NameStyle::GivenName);
	else
		Json["blue"] = "???";

	Json["state"] = (int)m_State;

	if (HasConcluded())
	{
		Json["winner"] = (int)m_Result.m_Winner;
		Json["score"]  = (int)m_Result.m_Score;
		Json["time"]   = m_Result.m_Time;
	}
	else
	{
		Json["winner"] = 0;
		Json["score"]  = 0;
		Json["time"]   = 0;
	}

	if (m_Table)
		Json["match_table"] = m_Table->GetDescription();
	else
		Json["match_table"] = "- - -";

	if (!m_Tag.IsNormal())
		Json["tag"] = (uint32_t)m_Tag.value;
	else
		Json["tag"] = 0;

	if (IsRunning())
	{
		//Export current state
	}
}



void Match::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	Yaml << YAML::Key << "uuid"              << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "current_breaktime" << YAML::Value << GetCurrentBreaktime();
	Yaml << YAML::Key << "breaktime"         << YAML::Value << GetRuleSet().GetBreakTime();

	if (GetFighter(Fighter::White))
	{
		Yaml << YAML::Key << "white_name" << YAML::Value << GetFighter(Fighter::White)->GetName(NameStyle::GivenName);
		Yaml << YAML::Key << "white_uuid" << YAML::Value << (std::string)GetFighter(Fighter::White)->GetUUID();
	}
	else
	{
		Yaml << YAML::Key << "white_dependency_type" << YAML::Value << (int)m_White.m_Type;
		if (m_White.m_DependentMatch)
			Yaml << YAML::Key << "white_dependency_uuid" << YAML::Value << (std::string)m_White.m_DependentMatch->GetUUID();
		else if (m_White.m_DependentMatchTable)
			Yaml << YAML::Key << "white_dependency_uuid" << YAML::Value << (std::string)m_White.m_DependentMatchTable->GetUUID();
	}

	if (GetFighter(Fighter::Blue))
	{
		Yaml << YAML::Key << "blue_name" << YAML::Value << GetFighter(Fighter::Blue)->GetName(NameStyle::GivenName);
		Yaml << YAML::Key << "blue_uuid" << YAML::Value << (std::string)GetFighter(Fighter::Blue)->GetUUID();
	}
	else
	{
		Yaml << YAML::Key << "blue_dependency_type" << YAML::Value << (int)m_Blue.m_Type;
		if (m_Blue.m_DependentMatch)
			Yaml << YAML::Key << "blue_dependency_uuid" << YAML::Value << (std::string)m_Blue.m_DependentMatch->GetUUID();
		else if (m_Blue.m_DependentMatchTable)
			Yaml << YAML::Key << "blue_dependency_uuid" << YAML::Value << (std::string)m_Blue.m_DependentMatchTable->GetUUID();
	}

	Yaml << YAML::Key << "mat_id" << YAML::Value << GetMatID();
	Yaml << YAML::Key << "state"  << YAML::Value << (int)GetStatus();

	if (m_Rules)
		Yaml << YAML::Key << "rule_set" << YAML::Value << (std::string)m_Rules->GetUUID();

	if (GetMatchTable())
	{
		Yaml << YAML::Key << "match_table" << YAML::Value << (std::string)GetMatchTable()->GetUUID();
		Yaml << YAML::Key << "match_table_name" << YAML::Value << GetMatchTable()->GetDescription();
		Yaml << YAML::Key << "color" << YAML::Value << GetMatchTable()->GetColor().ToHexString();
	}

	Yaml << YAML::Key << "winner" << YAML::Value << (int)m_Result.m_Winner;
	Yaml << YAML::Key << "score"  << YAML::Value << (int)m_Result.m_Score;
	Yaml << YAML::Key << "time"   << YAML::Value << m_Result.m_Time;

	Yaml << YAML::EndMap;
}



Status Match::GetStatus() const
{
	if (IsBestOfThree())
	{
		if (HasUnresolvedDependency())
			return Status::Optional;

		//Check if won by the same person
		if (m_White.m_DependentMatch && m_Blue.m_DependentMatch)
		{
			if (m_White.m_DependentMatch->GetWinner() &&
				m_Blue.m_DependentMatch->GetWinner()  &&
				m_White.m_DependentMatch->GetWinner()->GetUUID() == m_Blue.m_DependentMatch->GetWinner()->GetUUID())
				return Status::Skipped;//Skip match
		}

		return m_State;
	}
	return m_State;
}



bool Match::HasConcluded() const
{
	if (IsEmptyMatch())
		return true;

	if (IsBestOfThree() && m_White.m_DependentMatch && m_Blue.m_DependentMatch)
	{
		if (m_White.m_DependentMatch->HasConcluded() && m_Blue.m_DependentMatch->HasConcluded())
		{
			if (!m_White.m_DependentMatch->GetWinner())
				return true;
			if (!m_Blue.m_DependentMatch->GetWinner())
				return true;
			if (*m_White.m_DependentMatch->GetWinner() == *m_Blue.m_DependentMatch->GetWinner())
				return true;
		}
	}

	return m_State == Status::Concluded;
}



uint32_t Match::GetMatID() const
{
	if (m_MatID <= 0 && m_Table)//No override specified and associated to a match table
		return m_Table->GetMatID();//Take the mat of the match table
	return m_MatID;
}



const Judoka* Match::GetFighter(Fighter Fighter) const
{
	if (Fighter == Fighter::White)
	{
		if (m_White.m_Judoka)
			return m_White.m_Judoka;

		else if (m_White.m_Type == DependencyType::BestOfThree && m_White.m_DependentMatch)
			return m_White.m_DependentMatch->GetFighter(Fighter::White);

		else if (m_White.m_Type == DependencyType::TakeWinner)
		{
			if (m_White.m_DependentMatch && m_White.m_DependentMatch->HasConcluded())
				return m_White.m_DependentMatch->GetWinner();
		}

		else if (m_White.m_Type == DependencyType::TakeLoser)
		{
			if (m_White.m_DependentMatch && m_White.m_DependentMatch->HasConcluded())
				return m_White.m_DependentMatch->GetLoser();
		}

		else if (m_White.m_DependentMatchTable)
		{
			if (!m_White.m_DependentMatchTable->HasConcluded())
				return nullptr;

			auto results = m_White.m_DependentMatchTable->CalculateResults();
			return results.Get(m_White.m_Type);
		}

		return nullptr;
	}


	if (m_Blue.m_Judoka)
		return m_Blue.m_Judoka;

	else if (m_Blue.m_Type == DependencyType::BestOfThree && m_Blue.m_DependentMatch)
		return m_Blue.m_DependentMatch->GetFighter(Fighter::Blue);

	else if (m_Blue.m_Type == DependencyType::TakeWinner)
	{
		if (m_Blue.m_DependentMatch && m_Blue.m_DependentMatch->HasConcluded())
			return m_Blue.m_DependentMatch->GetWinner();
	}

	else if (m_Blue.m_Type == DependencyType::TakeLoser)
	{
		if (m_Blue.m_DependentMatch && m_Blue.m_DependentMatch->HasConcluded())
			return m_Blue.m_DependentMatch->GetLoser();
	}

	else if (m_Blue.m_DependentMatchTable)
	{
		if (!m_Blue.m_DependentMatchTable->HasConcluded())
			return nullptr;

		auto results = m_Blue.m_DependentMatchTable->CalculateResults();
		return results.Get(m_Blue.m_Type);
	}

	return nullptr;
}



void Match::SwapFighters()
{
	std::swap(m_White, m_Blue);
	m_Log.SwapEvents();
}



std::vector<const Judoka*> Match::GetPotentialFighters() const
{
	std::vector<const Judoka*> ret;

	if (m_White.m_Judoka)
		ret.emplace_back(m_White.m_Judoka);

	else if (m_White.m_Type == DependencyType::TakeWinner || m_White.m_Type == DependencyType::TakeLoser)
	{
		if (m_White.m_DependentMatch)
		{
			auto list = m_White.m_DependentMatch->GetPotentialFighters();
			ret.insert(ret.end(), list.begin(), list.end());
		}
	}

	if (m_Blue.m_Judoka)
		ret.emplace_back(m_Blue.m_Judoka);

	else if (m_Blue.m_Type == DependencyType::TakeWinner || m_Blue.m_Type == DependencyType::TakeLoser)
	{
		if (m_Blue.m_DependentMatch)
		{
			auto list = m_Blue.m_DependentMatch->GetPotentialFighters();
			ret.insert(ret.end(), list.begin(), list.end());
		}
	}

	return ret;
}



std::vector<const Judoka*> Match::GetPotentialFighters(Fighter Fighter) const
{
	std::vector<const Judoka*> ret;

	if (Fighter == Fighter::White)
	{
		if (m_White.m_Judoka)
			ret.emplace_back(m_White.m_Judoka);

		else if (m_White.m_Type == DependencyType::TakeWinner || m_White.m_Type == DependencyType::TakeLoser)
		{
			if (m_White.m_DependentMatch)
			{
				auto list = m_White.m_DependentMatch->GetPotentialFighters();
				ret.insert(ret.end(), list.begin(), list.end());
			}
		}
	}

	else
	{
		if (m_Blue.m_Judoka)
			ret.emplace_back(m_White.m_Judoka);

		else if (m_Blue.m_Type == DependencyType::TakeWinner || m_Blue.m_Type == DependencyType::TakeLoser)
		{
			if (m_Blue.m_DependentMatch)
			{
				auto list = m_Blue.m_DependentMatch->GetPotentialFighters();
				ret.insert(ret.end(), list.begin(), list.end());
			}
		}
	}

	return ret;
}



bool Match::Contains(const Judoka& Judoka) const
{
	if (m_White.m_Judoka && m_White.m_Judoka->GetUUID() == Judoka.GetUUID())
		return true;

	if (m_Blue.m_Judoka  && m_Blue.m_Judoka->GetUUID()  == Judoka.GetUUID())
		return true;

	return false;
}



Fighter Match::GetColorOfFighter(const Judoka& Judoka) const
{
	if (m_White.m_Judoka && m_White.m_Judoka->GetUUID() == Judoka.GetUUID())
		return Fighter::White;
	return Fighter::Blue;
}



const Judoka* Match::GetEnemyOf(const Judoka& Judoka) const
{
	if (m_White.m_Judoka && m_White.m_Judoka->GetUUID() == Judoka.GetUUID())
		return m_Blue.m_Judoka;

	if (m_Blue.m_Judoka  && m_Blue.m_Judoka->GetUUID()  == Judoka.GetUUID())
		return m_White.m_Judoka;

	return nullptr;
}



const Judoka* Match::GetWinner() const
{
	if (IsCompletelyEmptyMatch())
		return nullptr;

	if (IsEmptyMatch())
	{
		if (GetFighter(Fighter::White))
			return GetFighter(Fighter::White);
		return GetFighter(Fighter::Blue);
	}

	if (!HasConcluded())
		return nullptr;

	if (IsBestOfThree() && m_White.m_DependentMatch && m_Blue.m_DependentMatch)
	{
		if (m_White.m_DependentMatch->HasConcluded() && m_Blue.m_DependentMatch->HasConcluded())
		{
			if (!m_White.m_DependentMatch->GetWinner())
				return nullptr;
			if (!m_Blue.m_DependentMatch->GetWinner())
				return nullptr;

			if (*m_White.m_DependentMatch->GetWinner() == *m_Blue.m_DependentMatch->GetWinner())
				return m_White.m_DependentMatch->GetWinner();
		}
	}

	auto result = GetResult();

	if (result.m_Winner == Winner::Draw)
		return nullptr;

	if (result.m_Winner == Winner::White)
		return GetFighter(Fighter::White);
	else
		return GetFighter(Fighter::Blue);
}



const Judoka* Match::GetLoser() const
{
	if (IsCompletelyEmptyMatch())
		return nullptr;

	if (IsEmptyMatch())
	{
		if (GetFighter(Fighter::White))
			return GetFighter(Fighter::Blue);
		return GetFighter(Fighter::White);
	}

	if (!HasConcluded())
		return nullptr;

	if (IsBestOfThree() && m_White.m_DependentMatch && m_Blue.m_DependentMatch)
	{
		if (m_White.m_DependentMatch->HasConcluded() && m_Blue.m_DependentMatch->HasConcluded())
		{
			if (!m_White.m_DependentMatch->GetLoser())
				return nullptr;
			if (!m_Blue.m_DependentMatch->GetLoser())
				return nullptr;

			if (*m_White.m_DependentMatch->GetLoser() == *m_Blue.m_DependentMatch->GetLoser())
				return m_White.m_DependentMatch->GetLoser();
		}
	}

	auto result = GetResult();

	if (result.m_Winner == Winner::Draw)
		return nullptr;

	if (result.m_Winner == Winner::White)
		return GetFighter(Fighter::Blue);
	else
		return GetFighter(Fighter::White);
}



void Match::SetDependency(Fighter Fighter, DependencyType Type, Match* Reference)
{
	if (Fighter == Fighter::White)
	{
		m_White.m_Type = Type;
		m_White.m_DependentMatch = Reference;
	}
	else
	{
		m_Blue.m_Type = Type;
		m_Blue.m_DependentMatch = Reference;
	}
}



void Match::SetBestOfThree(Match* Reference1, Match* Reference2)
{
	SetDependency(Fighter::White, DependencyType::BestOfThree, Reference1);
	SetDependency(Fighter::Blue,  DependencyType::BestOfThree, Reference2);
}



bool Match::HasUnresolvedDependency() const
{
	if (m_White.m_DependentMatch)
		if (!m_White.m_DependentMatch->HasConcluded())
			return true;

	if (m_Blue.m_DependentMatch)
		if (!m_Blue.m_DependentMatch->HasConcluded())
			return true;

	return false;
}



const std::vector<const Match*> Match::GetDependentMatches() const
{
	std::vector<const Match*> ret;

	if (m_White.m_DependentMatch)
		ret.emplace_back(m_White.m_DependentMatch);
	if (m_Blue.m_DependentMatch)
		ret.emplace_back(m_Blue.m_DependentMatch);

	return ret;
}



bool Match::IsEmptyMatch() const
{
	if (m_White.m_DependentMatchTable && m_Blue.m_DependentMatchTable)
		return false;
	if (m_White.m_DependentMatchTable)
		return !GetFighter(Fighter::Blue);
	if (m_Blue.m_DependentMatchTable)
		return !GetFighter(Fighter::White);

	/*if (m_White.m_DependentMatchTable && !m_White.m_DependentMatchTable->HasConcluded())
		return !GetFighter(Fighter::Blue);
	if (m_Blue.m_DependentMatchTable  && !m_Blue.m_DependentMatchTable->HasConcluded())
		return !GetFighter(Fighter::White);*/

	if (m_White.m_DependentMatch && m_White.m_DependentMatch->IsCompletelyEmptyMatch())
		return true;
	if (m_Blue.m_DependentMatch && m_Blue.m_DependentMatch->IsCompletelyEmptyMatch())
		return true;

	return !GetFighter(Fighter::White) || !GetFighter(Fighter::Blue);
}



bool Match::IsCompletelyEmptyMatch() const
{
	if (m_White.m_DependentMatchTable && !m_White.m_DependentMatchTable->HasConcluded())
		return false;
	if (m_Blue.m_DependentMatchTable  && !m_Blue.m_DependentMatchTable->HasConcluded())
		return false;

	if (m_White.m_DependentMatch && m_Blue.m_DependentMatch)
	{
		if (m_White.m_DependentMatch->IsCompletelyEmptyMatch() && m_Blue.m_DependentMatch->IsCompletelyEmptyMatch())
			return true;
	}

	return !GetFighter(Fighter::White) && !GetFighter(Fighter::Blue);
}



const RuleSet& Match::GetRuleSet() const
{
	if (m_Rules)
		return *m_Rules;
	if (m_Table)
		return m_Table->GetRuleSet();
	if (GetTournament() && GetTournament()->GetDefaultRuleSet())
		return *GetTournament()->GetDefaultRuleSet();

	//ZED::Log::Debug("Could not find rule set, using the default rule set");

	return s_default_rules;
}



uint32_t Match::GetCurrentBreaktime() const
{
	uint32_t breakW = 0, breakB = 0;
	if (GetFighter(Fighter::White))
		breakW = GetFighter(Fighter::White)->GetLengthOfBreak();
	if (GetFighter(Fighter::Blue))
		breakB = GetFighter(Fighter::Blue)->GetLengthOfBreak();

	if (breakW == 0)
		return breakB;
	if (breakB == 0)
		return breakW;

	return std::min(breakW, breakB);
}



void Match::StartMatch()
{
	m_State = Status::Running;
	if (GetTournament())
		GetTournament()->OnMatchStarted(*this);
}



void Match::EndMatch()
{
	m_State = Status::Concluded;

	const auto &rules = GetRuleSet();

	auto break_time = rules.GetBreakTime();
	if (rules.IsExtendBreakTime() && m_Result.m_Time/1000 > break_time)
		break_time = m_Result.m_Time / 1000;

	if (m_White.m_Judoka)
		m_White.m_Judoka->StartBreak(break_time);
	if (m_Blue.m_Judoka)
		m_Blue.m_Judoka->StartBreak(break_time);

	if (GetTournament())
		GetTournament()->OnMatchConcluded(*this);
}