#include <fstream>
#include <algorithm>
#include <cassert>
#include "../ZED/include/log.h"
#include "tournament.h"
#include "database.h"
#include "weightclass.h"
#include "md5.h"



using namespace Judoboard;



Tournament::Tournament(const std::string& Name) : m_Name(Name)
{
	Load("tournaments/" + Name);
	//if (!Load("tournaments/" + Name))
		//m_pDefaultRules = new RuleSet();
}



Tournament::Tournament(const std::string& Name, const RuleSet* RuleSet) : m_pDefaultRules(RuleSet), m_Name(Name)
{
	Load("tournaments/" + Name);
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
		{
			new_judoka = pDatabase->UpdateOrAdd(*judoka);
			
			if (new_judoka)
				m_StandingData.AddJudoka(new_judoka);//Add participant
		}

		else
			Judoka* new_judoka = new Judoka(*judoka);

		if (new_judoka)
		{
			judoka->pUserData = new_judoka;
			AddParticipant(new_judoka);

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

		if (match.Weightclass && match.Weightclass->pUserData)
		{
			auto match_table = (Weightclass*)match.Weightclass->pUserData;
			match_table->AddMatch(new_match);//Add match to weightclass
		}

		m_Schedule.emplace_back(new_match);
	}
}



Tournament::~Tournament()
{
	if (m_Schedule.size() > 0 || GetParticipants().size() > 0 || m_MatchTables.size() > 0 || m_SchedulePlanner.size() > 0)
		Save();
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
			AddMatchTable(new_table);
			new_table->SetSchedule().clear();
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
			auto id = new_match->GetMatchTable()->GetID();
			FindMatchTable(id)->SetSchedule().emplace_back(new_match);
		}

		m_Schedule.emplace_back(new_match);
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
	std::vector<RuleSet*> rule_sets_to_add;

	for (auto it = m_StandingData.GetRuleSets().begin(); it != m_StandingData.GetRuleSets().end();)
	{
		auto* db_ref = db.FindRuleSet((*it)->GetUUID());
		if (db_ref)
		{
			delete *it;
			it = m_StandingData.GetRuleSets().erase(it);
			rule_sets_to_add.push_back(db_ref);
		}
		else
			++it;
	}

	for (auto* rule : rule_sets_to_add)
		m_StandingData.AddRuleSet(rule);

	if (m_pDefaultRules)
	{
		auto* db_ref = db.FindRuleSet(m_pDefaultRules->GetUUID());
		if (db_ref)
			m_pDefaultRules = db_ref;
	}
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
		NewMatch->GetFighter(Fighter::White)->GetID() == NewMatch->GetFighter(Fighter::Blue)->GetID())
	{
		ZED::Log::Warn("White and blue fighters need to be different");
		return false;
	}

	Lock();

	//Do we have the rule set already?
	if (!m_StandingData.FindRuleSet(NewMatch->GetRuleSet().GetID()))
		m_StandingData.AddRuleSet(new RuleSet(NewMatch->GetRuleSet()));//Copy rule set

	auto dependencies = NewMatch->GetDependentMatches();//Does this match depend on any other match?

	for (auto prevMatch : dependencies)//For all dependencies
		if (prevMatch) AddMatch(prevMatch);//Include them as well recursively

	//If the match has judoka attached, include them as participants
	if (NewMatch->GetFighter(Fighter::White) && !IsParticipant(*NewMatch->GetFighter(Fighter::White)))
		m_StandingData.AddJudoka(NewMatch->GetFighter(Fighter::White));
	if (NewMatch->GetFighter(Fighter::Blue) && !IsParticipant(*NewMatch->GetFighter(Fighter::Blue)))
		m_StandingData.AddJudoka(NewMatch->GetFighter(Fighter::Blue));

	NewMatch->SetScheduleIndex(GetMaxScheduleIndex() + 1);

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



bool Tournament::DeleteMatch(uint32_t MatchID)
{
	if (GetStatus() == Status::Concluded)
		return false;

	for (auto it = m_SchedulePlanner.begin(); it != m_SchedulePlanner.end(); ++it)
	{
		if ((*it)->GetSchedule().size() == 1 && (*it)->GetSchedule()[0]->GetID() == MatchID)
		{
			it = m_SchedulePlanner.erase(it);
			break;
		}
	}

	for (auto it = m_Schedule.begin(); it != m_Schedule.end(); ++it)
	{
		if ((*it)->GetID() == MatchID)
		{
			it = m_Schedule.erase(it);
			break;
		}
	}

	Save();
	return true;
}



Match* Tournament::FindMatch(uint32_t ID) const
{
	for (auto match : m_Schedule)
		if (match && match->GetID() == ID)
			return match;
	return nullptr;
}



Match* Tournament::FindMatch(const UUID& UUID) const
{
	for (auto match : m_Schedule)
		if (match && match->GetUUID() == UUID)
			return match;
	return nullptr;
}



bool Tournament::MoveMatchUp(uint32_t MatchID)
{
	size_t Index = 0;
	for (; Index < m_Schedule.size(); Index++)
	{
		if (m_Schedule[Index]->GetID() == MatchID)
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

	Save();
	Unlock();

	return true;
}



bool Tournament::MoveMatchDown(uint32_t MatchID)
{
	size_t Index = 0;
	bool found = false;
	for (; Index < m_Schedule.size(); Index++)
	{
		if (m_Schedule[Index]->GetID() == MatchID)
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

	Save();
	Unlock();

	return true;
}



std::vector<const Match*> Tournament::GetNextMatches(uint32_t MatID) const
{
	std::vector<const Match*> ret;

	uint32_t id = 0;
	for (int i = 0; i < 5; i++)
	{
		auto* nextMatch = GetNextMatch(MatID, id);

		if (nextMatch)
			ret.push_back(nextMatch);
	}

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
		if (table->IsElgiable(*Judoka))
		{
			table->AddParticipant(Judoka);
			table->GenerateSchedule();
		}
	}

	GenerateSchedule();
	Unlock();

	return true;
}



bool Tournament::RemoveParticipant(uint32_t ID)
{
	const Judoka* deleted_judoka = m_StandingData.FindJudoka(ID);

	if (!deleted_judoka)
		return false;

	m_StandingData.DeleteJudoka(ID);

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

	Save();
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
	for (const auto* match : m_Schedule)
		if (match && match->GetMatID() == ID)
			return true;

	return false;
}



MatchTable* Tournament::FindMatchTable(uint32_t ID)
{
	for (auto table : m_MatchTables)
		if (table && table->GetID() == ID)
			return table;
	return nullptr;
}



const MatchTable* Tournament::FindMatchTable(uint32_t ID) const
{
	for (auto table : m_MatchTables)
		if (table && table->GetID() == ID)
			return table;
	return nullptr;
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



int Tournament::FindMatchTableIndex(uint32_t ID) const
{
	for (uint32_t i = 0;i < m_MatchTables.size();i++)
		if (m_MatchTables[i] && m_MatchTables[i]->GetID() == ID)
			return i;
	return -1;
}



uint32_t Tournament::GetFreeMatchTableID() const
{
	uint32_t unusedID = 1;

	while (FindMatchTable(unusedID))
		unusedID++;

	return unusedID;
}



void Tournament::AddMatchTable(MatchTable* NewMatchTable)
{
	if (!NewMatchTable)
		return;

	//Add all eligable participants to the match table
	for (auto [id, judoka] : m_StandingData.GetAllJudokas())
	{
		if (judoka && NewMatchTable->IsElgiable(*judoka))
			NewMatchTable->AddParticipant(judoka);
	}

	if (NewMatchTable->GetScheduleIndex() < 0)//No schedule index?
		NewMatchTable->SetScheduleIndex(GetMaxScheduleIndex() + 1);//Put at the bottom of list
	NewMatchTable->GenerateSchedule();

	//Find a new color for this match table
	Color match_table_color = Color::Name::Blue;
	for (auto* table : m_MatchTables)
	{
		if (table->GetColor() >= match_table_color)
			match_table_color = table->GetColor() + 1;
	}
	NewMatchTable->SetColor(match_table_color);

	m_MatchTables.push_back(NewMatchTable);
	m_SchedulePlanner.push_back(NewMatchTable);

	GenerateSchedule();
	Save();
}



void Tournament::UpdateMatchTable(uint32_t ID)
{
	auto* matchTable = FindMatchTable(ID);

	if (!matchTable)
		return;

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
}



bool Tournament::DeleteMatchTable(uint32_t ID)
{
	auto* matchTable = FindMatchTable(ID);

	if (!matchTable)
		return false;

	if (matchTable->GetStatus() != Status::Scheduled)//Can safely delete the match table
		return false;

	//Remove match table
	for (auto table = m_MatchTables.begin(); table != m_MatchTables.end(); ++table)
	{
		if (*table && (*table)->GetID() == ID)
		{
			table = m_MatchTables.erase(table);
			break;
		}
	}

	//Remove the table for the schedule
	for (auto schedule_entry = m_SchedulePlanner.begin(); schedule_entry != m_SchedulePlanner.end(); ++schedule_entry)
	{
		if (*schedule_entry && (*schedule_entry)->GetID() == ID)
		{
			schedule_entry = m_SchedulePlanner.erase(schedule_entry);
			break;
		}
	}

	return true;
}



Schedulable* Tournament::GetScheduleEntry(uint32_t Index)
{
	if (Index >= m_SchedulePlanner.size())
		return nullptr;

	if (!m_SchedulePlanner[Index] || m_SchedulePlanner[Index]->GetScheduleIndex() < 0)
		return nullptr;

	return m_SchedulePlanner[Index];
}



bool Tournament::MoveScheduleEntryUp(uint32_t Index)
{
	if (GetStatus() == Status::Concluded)
		return false;

	if (Index >= m_SchedulePlanner.size())
		return false;

	if (!m_SchedulePlanner[Index] || m_SchedulePlanner[Index]->GetScheduleIndex() <= 0)
		return false;

	if (m_SchedulePlanner[Index]->GetStatus() != Status::Scheduled)//Don't move if already started
		return false;

	Lock();
	m_SchedulePlanner[Index]->SetScheduleIndex(m_SchedulePlanner[Index]->GetScheduleIndex() - 1);

	GenerateSchedule();
	Unlock();
	return true;
}



bool Tournament::MoveScheduleEntryDown(uint32_t Index)
{
	if (GetStatus() == Status::Concluded)
		return false;

	if (Index >= m_SchedulePlanner.size())
		return false;

	if (!m_SchedulePlanner[Index])
		return false;

	auto entry = m_SchedulePlanner[Index];

	//Is already at the last position and alone?
	if (entry->GetScheduleIndex() == GetMaxScheduleIndex() && GetMaxEntriesAtScheduleIndex(0, entry->GetScheduleIndex()) == 1)
		return false;

	if (entry->GetStatus() != Status::Scheduled)//Don't move if already started
		return false;

	Lock();
	m_SchedulePlanner[Index]->SetScheduleIndex(m_SchedulePlanner[Index]->GetScheduleIndex() + 1);

	GenerateSchedule();
	Unlock();
	return true;
}



void Tournament::Disqualify(const Judoka& Judoka)
{
	Lock();

	for (auto match : m_Schedule)
	{
		//Scheduled match where we are participating?
		if (!match->HasConcluded() && match->HasValidFighters() && match->Contains(Judoka))
		{
			//Determine winner of this match
			Fighter winner = Fighter::White;
			if (*match->GetFighter(Fighter::White) == Judoka)
				winner = Fighter::Blue;
			
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
	Lock();

	for (auto match : m_Schedule)
	{
		if (match->HasConcluded() && match->GetLog().GetNumEvent() == 0 && match->Contains(Judoka))
		{
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
		if (match)
			ret << match->ToString();
	Unlock();
	return ret;
}



const std::string Tournament::Participants2String() const
{
	ZED::CSV ret;
	Lock();
	for (auto [id, judoka] : m_StandingData.GetAllJudokas())
	{
		uint32_t num_matches = 0;
		for (auto match : m_Schedule)
		{
			if (match && match->HasValidFighters() && match->Contains(*judoka))
				num_matches++;
		}

		ret << id << judoka->GetName() << judoka->GetWeight() << num_matches;
	}
	Unlock();
	return ret;
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
				auto* entry = m_SchedulePlanner[it];

				if (!entry || entry->GetScheduleIndex() != index || entry->GetMatID() != matID)
					continue;

				entries.push_back({ it, entry });
			}

			ret << max << entries.size();
			for (auto [index, entry] : entries)
				ret << entry->IsEditable() << index << entry->GetID() << entry->GetColor().ToHexString() << entry->GetMatID() << entry->GetName();
		}
	}

	Unlock();
	return ret;
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
						auto* match = schedule.front();
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

	assert(Save());
}