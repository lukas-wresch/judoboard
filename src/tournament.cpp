#include <fstream>
#include <algorithm>
#include <cassert>
#include "../ZED/include/log.h"
#include "tournament.h"
#include "database.h"
#include "weightclass.h"
#include "md5.h"
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"



using namespace Judoboard;



Tournament::Tournament(const std::string& Name) : m_Name(Name)
{
	LoadYAML("tournaments/" + Name + ".yml");
	//if (!Load("tournaments/" + Name))
		//m_pDefaultRules = new RuleSet();
}



Tournament::Tournament(const std::string& Name, const RuleSet* RuleSet) : m_pDefaultRules(RuleSet), m_Name(Name)
{
	LoadYAML("tournaments/" + Name + ".yml");
}



Tournament::Tournament(const MD5& File, Database* pDatabase)
{
	m_Name = File.GetDescription();

	//Add clubs
	for (auto club : File.GetClubs())
	{
		if (pDatabase)
		{
			Club* new_club = pDatabase->AddClub(*club);
			m_StandingData.AddClub(new_club);
		}

		else
			m_StandingData.AddClub(new Club(*club));
	}

	//Add weightclasses
	for (auto weightclass : File.GetWeightclasses())
	{
		auto new_weightclass = new Weightclass(*weightclass, this);
		weightclass->pUserData = new_weightclass;
		m_MatchTables.emplace_back(new_weightclass);
	}

	//Add judoka
	for (auto judoka : File.GetParticipants())
	{
		Judoka* new_judoka = nullptr;

		if (pDatabase)
			new_judoka = pDatabase->UpdateOrAdd(*judoka);
		else
			new_judoka = new Judoka(*judoka);

		if (new_judoka)
		{
			judoka->pUserData = new_judoka;
			m_StandingData.AddJudoka(new_judoka);

			if (judoka->Weightclass)
			{
				auto match_table = (Weightclass*)judoka->Weightclass->pUserData;
				if (match_table)
					match_table->AddParticipant(new_judoka, true);//Add with force
			}
		}
	}

	//Add matches
	for (auto& match : File.GetMatches())
	{
		if (match.WhiteID == match.RedID)//Filter dummy matches
			continue;

		if (!match.White || !match.Red)
			continue;
		if (!match.White->pUserData || !match.Red->pUserData)
			continue;

		Match* new_match = new Match(this, (Judoka*)match.White->pUserData, (Judoka*)match.Red->pUserData);

		if (match.WinnerID == match.WhiteID)
			new_match->SetResult(Match::Result(Fighter::White, (Match::Score)match.ScoreWinner, match.Time));
		else
			new_match->SetResult(Match::Result(Fighter::Blue,  (Match::Score)match.ScoreWinner, match.Time));

		if (match.Weightclass && match.Weightclass->pUserData)
		{
			auto match_table = (Weightclass*)match.Weightclass->pUserData;
			match_table->AddMatch(new_match);//Add match to weightclass
		}

		m_Schedule.emplace_back(new_match);
	}

	Save();
}



Tournament::~Tournament()
{
	if (m_Schedule.size() > 0 || GetParticipants().size() > 0 || m_MatchTables.size() > 0 || m_SchedulePlanner.size() > 0)
		Save();

	for (auto table : m_MatchTables)
		delete table;

	//for (auto match : m_Schedule)
		//if (!match->GetMatchTable())
			//delete match;//Match might now be stored on the heap, then this will crash
}



void Tournament::Reset()
{
	//Clear but don't delete standing data since this could be shared with a database
	m_StandingData.GetAllJudokas().clear();
	m_StandingData.GetRuleSets().clear();

	for (auto table : m_MatchTables)
		delete table;
	m_MatchTables.clear();

	for (auto match : m_Schedule)
		if (!match->GetMatchTable())
			delete match;
	m_Schedule.clear();

	m_SchedulePlanner.clear();

	//Don't delete since this could be shared memory
	//m_pDefaultRules = nullptr;

	assert(m_StandingData.GetNumJudoka() == 0);
}



bool Tournament::Load(const std::string& Filename)
{
	std::ifstream File(Filename, std::ios::binary);

	if (!File)
	{
		ZED::Log::Warn("Could not open file " + Filename);
		return false;
	}

	ZED::CSV csv(File);

	std::string name;
	csv >> name;

	if (name != m_Name)
	{
		ZED::Log::Warn("Invalid tournament name for file " + Filename + ". Tournament name is: " + name + " vs. " + m_Name);
		return false;
	}

	int version;
	csv >> version;

	if (version != 1)
	{
		ZED::Log::Warn("File format is too new for this application to read. Please update this application");
		return false;
	}

	m_StandingData << csv;
	ZED::Log::Info("Number of participants: " + std::to_string(m_StandingData.GetNumJudoka()));

	int disqualified_judoka;
	csv >> disqualified_judoka;
	ZED::Log::Info("Number of disqualified participants: " + std::to_string(disqualified_judoka));
	
	for (int i = 0; i < disqualified_judoka; i++)
	{
		std::string uuid;
		csv >> uuid;
		m_DisqualifiedJudoka.emplace(UUID(std::move(uuid)));
	}

	m_pDefaultRules = new RuleSet(csv);


	uint32_t MatchTableSize;
	csv >> MatchTableSize;
	ZED::Log::Info("Number of match tables: " + std::to_string(MatchTableSize));

	for (uint32_t i = 0; i < MatchTableSize; i++)
	{
		MatchTable::Type type;
		csv >> type;

		MatchTable* new_table = nullptr;

		switch (type)
		{
			case MatchTable::Type::Weightclass:
				new_table = new Weightclass(csv, this);
				break;
		}

		if (new_table)
		{
			//AddMatchTable(new_table, false);//TODO: Do this in a separate method
			m_MatchTables.push_back(new_table);
			m_SchedulePlanner.push_back(new_table);
		}
	}

	m_Schedule.clear();

	int ScheduleSize;
	csv >> ScheduleSize;

	for (int i = 0; i < ScheduleSize; i++)
	{
		Match* new_match = new Match(csv, this);

		if (new_match->GetMatchTable())
		{
			//TODO find a workaround
			auto id = new_match->GetMatchTable()->GetUUID();
			FindMatchTable(id)->SetSchedule().emplace_back(new_match);
		}

		m_Schedule.emplace_back(new_match);
	}

	ZED::Log::Info("Tournament " + Filename + " loaded successfully");
	return true;
}



bool Tournament::LoadYAML(const std::string& Filename)
{
	std::ifstream file(Filename);
	if (!file)
	{
		ZED::Log::Warn("Could not open file " + Filename);
		return false;
	}

	YAML::Node yaml = YAML::LoadFile(Filename);

	if (!yaml)
	{
		ZED::Log::Warn("Could not open file " + Filename);
		return false;
	}

	if (!yaml["version"] || !yaml["name"])
	{
		ZED::Log::Warn("File " + Filename + " is not a valid tournament file");
		return false;
	}

	if (yaml["version"].as<int>() != 1)
	{
		ZED::Log::Warn("Tournament file has been written with a new version. Please update this software!");
		return false;
	}

	m_Name = yaml["name"].as<std::string>();

	//Read standing data
	m_StandingData << yaml;

	if (yaml["disqualified_judoka"] && yaml["disqualified_judoka"].IsSequence())
	{
		for (const auto& node : yaml["disqualified_judoka"])
			m_DisqualifiedJudoka.insert(node.as<std::string>());
	}

	if (yaml["default_rule_set"])
		m_pDefaultRules = m_StandingData.FindRuleSet(yaml["default_rule_set"].as<std::string>());
	else
		m_pDefaultRules = nullptr;

	if (yaml["match_tables"] && yaml["match_tables"].IsSequence())
	{
		for (const auto& node : yaml["match_tables"])
		{
			if (!node["type"])
			{
				ZED::Log::Warn("match table has no attribute 'type'. Can not load this match table!");
				continue;
			}

			MatchTable* new_table = nullptr;

			switch ((MatchTable::Type)node["type"].as<int>())
			{
			case MatchTable::Type::Weightclass:
				new_table = new Weightclass(node, this);
				break;
			}

			if (new_table)
			{
				m_MatchTables.push_back(new_table);
				m_SchedulePlanner.push_back(new_table);
			}
		}
	}

	m_Schedule.clear();

	if (yaml["schedule"] && yaml["schedule"].IsSequence())
	{
		for (const auto& node : yaml["schedule"])
		{
			Match* new_match = new Match(node, this);

			if (new_match->GetMatchTable())
			{
				//TODO find a workaround
				auto id = new_match->GetMatchTable()->GetUUID();
				FindMatchTable(id)->SetSchedule().emplace_back(new_match);
			}

			m_Schedule.emplace_back(new_match);
		}
	}

	ZED::Log::Info("Tournament " + Filename + " loaded successfully");
	return true;
}



bool Tournament::Save(const std::string& Filename) const
{
	std::ofstream file(Filename, std::ios::binary);

	if (!file)
		return false;

	ZED::CSV stream;

	stream << m_Name;
	stream << 1;//Version

	m_StandingData >> stream;

	stream << m_DisqualifiedJudoka.size();
	for (const auto& judoka_uuid : m_DisqualifiedJudoka)
		stream << (std::string)judoka_uuid;

	if (m_pDefaultRules)
		*m_pDefaultRules >> stream;
	else
	{
		RuleSet temp;
		temp >> stream;
	}

	stream << m_MatchTables.size();

	for (auto table : m_MatchTables)
		*table >> stream;

	stream << m_Schedule.size();

	for (auto match : m_Schedule)
		*match >> stream;

	stream >> file;
	ZED::Log::Info("Tournament " + Filename + " saved successfully");
	return true;
}



bool Tournament::SaveYAML(const std::string& Filename) const
{
	std::ofstream file(Filename);

	if (!file)
		return false;

	YAML::Emitter yaml;

	yaml << YAML::BeginMap;
	yaml << YAML::Key << "name";
	yaml << YAML::Value << m_Name;
	yaml << YAML::Key << "version";
	yaml << YAML::Value << "1";

	m_StandingData >> yaml;

	yaml << YAML::Key << "disqualified_judoka";
	yaml << YAML::Value;
	yaml << YAML::BeginSeq;

	for (const auto& judoka_uuid : m_DisqualifiedJudoka)
		yaml << (std::string)judoka_uuid;

	yaml << YAML::EndSeq;

	if (m_pDefaultRules)
	{
		yaml << YAML::Key << "default_rule_set";
		yaml << YAML::Value << (std::string)m_pDefaultRules->GetUUID();
	}

	yaml << YAML::Key << "match_tables";
	yaml << YAML::Value;
	yaml << YAML::BeginSeq;

	for (auto table : m_MatchTables)
		*table >> yaml;

	yaml << YAML::EndSeq;

	yaml << YAML::Key << "schedule";
	yaml << YAML::Value;
	yaml << YAML::BeginSeq;

	for (auto match : m_Schedule)
		*match >> yaml;

	yaml << YAML::EndSeq;

	yaml << YAML::EndMap;
	file << yaml.c_str();

	ZED::Log::Info("Tournament " + Filename + " saved successfully");
	return true;
}



Status Tournament::GetStatus() const
{
	if (m_Schedule.size() == 0)
		return Status::Scheduled;

	bool one_match_finished   = false;
	bool all_matches_finished = true;

	for (auto match : m_Schedule)
	{
		if (!match->HasConcluded())
			all_matches_finished = false;
			
		if (match->IsRunning() || match->HasConcluded())
			one_match_finished = true;
	}

	if (all_matches_finished && !m_Name.empty())//Temporary tournaments can never conclude
		return Status::Concluded;
	if (one_match_finished)
		return Status::Running;
	return Status::Scheduled;
}



void Tournament::ConnectToDatabase(Database& db)
{
	if (GetStatus() == Status::Concluded)
		return;

	std::vector<Judoka*> judoka_to_add;

	for (auto it = m_StandingData.GetAllJudokas().begin(); it != m_StandingData.GetAllJudokas().end();)
	{
		auto db_ref = db.FindJudoka(it->second->GetUUID());
		if (db_ref && it->second != db_ref)//Pointing to different objects
		{
			delete it->second;
			it = m_StandingData.GetAllJudokas().erase(it);
			judoka_to_add.push_back(db_ref);
		}
		else
			++it;
	}

	for (auto judoka : judoka_to_add)
		m_StandingData.AddJudoka(judoka);


	//Do the same for rule sets

	if (m_pDefaultRules)
	{
		auto db_ref = db.FindRuleSet(m_pDefaultRules->GetUUID());
		if (db_ref)
			m_pDefaultRules = db_ref;
	}

	std::vector<RuleSet*> rule_sets_to_add;

	for (auto it = m_StandingData.GetRuleSets().begin(); it != m_StandingData.GetRuleSets().end();)
	{
		auto db_ref = db.FindRuleSet((*it)->GetUUID());
		if (db_ref)
		{
			delete *it;
			it = m_StandingData.GetRuleSets().erase(it);
			rule_sets_to_add.push_back(db_ref);
		}
		else
			++it;
	}

	for (auto rule : rule_sets_to_add)
		m_StandingData.AddRuleSet(rule);
}



bool Tournament::CanCloseTournament() const
{
	for (auto match : m_Schedule)
		if (match && match->IsRunning())
			return false;

	return true;
}



void Tournament::DeleteAllMatchResults()
{
	for (auto match : m_Schedule)
		if (match)
	{
		match->m_State = Status::Scheduled;
		match->GetLog().SetEvents().clear();
	}
}



bool Tournament::AddMatch(Match* NewMatch)
{
	if (!NewMatch)
	{
		ZED::Log::Error("Invalid match");
		return false;
	}
	if (GetStatus() == Status::Concluded)
	{
		ZED::Log::Warn("Can not add a match to a tournament which is finalized");
		return false;
	}

	if (NewMatch->GetFighter(Fighter::White) && NewMatch->GetFighter(Fighter::Blue) &&
		NewMatch->GetFighter(Fighter::White)->GetUUID() == NewMatch->GetFighter(Fighter::Blue)->GetUUID())
	{
		ZED::Log::Warn("White and blue fighters need to be different");
		return false;
	}

	Lock();

	//Do we have the rule set already?
	if (!m_StandingData.FindRuleSet(NewMatch->GetRuleSet().GetUUID()))
		m_StandingData.AddRuleSet(new RuleSet(NewMatch->GetRuleSet()));//Copy rule set

	auto dependencies = NewMatch->GetDependentMatches();//Does this match depend on any other match?

	for (auto prevMatch : dependencies)//For all dependencies
		if (prevMatch) AddMatch(prevMatch);//Include them as well recursively

	//If the match has judoka attached, include them as participants
	if (NewMatch->GetFighter(Fighter::White) && !IsParticipant(*NewMatch->GetFighter(Fighter::White)))
		m_StandingData.AddJudoka(NewMatch->GetFighter(Fighter::White));
	if (NewMatch->GetFighter(Fighter::Blue) && !IsParticipant(*NewMatch->GetFighter(Fighter::Blue)))
		m_StandingData.AddJudoka(NewMatch->GetFighter(Fighter::Blue));

	Unlock();

	if (NewMatch->GetFighter(Fighter::White) && IsDisqualified(*NewMatch->GetFighter(Fighter::White)))
	{
		//Store result
		Match::Result result(Fighter::Blue, Match::Score::Ippon);
		NewMatch->SetResult(result);
		NewMatch->EndMatch();//Mark match as concluded
	}
	if (NewMatch->GetFighter(Fighter::Blue) && IsDisqualified(*NewMatch->GetFighter(Fighter::Blue)))
	{
		if (NewMatch->HasConcluded())//Double disqualification?
		{
			//Store result
			Match::Result result(Winner::Draw, Match::Score::Draw);
			NewMatch->SetResult(result);
			NewMatch->EndMatch();//Mark match as concluded
		}

		else
		{
			//Store result
			Match::Result result(Fighter::White, Match::Score::Ippon);
			NewMatch->SetResult(result);
			NewMatch->EndMatch();//Mark match as concluded
		}
	}

	NewMatch->SetScheduleIndex(GetMaxScheduleIndex() + 1);

	Lock();

	m_Schedule.emplace_back(NewMatch);
	m_SchedulePlanner.emplace_back(NewMatch);

	Save();
	Unlock();
	return true;
}



Match* Tournament::GetNextMatch(int32_t MatID) const
{
	for (auto match : m_Schedule)
	{
		if (!match || match->HasConcluded() || match->IsRunning())
			continue;

		if (MatID < 0 || match->GetMatID() == MatID)
			return match;
	}

	return nullptr;
}



const Match* Tournament::GetNextMatch(int32_t MatID, uint32_t& StartIndex) const
{
	for (; StartIndex < m_Schedule.size(); StartIndex++)
	{
		if (m_Schedule[StartIndex]->HasConcluded() || m_Schedule[StartIndex]->IsRunning())
			continue;

		if (MatID < 0 || m_Schedule[StartIndex]->GetMatID() == MatID)
		{
			StartIndex++;
			return m_Schedule[StartIndex-1];
		}
	}

	return nullptr;
}



bool Tournament::RemoveMatch(const UUID& MatchID)
{
	if (GetStatus() == Status::Concluded)
		return false;

	for (auto it = m_SchedulePlanner.begin(); it != m_SchedulePlanner.end(); ++it)
	{
		if ((*it)->GetSchedule().size() == 1 && (*it)->GetSchedule()[0]->GetUUID() == MatchID)
		{
			it = m_SchedulePlanner.erase(it);
			break;
		}
	}

	for (auto it = m_Schedule.begin(); it != m_Schedule.end(); ++it)
	{
		if ((*it)->GetUUID() == MatchID)
		{
			it = m_Schedule.erase(it);
			break;
		}
	}

	Save();
	return true;
}



Match* Tournament::FindMatch(const UUID& UUID) const
{
	for (auto match : m_Schedule)
		if (match && match->GetUUID() == UUID)
			return match;
	return nullptr;
}



bool Tournament::MoveMatchUp(const UUID& MatchID)
{
	size_t Index = 0;
	for (; Index < m_Schedule.size(); Index++)
	{
		if (m_Schedule[Index]->GetUUID() == MatchID)
			break;
	}

	if (Index == 0 || Index == m_Schedule.size())
		return false;

	auto prev_match = m_Schedule[Index - 1];
	auto match = m_Schedule[Index];

	if (!prev_match || !match)
		return false;

	//Is either match running?
	if (!prev_match->IsScheduled() || !match->IsScheduled())
		return false;

	//Swap matches
	Lock();

	m_Schedule[Index-1] = match;
	m_Schedule[Index]   = prev_match;

	Unlock();
	Save();

	return true;
}



bool Tournament::MoveMatchDown(const UUID& MatchID)
{
	size_t Index = 0;
	bool found = false;
	for (; Index < m_Schedule.size(); Index++)
	{
		if (m_Schedule[Index]->GetUUID() == MatchID)
		{
			found = true;
			break;
		}
	}

	if (!found || Index+1 >= m_Schedule.size())
		return false;

	auto match = m_Schedule[Index];
	auto next_match = m_Schedule[Index + 1];

	if (!match || !next_match)
		return false;

	//Is either match running?
	if (!match->IsScheduled() || !next_match->IsScheduled())
		return false;

	//Swap matches
	Lock();
	m_Schedule[Index]   = next_match;
	m_Schedule[Index+1] = match;

	Unlock();
	Save();

	return true;
}



std::vector<Match> Tournament::GetNextMatches(uint32_t MatID) const
{
	std::vector<Match> ret;

	Lock();

	uint32_t id = 0;
	for (int i = 0; i < 5; i++)
	{
		auto nextMatch = GetNextMatch(MatID, id);

		if (nextMatch)
			ret.push_back(*nextMatch);
	}

	Unlock();
	return ret;
}



bool Tournament::AddParticipant(Judoka* Judoka)
{
	if (!Judoka || IsParticipant(*Judoka))
		return false;

	Lock();
	m_StandingData.AddJudoka(Judoka);

	for (auto table : m_MatchTables)
	{
		if (table && table->IsElgiable(*Judoka))
		{
			table->AddParticipant(Judoka);
			table->GenerateSchedule();
		}
	}

	FindAgeGroupForJudoka(*Judoka);

	GenerateSchedule();
	Unlock();

	return true;
}



bool Tournament::RemoveParticipant(const UUID& UUID)
{
	const Judoka* deleted_judoka = m_StandingData.FindJudoka(UUID);

	if (!deleted_judoka)
		return false;

	m_StandingData.DeleteJudoka(UUID);

	for (auto table : m_MatchTables)
	{
		if (!table->Contains(deleted_judoka))
			continue;

		table->SetParticipants().clear();

		for (auto& [id, judoka] : m_StandingData.GetAllJudokas())
		{
			if (table->IsElgiable(*judoka))
			{
				table->AddParticipant(judoka);
				table->GenerateSchedule();
			}
		}
	}

	m_JudokaToAgeGroup.erase(deleted_judoka->GetUUID());

	GenerateSchedule();//Recalculate schedule

	return true;
}



uint32_t Tournament::GetHighestMatIDUsed() const
{
	uint32_t max = 0;

	for (auto match : m_Schedule)
		if (match && match->GetMatID() > max)
			max = match->GetMatID();

	return max;
}



bool Tournament::IsMatUsed(uint32_t ID) const
{
	for (const auto match : m_Schedule)
		if (match && match->GetMatID() == ID)
			return true;

	return false;
}



MatchTable* Tournament::FindMatchTable(const UUID& ID)
{
	for (auto table : m_MatchTables)
		if (table && table->GetUUID() == ID)
			return table;
	return nullptr;
}



const MatchTable* Tournament::FindMatchTable(const UUID& ID) const
{
	for (auto table : m_MatchTables)
		if (table && table->GetUUID() == ID)
			return table;
	return nullptr;
}



MatchTable* Tournament::FindMatchTableByName(const std::string& Name)
{
	for (auto table : m_MatchTables)
		if (table && table->GetName() == Name)
			return table;
	return nullptr;
}



void Tournament::AddMatchTable(MatchTable* NewMatchTable)
{
	if (!NewMatchTable)
		return;

	//Add all eligable participants to the match table
	for (auto [id, judoka] : m_StandingData.GetAllJudokas())//TODO This is bad if NewMatchTable is already stored on disk
	{
		if (judoka && NewMatchTable->IsElgiable(*judoka))
			NewMatchTable->AddParticipant(judoka);
	}

	if (NewMatchTable->GetScheduleIndex() < 0)//No schedule index?
		NewMatchTable->SetScheduleIndex(GetMaxScheduleIndex() + 1);//Put at the bottom of list
	NewMatchTable->GenerateSchedule();

	//Find a new color for this match table
	Color match_table_color = Color::Name::Blue;
	for (auto table : m_MatchTables)
	{
		if (table->GetColor() >= match_table_color)
			match_table_color = table->GetColor() + 1;
	}
	NewMatchTable->SetColor(match_table_color);

	m_MatchTables.push_back(NewMatchTable);
	m_SchedulePlanner.push_back(NewMatchTable);

	GenerateSchedule();
}



bool Tournament::UpdateMatchTable(const UUID& UUID)
{
	auto matchTable = FindMatchTable(UUID);

	if (!matchTable)
		return false;

	if (matchTable->GetStatus() == Status::Scheduled)//Can safely recalculate the match table
	{
		matchTable->SetParticipants().clear();

		for (auto& [id, judoka] : m_StandingData.GetAllJudokas())
		{
			if (judoka && matchTable->IsElgiable(*judoka))
				matchTable->AddParticipant(judoka);
		}

		matchTable->GenerateSchedule();
		GenerateSchedule();
	}

	return true;
}



bool Tournament::RemoveMatchTable(const UUID& UUID)
{
	auto matchTable = FindMatchTable(UUID);

	if (!matchTable)
		return false;

	if (matchTable->GetStatus() != Status::Scheduled)//Can safely delete the match table
		return false;

	//Remove match table
	for (auto table = m_MatchTables.begin(); table != m_MatchTables.end(); ++table)
	{
		if (*table && (*table)->GetUUID() == UUID)
		{
			table = m_MatchTables.erase(table);
			break;
		}
	}

	//Remove the table for the schedule
	for (auto schedule_entry = m_SchedulePlanner.begin(); schedule_entry != m_SchedulePlanner.end(); ++schedule_entry)
	{
		if (*schedule_entry && (*schedule_entry)->GetUUID() == UUID)
		{
			schedule_entry = m_SchedulePlanner.erase(schedule_entry);
			break;
		}
	}

	return true;
}



bool Tournament::AddAgeGroup(AgeGroup* NewAgeGroup)
{
	if (!NewAgeGroup)
		return false;

	if (m_StandingData.FindAgeGroup(NewAgeGroup->GetUUID()))
		return false;

	if (!m_StandingData.AddAgeGroup(NewAgeGroup))
		return false;

	Lock();
	for (auto [id, judoka] : m_StandingData.GetAllJudokas())
	{
		auto age_group = GetAgeGroupOfJudoka(judoka);

		//Not assigned to any age group and eligable for this new one?
		if (!age_group && NewAgeGroup->IsElgiable(*judoka))
		{
			//Add him to his new age group
			m_JudokaToAgeGroup.insert({ judoka->GetUUID(), NewAgeGroup->GetUUID() });
		}
	}
	Unlock();

	Save();
	return true;
}



bool Tournament::RemoveAgeGroup(const UUID& UUID)
{
	auto age_group_to_remove = m_StandingData.FindAgeGroup(UUID);

	if (!age_group_to_remove)
		return false;

	if (!m_StandingData.RemoveAgeGroup(UUID))
		return false;

	Lock();

	//Remove all assignments from this age group
	for (auto [judoka, age_group] : m_JudokaToAgeGroup)
	{
		if (age_group == UUID)
			m_JudokaToAgeGroup.erase(judoka);//Remove the assignment
	}

	//Assign not-assigned judoka to a age group if possible
	for (auto [id, judoka] : m_StandingData.GetAllJudokas())
	{
		//Not assigned to any age group?
		if (judoka && !GetAgeGroupOfJudoka(judoka))
			FindAgeGroupForJudoka(*judoka);
	}

	Unlock();

	Save();
	return true;
}



bool Tournament::AssignJudokaToAgeGroup(const Judoka* Judoka, const AgeGroup* AgeGroup)
{
	if (!Judoka || !AgeGroup)
		return false;

	if (!IsParticipant(*Judoka))
		return false;

	if (!m_StandingData.FindAgeGroup(AgeGroup->GetUUID()))
		return false;

	Lock();

	//Remove judoka to the age group he currently belongs to
	m_JudokaToAgeGroup.erase(Judoka->GetUUID());

	//Add him to his new age group
	m_JudokaToAgeGroup.insert({ Judoka->GetUUID(), AgeGroup->GetUUID() });

	Unlock();

	Save();
	return true;
}



void Tournament::ListAgeGroups(YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginSeq;

	for (auto age_group : m_StandingData.GetAgeGroups())
	{
		Yaml << YAML::BeginMap;

		Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)age_group->GetUUID();
		Yaml << YAML::Key << "name" << YAML::Value << age_group->GetName();

		size_t num_match_tables = 0;
		size_t num_matches = 0;
		for (auto match_table : m_MatchTables)
		{
			if (match_table->GetAgeGroup()->GetUUID() == age_group->GetUUID())
			{
				num_match_tables++;
				num_matches += match_table->GetSchedule().size();
			}
		}

		Yaml << YAML::Key << "num_match_tables" << YAML::Value << num_match_tables;
		Yaml << YAML::Key << "num_matches" << YAML::Value << num_matches;

		size_t num_participants = 0;
		for (auto& [judoka_id, age_group_id] : m_JudokaToAgeGroup)
		{
			if (age_group_id == age_group->GetUUID())
				num_participants++;
		}

		Yaml << YAML::Key << "num_participants" << YAML::Value << num_participants;
		Yaml << YAML::EndMap;
	}

	Yaml << YAML::EndSeq;
}



Schedulable* Tournament::GetScheduleEntry(const UUID& UUID)
{
	for (auto entry : m_SchedulePlanner)
		if (entry->GetUUID() == UUID)
			return entry;
	return nullptr;
}



bool Tournament::MoveScheduleEntryUp(const UUID& UUID)
{
	if (GetStatus() == Status::Concluded)
		return false;

	Lock();
	uint32_t index = 0;
	for (; index < m_SchedulePlanner.size(); ++index)
		if (m_SchedulePlanner[index]->GetUUID() == UUID)
			break;
	Unlock();

	if (index >= m_SchedulePlanner.size() || !m_SchedulePlanner[index])
		return false;

	if (m_SchedulePlanner[index]->GetScheduleIndex() <= 0)
		return false;

	if (m_SchedulePlanner[index]->GetStatus() != Status::Scheduled)//Don't move if already started
		return false;

	Lock();
	m_SchedulePlanner[index]->SetScheduleIndex(m_SchedulePlanner[index]->GetScheduleIndex() - 1);

	GenerateSchedule();
	Unlock();
	return true;
}



bool Tournament::MoveScheduleEntryDown(const UUID& UUID)
{
	if (GetStatus() == Status::Concluded)
		return false;

	Lock();
	uint32_t index = 0;
	for (; index < m_SchedulePlanner.size(); ++index)
		if (m_SchedulePlanner[index]->GetUUID() == UUID)
			break;
	Unlock();

	if (index >= m_SchedulePlanner.size() || !m_SchedulePlanner[index])
		return false;

	auto entry = m_SchedulePlanner[index];

	//Is already at the last position and alone?
	if (entry->GetScheduleIndex() == GetMaxScheduleIndex() && GetMaxEntriesAtScheduleIndex(0, entry->GetScheduleIndex()) == 1)
		return false;

	if (entry->GetStatus() != Status::Scheduled)//Don't move if already started
		return false;

	Lock();
	m_SchedulePlanner[index]->SetScheduleIndex(m_SchedulePlanner[index]->GetScheduleIndex() + 1);

	GenerateSchedule();
	Unlock();
	return true;
}



bool Tournament::IsDisqualified(const Judoka& Judoka) const
{
	Lock();

	bool ret = m_DisqualifiedJudoka.find(Judoka.GetUUID()) != m_DisqualifiedJudoka.cend();

	Unlock();

	return ret;
}



void Tournament::Disqualify(const Judoka& Judoka)
{
	Lock();

	m_DisqualifiedJudoka.insert(Judoka.GetUUID());

	for (auto match : m_Schedule)
	{
		//Scheduled match where we are participating?
		if (!match->HasConcluded() && match->Contains(Judoka))
		{
			//Determine winner of this match
			const Fighter winner = !match->GetColorOfFighter(Judoka);
			
			//Store result
			Match::Result result(winner, Match::Score::Ippon);
			match->SetResult(result);
			match->EndMatch();//Mark match as concluded
		}

		//Concluded match via disqualification
		else if (match->HasConcluded() && match->GetLog().GetNumEvent() == 0 && match->Contains(Judoka))
		{
			if (match->GetWinningJudoka() && *match->GetWinningJudoka() == Judoka)
			{
				match->SetResult(Match::Result(Winner::Draw, Match::Score::Draw));
			}
		}
	}

	Unlock();
	Save();
}



void Tournament::RevokeDisqualification(const Judoka& Judoka)
{
	if (!IsDisqualified(Judoka))
		return;

	Lock();

	m_DisqualifiedJudoka.erase(Judoka.GetUUID());

	for (auto match : m_Schedule)
	{
		if (match->HasConcluded() && match->GetLog().GetNumEvent() == 0 && match->Contains(Judoka))
		{
			auto enemy = match->GetEnemyOf(Judoka);

			if (enemy && IsDisqualified(*enemy))//Is the enemy disqualified?
			{
				//Store result
				Match::Result result(match->GetColorOfFighter(Judoka), Match::Score::Ippon);
				match->SetResult(result);
				match->EndMatch();//Mark match as concluded
			}
			else
				match->SetState(Status::Scheduled);//Reset match result
		}
	}

	Unlock();
	Save();
}



const std::string Tournament::Schedule2String() const
{
	ZED::CSV ret;
	Lock();
	for (auto match : m_Schedule)
	{
		if (match)
			ret << match->ToString();
	}
	Unlock();
	return ret;
}



const std::string Tournament::Participants2String() const
{
	YAML::Emitter ret;
	ret << YAML::BeginSeq;
	Lock();
	for (auto [id, judoka] : m_StandingData.GetAllJudokas())
	{
		uint32_t num_matches = 0;
		for (auto match : m_Schedule)
		{
			if (match && match->HasValidFighters() && match->Contains(*judoka))
				num_matches++;
		}

		auto judoka_age_group = GetAgeGroupOfJudoka(judoka);

		ret << YAML::BeginMap;

		ret << YAML::Key << "uuid" << YAML::Value << (std::string)judoka->GetUUID();
		ret << YAML::Key << "name" << YAML::Value << judoka->GetName();
		ret << YAML::Key << "weight" << YAML::Value << judoka->GetWeight();
		ret << YAML::Key << "num_matches" << YAML::Value << num_matches;

		//Calculate eligable age groups
		ret << YAML::Key << "age_groups" << YAML::Value;
		ret << YAML::BeginSeq;

		for (auto age_group : m_StandingData.GetAgeGroups())
		{
			if (age_group->IsElgiable(*judoka))
			{
				ret << YAML::BeginMap;
				ret << YAML::Key << "uuid" << YAML::Value << (std::string)age_group->GetUUID();
				ret << YAML::Key << "name" << YAML::Value << age_group->GetName();
				if (judoka_age_group)
					ret << YAML::Key << "is_assigned" << YAML::Value << (age_group->GetUUID() == judoka_age_group->GetUUID());
				ret << YAML::EndMap;
			}
		}

		ret << YAML::EndSeq;

		ret << YAML::EndMap;
	}
	Unlock();
	ret << YAML::EndSeq;
	return ret.c_str();
}



const std::string Tournament::MasterSchedule2String() const
{
	ZED::CSV ret;

	Lock();
	//ret << mats.size();

	auto highest_matID = GetHighestMatIDUsed();
	ret << highest_matID;

	//for (auto mat : mats)//For all mat IDs
	for (uint32_t matID = 1; matID <= highest_matID; matID++)
	{
		int max = 0;
		for (int32_t index = 0; index <= GetMaxScheduleIndex(matID); index++)
		{
			int width = GetMaxEntriesAtScheduleIndex(matID, index);
			if (width > max)
				max = width;
		}

		ret << max << matID;
	}

	for (int32_t index = 0; index <= GetMaxScheduleIndex(); index++)//For all times
	{
		//for (auto mat : mats)//For all mat IDs
		for (uint32_t matID = 1; matID <= highest_matID; matID++)
		{
			uint32_t max = 0;
			for (int32_t i = 0; i <= GetMaxScheduleIndex(matID); i++)
				if (GetMaxEntriesAtScheduleIndex(matID, i) > max)
					max = GetMaxEntriesAtScheduleIndex(matID, i);

			std::vector<std::pair<uint32_t, Schedulable*>> entries;
			for (uint32_t it = 0; it < m_SchedulePlanner.size(); ++it)//For all schedule entries
			{
				auto entry = m_SchedulePlanner[it];

				if (!entry || entry->GetScheduleIndex() != index || entry->GetMatID() != matID)
					continue;

				entries.push_back({ it, entry });
			}

			ret << max << entries.size();
			for (auto [index, entry] : entries)
				ret << entry->IsEditable() << index << (std::string)entry->GetUUID() << entry->GetColor().ToHexString() << entry->GetMatID() << entry->GetName();
		}
	}

	Unlock();
	return ret;
}



void Tournament::FindAgeGroupForJudoka(const Judoka& Judoka)
{
	//Find age groups this judoka can belong to
	std::vector<AgeGroup*> EligableAgeGroups;
	for (auto age_group : m_StandingData.GetAgeGroups())
	{
		if (age_group && age_group->IsElgiable(Judoka))
			EligableAgeGroups.emplace_back(age_group);
	}

	//Only one choice?
	if (EligableAgeGroups.size() == 1)//Add judoka to age group
		m_JudokaToAgeGroup.insert({ Judoka.GetUUID(), EligableAgeGroups[0]->GetUUID() });
}



int32_t Tournament::GetMaxScheduleIndex(uint32_t Mat) const
{
	int32_t ret = -1;

	for (auto entry : m_SchedulePlanner)
	{
		if (entry && entry->GetScheduleIndex() > ret)
		{
			if (Mat == 0 || entry->GetMatID() == Mat)
				ret = entry->GetScheduleIndex();
		}
	}

	return ret;
}



uint32_t Tournament::GetMaxEntriesAtScheduleIndex(uint32_t MatID, int32_t ScheduleIndex) const
{
	uint32_t count = 0;

	for (auto entry : m_SchedulePlanner)
	{
		if (entry && (ScheduleIndex < 0 || entry->GetScheduleIndex() == ScheduleIndex) )
			if (MatID == 0 || entry->GetMatID() == MatID)
				count++;
	}

	return count;
}



void Tournament::GenerateSchedule()
{
	if (GetStatus() != Status::Scheduled)
		return;

	Lock();

	for (auto match : m_Schedule)
		delete match;
	m_Schedule.clear();

	for (auto table : m_MatchTables)
	{
		if (table)
			table->GenerateSchedule();
	}

	for (int32_t index = 0; index <= GetMaxScheduleIndex(); index++)
	{
		std::vector<std::pair<uint32_t, std::vector<Match*>>> Plan;

		for (auto entry : m_SchedulePlanner)
		{
			if (!entry || entry->GetScheduleIndex() != index)
				continue;

			auto schedule = entry->GetSchedule();
			Plan.push_back({ entry->GetRecommendedNumMatchesBeforeBreak(), schedule });
		}

		std::sort(Plan.begin(), Plan.end(), [](auto& a, auto& b) {
			return a.second.size() > b.second.size();
		});

		while (true)
		{
			bool done = true;

			for (auto& [num, schedule] : Plan)
			{
				for (uint32_t i = 0; i < num; i++)
				{
					if (schedule.size() > 0)
					{
						auto match = schedule.front();
						m_Schedule.push_back(match);

						schedule.erase(schedule.begin());
						done = false;
					}
				}
			}

			if (done)
				break;
		}
	}

	Unlock();

	assert(Save());
}