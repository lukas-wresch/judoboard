#include <fstream>
#include <algorithm>
#include <random>
#include <cassert>
#include "../ZED/include/log.h"
#include "tournament.h"
#include "database.h"
#include "weightclass.h"
#include "customtable.h"
#include "round_robin.h"
#include "single_elimination.h"
#include "pool.h"
#include "double_elimination.h"
#include "fixed.h"
#include "weightclass_generator.h"



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

	int day, month, year;
	if (sscanf_s(File.GetDateStart().c_str(), "D%d-%d-%d", &year, &month, &day) == 3)
		m_StandingData.SetYear(year);

	m_LotteryTier = File.GetLotteryTierID() - 2;

	//Add clubs
	for (auto club : File.GetClubs())
	{
		Club* new_club = nullptr;

		if (pDatabase && pDatabase->FindClubByName(club->Name))
			new_club = pDatabase->FindClubByName(club->Name);
		else
			new_club = new Club(*club);
		
		m_StandingData.AddClub(new_club);
		club->pUserData = new_club;
	}

	//Find organizer
	assert(File.GetOrganizer());
	if (File.GetOrganizer())
	{
		for (auto assoc : m_StandingData.GetAllAssociations())
		{
			if (assoc->GetName() == File.GetOrganizer()->Description)
			{
				m_Organizer = assoc;
				break;
			}
		}
	}

	//Add age groups
	for (auto age_group : File.GetAgeGroups())
	{
		auto new_age_group = new AgeGroup(*age_group, m_StandingData);
		age_group->pUserData = new_age_group;
		m_StandingData.AddAgeGroup(new_age_group);
	}

	//Add weightclasses
	Color color;
	for (auto weightclass : File.GetWeightclasses())
	{
		MatchTable* new_table = nullptr;

		//Round robin
		if (weightclass->FightSystemID == 13 || weightclass->FightSystemID == 14 || weightclass->FightSystemID == 15 || weightclass->FightSystemID == 16 || weightclass->FightSystemID == 17 || weightclass->FightSystemID == 41)
		{
			new_table = new RoundRobin(*weightclass, this);
			if (weightclass->FightSystemID == 41)
				new_table->IsBestOfThree(true);
		}
		else if (weightclass->FightSystemID == 19 ||//Single elimination (single consulation bracket)
				 weightclass->FightSystemID == 20)  //Single elimination (single consulation bracket)
		{
			auto single = new SingleElimination(*weightclass, this);
			single->IsThirdPlaceMatch(weightclass->MatchForThirdPlace);
			single->IsFifthPlaceMatch(weightclass->MatchForFifthPlace);
			new_table = single;
		}
		else if (weightclass->FightSystemID == 1  ||//Double elimination (16 system)
				 weightclass->FightSystemID == 2)   //Double elimination
		{
			auto de = new DoubleElimination(*weightclass, this);
			de->IsThirdPlaceMatch(weightclass->MatchForThirdPlace);
			de->IsFifthPlaceMatch(weightclass->MatchForFifthPlace);
			delete de->GetLoserBracket().GetFilter();
			de->GetLoserBracket().SetFilter(new Fixed);
			new_table = de;
		}
		else if (weightclass->FightSystemID == 24)//Pool (3+3 system)
		{
			auto pool = new Pool(*weightclass, this);
			pool->SetPoolCount(2);
			pool->IsThirdPlaceMatch(weightclass->MatchForThirdPlace);
			pool->IsFifthPlaceMatch(weightclass->MatchForFifthPlace);
			new_table = pool;
		}

		if (!new_table)
		{
			ZED::Log::Error("Unknown fight system id " + std::to_string(weightclass->FightSystemID) + ". Can not import match table from MD5/7. Skipping this table.");
			continue;
		}

		//Connect to age group
		if (weightclass->AgeGroup)
			new_table->SetAgeGroup((AgeGroup*)weightclass->AgeGroup->pUserData);

		new_table->SetName(weightclass->Description);
		//new_table->IsBestOfThree(weightclass->BestOfThree);
		new_table->SetMatID(1);//Choose 1 as the default mat
		new_table->SetScheduleIndex(GetFreeScheduleIndex(1));
		new_table->DeleteSchedule();
		new_table->AutoGenerateSchedule(false);
		new_table->SetColor(color);
		++color;

		weightclass->pUserData = new_table;

		m_MatchTables.emplace_back(new_table);
	}

	//Add judoka
	for (auto judoka : File.GetParticipants())
	{
		Judoka* new_judoka = nullptr;

		if (pDatabase && pDatabase->FindJudoka_ExactMatch(*judoka))
			new_judoka = pDatabase->FindJudoka_ExactMatch(*judoka);
		else
			new_judoka = new Judoka(JudokaData(*judoka), &m_StandingData);

		if (new_judoka)
		{
			judoka->pUserData = new_judoka;
			m_StandingData.AddJudoka(new_judoka);

			if (judoka->Club)//Connect to club
				new_judoka->SetClub((Club*)judoka->Club->pUserData);

			if (judoka->Weightclass)
			{
				auto match_table = (MatchTable*)judoka->Weightclass->pUserData;
				if (match_table)
				{
					match_table->AddParticipant(new_judoka, true);//Add with force

					//If double elimination also add as a participant to the loser bracket
					if (match_table->GetType() == MatchTable::Type::DoubleElimination)
					{
						auto de = (DoubleElimination*)match_table;
						de->GetLoserBracket().AddParticipant(new_judoka, true);
					}
				}
			}

			if (judoka->AgeGroup)
			{
				auto age_group = (AgeGroup*)judoka->AgeGroup->pUserData;
				if (age_group)
					m_JudokaToAgeGroup.insert({ new_judoka->GetUUID(), age_group->GetUUID() });
					//AssignJudokaToAgeGroup(new_judoka, age_group);
			}
		}
	}

	//Convert start positions
	for (auto judoka : File.GetParticipants())
	{
		if (!judoka->pUserData || !judoka->Weightclass)
			continue;

		Judoka* native_judoka = (Judoka*)judoka->pUserData;
		auto match_table = (MatchTable*)judoka->Weightclass->pUserData;

		int pos = File.FindStartNo(judoka->Weightclass->AgeGroupID, judoka->Weightclass->ID, judoka->ID);
		if (pos > 0 && match_table)
			match_table->SetStartPosition(native_judoka, pos - 1);
	}

	//Convert lots
	for (auto lot : File.GetLottery())
	{
		auto assoc = File.FindAssociation(lot.AssociationID);
		
		if (assoc)
		{
			auto native_assoc = m_StandingData.FindAssociationByName(assoc->Description);
			if (native_assoc)
				m_AssociationToLotNumber.emplace_back(*native_assoc, lot.StartNo);
		}
	}

	//Clear all matches (these got added when the participants got added)
	for (auto table : m_MatchTables)
		table->DeleteSchedule();

	//Add matches
	std::vector<int> injured_judokas;
	for (auto& match : File.GetMatches())
	{
		Judoka* white = nullptr;
		if (match.White && match.White->pUserData)
			white = (Judoka*)match.White->pUserData;

		Judoka* blue = nullptr;
		if (match.Red && match.Red->pUserData)
			blue = (Judoka*)match.Red->pUserData;

		if (white && blue && *white == *blue)//Filter dummy matches
			continue;

		//Filter UP and DOWN matches in round robin (MD7)
		if ((!white || !blue) && match.Weightclass && match.Weightclass->pUserData)
				if ( ((MatchTable*)match.Weightclass->pUserData)->GetType() == MatchTable::Type::RoundRobin)
					continue;

		//if (!white && !blue)//Filter dummy matches
			//continue;
		
		Match* new_match = new Match(white, blue, this);

		if (match.Result == 1)//Match completed?
		{
			if (match.WinnerID == match.WhiteID)
				new_match->SetResult(Match::Result(Fighter::White, (Match::Score)match.ScoreWinner, match.Time));
			else
				new_match->SetResult(Match::Result(Fighter::Blue,  (Match::Score)match.ScoreWinner, match.Time));
		}
		else if (match.WhiteOutMatchNo >= match.MatchNo)//White injured?
		{
			injured_judokas.push_back(match.WhiteID);
			new_match->SetResult(Match::Result(Fighter::Blue, (Match::Score)match.ScoreWinner, match.Time));
		}
		else if (match.RedOutMatchNo >= match.MatchNo)//Red/Blue injured?
		{
			injured_judokas.push_back(match.RedID);
			new_match->SetResult(Match::Result(Fighter::White, (Match::Score)match.ScoreWinner, match.Time));
		}
		else//Is one judoka injured?
		{
			if (std::find(injured_judokas.begin(), injured_judokas.end(), match.WhiteID) != injured_judokas.end())
				new_match->SetResult(Match::Result(Fighter::Blue, Match::Score::Ippon, 0));
			else if (std::find(injured_judokas.begin(), injured_judokas.end(), match.RedID) != injured_judokas.end())
				new_match->SetResult(Match::Result(Fighter::White, Match::Score::Ippon, 0));
		}

		if (match.Weightclass && match.Weightclass->pUserData)
		{
			auto match_table = (MatchTable*)match.Weightclass->pUserData;

			//Add match to match table
			if (match_table->GetType() == MatchTable::Type::DoubleElimination)
			{
				auto de = (DoubleElimination*)match_table;

				if (match.AreaID == 1 || match.AreaID == 3 || match.AreaID == 4 || match.AreaID == 5)
					de->AddMatchToLoserBracket(new_match);
				else
					de->AddMatchToWinnerBracket(new_match);
			}
			else if (match_table->GetType() == MatchTable::Type::Pool)
			{
				auto pool = (Pool*)match_table;

				assert(match.Pool <= pool->GetPoolCount());

				if (match.Pool == 0)
				{
					if (match.Status != 0)
					{
						if (match.AreaID == 9)
							new_match->SetTag(Match::Tag::Finals());
						else if (match.AreaID == 4)
							new_match->SetTag(Match::Tag::Third());
						else if (match.AreaID == 5)
							new_match->SetTag(Match::Tag::Fifth());

						pool->GetFinals().AddMatch(new_match);
					}
				}
				else if (match.Pool <= pool->GetPoolCount())
					pool->GetPool(match.Pool - 1)->AddMatch(new_match);
				else
					ZED::Log::Error("Can not import match into pool match table");

				pool->CopyMatchesFromSubtables();
			}
			else
				match_table->AddMatch(new_match);
		}
	}

	//Re-order matches (for elimination for third and fifth place)
	for (auto table : m_MatchTables)
	{
		if (table->GetType() == MatchTable::Type::Pool)
		{
			auto pool = (Pool*)table;
			if (pool->GetFinals().GetSchedule().empty())
			{
				//pool->GetFinals().GenerateSchedule();
				pool->AutoGenerateSchedule(true);
				pool->GenerateSchedule();
				pool->CopyMatchesFromSubtables();

				//for (auto match : pool->GetFinals().GetSchedule())
					//m_Schedule.emplace_back(table, table->FindMatchIndex(*match));
			}
			//else
				//pool->GetFinals().ReorderLastMatches();
		}
		else if (table->GetType() == MatchTable::Type::SingleElimination)
		{
			auto se = (SingleElimination*)table;
			//se->ReorderLastMatches();
		}
		else if (table->GetType() == MatchTable::Type::DoubleElimination)
		{
			auto de = (DoubleElimination*)table;
			de->AutoGenerateSchedule(true);
			de->GenerateSchedule();//Re-generated matches
			//de->GetWinnerBracket().ReorderLastMatches();
		}
		else if (table->GetType() == MatchTable::Type::RoundRobin && table->IsBestOfThree())
		{
			if (table->GetSchedule().size() == 3)
				table->SetSchedule()[2]->SetBestOfThree(table->GetSchedule()[0], table->GetSchedule()[1]);
		}
	}

	//Build schedule
	BuildSchedule();

	//Check number of matches
#ifdef _DEBUG
	for (auto weightclass : File.GetWeightclasses())
	{
		auto table = (MatchTable*)weightclass->pUserData;
		//auto md5_schedule = File.FindMatchesOfWeightclass(weightclass->AgeGroupID, weightclass->ID);

		if (!table || table->GetType() == MatchTable::Type::RoundRobin)
			continue;

		//assert(table->GetSchedule().size() == md5_schedule.size());
	}

	//Check for duplicates
	auto schedule = GetSchedule();
	for (size_t i = 0; i < schedule.size(); ++i)
		for (size_t j = i + 1; j < schedule.size(); ++j)
	{
		assert(schedule[i]->GetUUID() != schedule[j]->GetUUID());
	}

	//Check for invalids
	for (auto match : schedule)
		assert(!match->IsEmptyMatch());
#endif

	//Re-enabled auto generation
	for (auto table : m_MatchTables)
		table->AutoGenerateSchedule(true);
  
	//If there are not matches, create them
	if (GetSchedule().size() == 0)
		GenerateSchedule();
}



Tournament::~Tournament()
{
	if (m_Schedule.size() > 0 || GetParticipants().size() > 0 || m_MatchTables.size() > 0)
		Save();

	for (auto table : m_MatchTables)
		delete table;
}



void Tournament::Reset()
{
	if (IsReadonly())
		return;

	m_Description.clear();
  
	//Clear but don't delete standing data since this could be shared with a database
	m_StandingData.GetAllJudokas().clear();
	m_StandingData.GetRuleSets().clear();

	for (auto table : m_MatchTables)
		delete table;
	m_MatchTables.clear();

	m_Schedule.clear();

	//Don't delete since this could be shared memory
	//m_pDefaultRules = nullptr;

	m_Organizer = nullptr;

	assert(m_StandingData.GetNumJudoka() == 0);
}



bool Tournament::Load(const YAML::Node& yaml)
{
	if (!yaml["version"] || !yaml["name"])
	{
		ZED::Log::Warn("Data is not a valid tournament file");
		return false;
	}

	if (yaml["version"].as<int>() != 1)
	{
		ZED::Log::Warn("Tournament file has been written with a new version. Please update this software!");
		return false;
	}

	auto guard = LockWriteForScope();

	m_Name = yaml["name"].as<std::string>();

	//Read standing data
	m_StandingData << yaml;

	if (yaml["description"] && yaml["description"].IsScalar())
		m_Description = yaml["description"].as<std::string>();

	if (yaml["readonly"] && yaml["readonly"].IsScalar())
		IsReadonly(yaml["readonly"].as<bool>());

	if (yaml["organizer"] && yaml["organizer"].IsScalar())
	{
		m_Organizer = m_StandingData.FindAssociation(yaml["organizer"].as<std::string>());
		if (!m_Organizer)
			m_Organizer = m_StandingData.FindClub(yaml["organizer"].as<std::string>());

		if (!m_Organizer)
			ZED::Log::Error("Could not resolve organizer in tournament data");
	}

	if (yaml["lottery_tier"] && yaml["lottery_tier"].IsScalar())
		m_LotteryTier = yaml["lottery_tier"].as<uint32_t>();

	if (yaml["lots"] && yaml["lots"].IsMap())
	{
		for (const auto& node : yaml["lots"])
			m_AssociationToLotNumber.emplace_back(node.first.as<std::string>(), node.second.as<size_t>());
	}

	if (yaml["judoka_to_age_group"] && yaml["judoka_to_age_group"].IsMap())
	{
		for (const auto& node : yaml["judoka_to_age_group"])
			m_JudokaToAgeGroup.insert({ node.first.as<std::string>(), node.second.as<std::string>() });
	}

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

			MatchTable* new_table = MatchTable::CreateMatchTable(node, this);

			if (new_table)
				m_MatchTables.push_back(new_table);
		}
	}

	m_Schedule.clear();

	if (yaml["schedule"] && yaml["schedule"].IsSequence())
	{
		for (const auto& node : yaml["schedule"])
		{
			if (!node.IsScalar())
				continue;

#ifdef _DEBUG
			auto old_size = m_Schedule.size();
#endif

			UUID id = node.as<std::string>();
			
			for (auto table : m_MatchTables)
			{
				auto index = table->FindMatchIndex(id);
				if (index != SIZE_MAX)
					m_Schedule.emplace_back(table, index);
			}

#ifdef _DEBUG
			assert(m_Schedule.size() == old_size + 1);
#endif
		}
	}

	m_LastSaveTime = Timer::GetTimestamp();

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
	else
		ZED::Log::Info("Opening file " + Filename);

	YAML::Node yaml = YAML::LoadFile(Filename);

	if (!yaml)
	{
		ZED::Log::Warn("Could not open file " + Filename);
		return false;
	}

	if (!Load(yaml))
	{
		ZED::Log::Error("Failed to load tournament " + Filename);
		return false;
	}

	ZED::Log::Info("Tournament " + Filename + " loaded successfully");
	return true;
}



bool Tournament::SaveYAML(const std::string& Filename)
{
	if (m_Name.empty())
		return false;

	std::ofstream file(Filename);

	if (!file)
	{
		ZED::Log::Error("Could not open file: " + Filename);
		return false;
	}

	auto guard = LockReadForScope();

	YAML::Emitter yaml;

	yaml << YAML::BeginMap;
	yaml << YAML::Key << "name";
	yaml << YAML::Value << m_Name;
	yaml << YAML::Key << "version";
	yaml << YAML::Value << "1";

	//Prune unused clubs
	std::set<UUID> used_clubs;
	for (auto judoka : m_StandingData.GetAllJudokas())
		if (judoka->GetClub())
			used_clubs.insert(*judoka->GetClub());

	for (auto club : m_StandingData.GetAllClubs())
	{
		if (used_clubs.find(*club) == used_clubs.end())//Not found
		{
			m_StandingData.DeleteClub(*club);
			break;//Have to stop since we have deleted while iterating
		}
	}

	m_StandingData >> yaml;

	if (!m_Description.empty())
		yaml << YAML::Key << "description" << YAML::Value << m_Description;

	if (IsReadonly())
		yaml << YAML::Key << "readonly" << YAML::Value << IsReadonly();

	if (m_Organizer)
		yaml << YAML::Key << "organizer" << YAML::Value << (std::string)m_Organizer->GetUUID();
	if (m_LotteryTier > 0)
		yaml << YAML::Key << "lottery_tier" << YAML::Value << m_LotteryTier;

	yaml << YAML::Key << "lots";
	yaml << YAML::Value;
	yaml << YAML::BeginMap;

	for (auto [assoc, lot] : m_AssociationToLotNumber)
		yaml << YAML::Key << (std::string)assoc << YAML::Value << lot;

	yaml << YAML::EndMap;

	yaml << YAML::Key << "judoka_to_age_group";
	yaml << YAML::Value;
	yaml << YAML::BeginMap;

	for (const auto& [judoka_uuid, age_group_uuid] : m_JudokaToAgeGroup)
		yaml << YAML::Key << (std::string)judoka_uuid << YAML::Value << (std::string)age_group_uuid;

	yaml << YAML::EndMap;

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

	auto schedule = GetSchedule();
	for (auto match : schedule)
		yaml << (std::string)match->GetUUID();

#ifdef _DEBUG
	for (const auto& match : schedule)
	{
		assert(!match->IsEmptyMatch());
		assert(!match->IsCompletelyEmptyMatch());

		UUID id = match->GetUUID();
		bool found = false;

		for (auto table : m_MatchTables)
		{
			auto index = table->FindMatchIndex(id);
			if (index != SIZE_MAX)
				found = true;
		}

		assert(found);
	}
#endif

	yaml << YAML::EndSeq;

	yaml << YAML::EndMap;
	file << yaml.c_str();

	m_LastSaveTime = Timer::GetTimestamp();

	ZED::Log::Info("Tournament " + Filename + " saved successfully");
	return true;
}



Status Tournament::GetStatus() const
{
	if (IsReadonly())
		return Status::Concluded;
    
	auto guard = LockReadForScope();

	if (m_Schedule.size() == 0)
		return Status::Scheduled;

	bool one_match_finished   = false;
	bool all_matches_finished = true;

	auto schedule = GetSchedule();
	for (auto match : schedule)
	{
		if (match->IsEmptyMatch())
			continue;

		auto status = match->GetStatus();

		if (status == Status::Scheduled || status == Status::Optional)
			all_matches_finished = false;

		else if (status != Status::Concluded && status != Status::Skipped)
			all_matches_finished = false;
			
		else if (status != Status::Skipped)
			one_match_finished = true;
	}

	if (all_matches_finished && one_match_finished && !m_Name.empty())//Temporary tournaments can never conclude
		return Status::Concluded;
	if (one_match_finished)
		return Status::Running;
	return Status::Scheduled;
}



bool Tournament::CanCloseTournament() const
{
	auto guard = LockReadForScope();

	auto schedule = GetSchedule();
	for (auto match : schedule)
		if (match && match->IsRunning())
			return false;

	return true;
}



void Tournament::DeleteAllMatchResults()
{
	if (IsReadonly())
		return;

	auto guard = LockWriteForScope();

	auto schedule = GetSchedule();
	for (auto match : schedule)
		if (match)
	  {
		  match->m_State = Status::Scheduled;
		  match->GetLog().SetEvents().clear();
	  }
}



bool Tournament::AddMatch(Match* NewMatch)
{
	if (IsReadonly())
	{
		ZED::Log::Warn("Can not add a match to a tournament which is read only");
		return false;
	}

	if (!NewMatch)
	{
		ZED::Log::Error("Invalid match");
		return false;
	}
	/*if (GetStatus() == Status::Concluded)
	{
		ZED::Log::Warn("Can not add a match to a tournament which is finalized");
		return false;
	}*/

	if (NewMatch->GetFighter(Fighter::White) && NewMatch->GetFighter(Fighter::Blue) &&
		NewMatch->GetFighter(Fighter::White)->GetUUID() == NewMatch->GetFighter(Fighter::Blue)->GetUUID())
	{
		ZED::Log::Warn("White and blue fighters need to be different");
		return false;
	}

	//auto guard = LockWriteForScope();//TODO, check if respoinsible for crash

	//Do we have the match already?
	auto schedule = GetSchedule();
	for (auto match : schedule)
	{
		if (match && match->GetUUID() == NewMatch->GetUUID())
			return false;
	}

	auto guard = LockWriteForScope();//TODO, this should move above

	NewMatch->SetTournament(this);//If not yet associated

	if (NewMatch->GetMatID() == 0)//Not assigned to any mat?
		NewMatch->SetMatID(1);//Use mat ID 1 as the default

	//Do we have the rule set already?
	if (!m_StandingData.FindRuleSet(NewMatch->GetRuleSet().GetUUID()))
		m_StandingData.AddRuleSet(new RuleSet(NewMatch->GetRuleSet()));//Copy rule set

	auto dependencies = NewMatch->GetDependentMatches();//Does this match depend on any other match?

	for (auto prevMatch : dependencies)//For all dependencies
		if (prevMatch) AddMatch(const_cast<Match*>(prevMatch));//Include them as well recursively

	//If the match has judoka attached, include them as participants
	if (NewMatch->GetFighter(Fighter::White))
		AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::White)));
	if (NewMatch->GetFighter(Fighter::Blue))
		AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::Blue)));

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
	
	if (NewMatch->GetMatchTable())
	{
		AddMatchTable((MatchTable*)NewMatch->GetMatchTable());
		GenerateSchedule();
	}
	else
	{
		auto new_match_table = new CustomTable(this);
		new_match_table->SetFilter(new Fixed);
		new_match_table->AddMatch(NewMatch);
		new_match_table->SetMatID(NewMatch->GetMatID());
		AddMatchTable(new_match_table);
	}

	ScheduleSave();

	return true;
}



Match* Tournament::GetNextMatch(int32_t MatID) const
{
	auto guard = LockReadForScope();

	auto schedule = GetSchedule();
	for (auto match : schedule)
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
	auto guard = LockReadForScope();

	auto schedule = GetSchedule();
	for (; StartIndex < schedule.size(); StartIndex++)
	{
		if (schedule[StartIndex]->HasConcluded() || schedule[StartIndex]->IsRunning())
			continue;

		if (MatID < 0 || schedule[StartIndex]->GetMatID() == MatID)
		{
			StartIndex++;
			return schedule[StartIndex-1];
		}
	}

	return nullptr;
}



Match* Tournament::GetNextOngoingMatch(int32_t MatID)
{
	auto guard = LockReadForScope();

	auto schedule = GetSchedule();
	for (auto match : schedule)
	{
		if (!match || !match->IsRunning())
			continue;

		if (MatID < 0 || match->GetMatID() == MatID)
			return match;
	}

	return nullptr;
}



bool Tournament::ReviseMatch(const UUID& MatchID, IMat& Mat)
{
	auto match = FindMatch(MatchID);
	if (!match)
		return false;

	if (Mat.GetMatID() != match->GetMatID())
		return false;
	if (!match->HasConcluded())
		return false;

	if (Mat.AreFightersOnMat())
		return false;

	return Mat.StartMatch(match, true);
}



bool Tournament::RemoveMatch(const UUID& MatchID)
{
	if (IsReadonly())
		return false;
    
	auto guard = LockWriteForScope();

	if (GetStatus() == Status::Concluded)
		return false;

	for (auto it = m_MatchTables.begin(); it != m_MatchTables.end(); ++it)
	{
		if ((*it)->GetType() != MatchTable::Type::Custom)
			continue;

		if ((*it)->GetSchedule().size() == 1 && (*it)->GetSchedule()[0]->GetUUID() == MatchID)
		{
			it = m_MatchTables.erase(it);
			break;
		}
	}

	for (auto table : m_MatchTables)
		table->DeleteMatch(MatchID);

	auto schedule = GetSchedule();
	for (size_t i = 0; i < schedule.size(); ++i)
	{
		if (schedule[i]->GetUUID() == MatchID)
		{
			m_Schedule.erase(m_Schedule.begin() + i);
			break;
		}
	}

	ScheduleSave();
	return true;
}



std::vector<Match*> Tournament::GetSchedule() const
{
	std::vector<Match*> ret;

	auto guard = LockReadForScope();

	for (auto [table, index] : m_Schedule)
	{
		auto& sub_schedule = table->GetSchedule();
		if (index < sub_schedule.size())
			ret.emplace_back(sub_schedule[index]);
	}

	return ret;
}



Match* Tournament::FindMatch(const UUID& UUID) const
{
	auto guard = LockReadForScope();

	for (auto table : m_MatchTables)
		for (auto match : table->GetSchedule())
			if (match->GetUUID() == UUID)
				return match;
	/*for (auto match : m_Schedule)
		if (match && match->GetUUID() == UUID)
			return match;*/

	return nullptr;
}



bool Tournament::MoveMatchUp(const UUID& MatchID, uint32_t MatID)
{
	if (IsReadonly())
		return false;

	size_t prev_match_index = 0;
	size_t current_index = 0;

	auto guard = LockWriteForScope();

	auto schedule = GetSchedule();
	for (; current_index < schedule.size(); current_index++)
	{
		if (MatID != 0 && schedule[current_index]->GetMatID() != MatID)
			continue;
		if (schedule[current_index]->GetUUID() == MatchID)//CRASH HERE, MatchID points to invalid memory
			break;
		prev_match_index = current_index;
	}

	if (current_index == 0 || current_index == m_Schedule.size())
		return false;

	if (!schedule[prev_match_index] || !schedule[current_index])
		return false;

	//Don't move if both have concluded
	if (schedule[prev_match_index]->HasConcluded() && schedule[current_index]->HasConcluded())
		return false;

	//Swap matches

	auto prev_match = m_Schedule[prev_match_index];
	auto curr_match = m_Schedule[current_index];

	m_Schedule[prev_match_index] = curr_match;
	m_Schedule[current_index]    = prev_match;

	ScheduleSave();

	return true;
}



bool Tournament::MoveMatchDown(const UUID& MatchID, uint32_t MatID)
{
	if (IsReadonly())
		return false;

	size_t next_match_index = 0;
	size_t curr_match_index = 0;
	bool found = false;

	auto guard = LockWriteForScope();

	auto schedule = GetSchedule();
	for (size_t index = 0; index < m_Schedule.size(); index++)
	{
		if (MatID != 0 && schedule[index]->GetMatID() != MatID)
			continue;

		if (!found && schedule[index]->GetUUID() == MatchID)
		{
			curr_match_index = index;
			found = true;
		}
		else if (found)
		{
			next_match_index = index;
			break;
		}
	}

	if (!found || curr_match_index >= m_Schedule.size() || next_match_index >= m_Schedule.size() || next_match_index < curr_match_index)
		return false;

	if (!schedule[curr_match_index] || !schedule[next_match_index])
		return false;

	//Don't move if both have concluded
	if (schedule[curr_match_index]->HasConcluded() && schedule[next_match_index]->HasConcluded())
		return false;

	//Swap matches
	auto curr_match = m_Schedule[curr_match_index];
	auto next_match = m_Schedule[next_match_index];

	m_Schedule[curr_match_index] = next_match;
	m_Schedule[next_match_index] = curr_match;

	ScheduleSave();

	return true;
}



std::vector<Match> Tournament::GetNextMatches(int32_t MatID) const
{
	std::vector<Match> ret;

	auto guard = LockReadForScope();

	uint32_t id = 0;
	for (int i = 0; i < 3; i++)
	{
		auto nextMatch = GetNextMatch(MatID, id);

		if (nextMatch)
			ret.push_back(*nextMatch);
	}

	return ret;
}



bool Tournament::AddParticipant(Judoka* Judoka)
{
	if (IsReadonly())
		return false;
	if (!Judoka || IsParticipant(*Judoka))
		return false;

	auto guard = LockWriteForScope();

	//Do we have an organizer?
	if (m_Organizer)
	{
		//Check if the judoka is allowed to participate as in
		//his/her club belongs to this association

		auto club = Judoka->GetClub();
		if (!club)
		{
			ZED::Log::Info("Can not add a participant to a tournament without a club");
			return false;
		}

		if (!club->IsChildOf(*m_Organizer))
		{
			ZED::Log::Info("Participant can not be added, wrong tier");
			return false;
		}
	}

	if (!m_StandingData.AddJudoka(Judoka))
	{
		ZED::Log::Warn("Could not add judoka!");
		return false;
	}

	const bool club_added = m_StandingData.AddClub((Club*)Judoka->GetClub());

	FindAgeGroupForJudoka(*Judoka);

	bool added = false;
	for (auto table : m_MatchTables)
	{
		if (table && table->IsElgiable(*Judoka))
		{
			table->AddParticipant(Judoka);
			added = true;
		}
	}

	if (club_added)//New club got added
		PerformLottery();//Redo lottery

	if (added)
		GenerateSchedule();
	ScheduleSave();

	return true;
}



bool Tournament::RemoveParticipant(const UUID& UUID)
{
	if (IsReadonly())
		return false;
    
	auto guard = LockWriteForScope();

	const Judoka* deleted_judoka = m_StandingData.FindJudoka(UUID);

	if (!deleted_judoka)
		return false;

	m_StandingData.DeleteJudoka(UUID);

	for (auto table : m_MatchTables)
	{
		if (!table->IsIncluded(*deleted_judoka))
			continue;

		for (auto judoka : m_StandingData.GetAllJudokas())
		{
			if (table->IsElgiable(*judoka))
				table->AddParticipant(judoka);
		}
	}

	m_JudokaToAgeGroup.erase(deleted_judoka->GetUUID());

	GenerateSchedule();//Recalculate schedule

	return true;
}



uint32_t Tournament::GetHighestMatIDUsed() const
{
	uint32_t max = 0;

	auto guard = LockReadForScope();

	auto schedule = GetSchedule();
	for (auto match : schedule)
		if (match && match->GetMatID() > max)
			max = match->GetMatID();

	for (auto table : m_MatchTables)
		if (table && table->GetMatID() > max)
			max = table->GetMatID();

	return max;
}



bool Tournament::IsMatUsed(uint32_t ID) const
{
	auto guard = LockReadForScope();

	auto schedule = GetSchedule();
	for (const auto match : schedule)
		if (match && match->GetMatID() == ID)
			return true;

	return false;
}



void Tournament::SwapAllFighters()
{
	if (IsReadonly())
		return;

	auto guard = LockWriteForScope();

	for (auto match : GetSchedule())
	{
		if (!match->IsRunning())
			match->SwapFighters();
	}
}



MatchTable* Tournament::FindMatchTable(const UUID& ID)
{
	if (IsReadonly())
		return nullptr;

	auto guard = LockReadForScope();

	for (auto table : m_MatchTables)
	{
		if (!table)
			continue;

		if (table->GetUUID() == ID)
			return table;

		//Check sub table of pool
		if (table->GetType() == MatchTable::Type::Pool)
		{
			Pool* pool = (Pool*)table;

			for (size_t i = 0; i < pool->GetPoolCount(); ++i)
				if (*pool->GetPool(i) == ID)
					return (MatchTable*)pool->GetPool(i);

			if (pool->GetFinals() == ID)
				return (MatchTable*)&pool->GetFinals();
		}
	}

	return nullptr;
}



const MatchTable* Tournament::FindMatchTable(const UUID& ID) const
{
	auto guard = LockReadForScope();

	for (auto table : m_MatchTables)
	{
		if (!table)
			continue;

		if (table->GetUUID() == ID)
			return table;

		//Check sub table of pool
		if (table->GetType() == MatchTable::Type::Pool)
		{
			const Pool* pool = (const Pool*)table;

			for (size_t i = 0; i < pool->GetPoolCount(); ++i)
				if (*pool->GetPool(i) == ID)
					return pool->GetPool(i);

			if (pool->GetFinals() == ID)
				return &pool->GetFinals();
		}
	}

	return nullptr;
}



MatchTable* Tournament::FindMatchTableByName(const std::string& Name)
{
	if (IsReadonly())
		return nullptr;

	auto guard = LockReadForScope();

	for (auto table : m_MatchTables)
	{
		if (table && table->GetName() == Name)
			return table;
	}

	return nullptr;
}



MatchTable* Tournament::FindMatchTableByDescription(const std::string& Description)
{
	if (IsReadonly())
		return nullptr;

	auto guard = LockReadForScope();

	for (auto table : m_MatchTables)
	{
		if (table && table->GetDescription() == Description)
			return table;
	}

	return nullptr;
}



void Tournament::AddMatchTable(MatchTable* NewMatchTable)
{
	if (IsReadonly())
		return;
	if (!NewMatchTable)
		return;

	auto guard = LockWriteForScope();

	if (FindMatchTable(*NewMatchTable))
		return;//Nothing to add

	NewMatchTable->SetTournament(this);

	//Add all judoka of the match table to the tournament
	for (auto judoka : NewMatchTable->GetParticipants())
		AddParticipant(const_cast<Judoka*>(judoka));

	if (NewMatchTable->GetAgeGroup())
		AddAgeGroup(const_cast<AgeGroup*>(NewMatchTable->GetAgeGroup()));

	if (NewMatchTable->GetOwnRuleSet())
		AddRuleSet(const_cast<RuleSet*>(NewMatchTable->GetOwnRuleSet()));

	//Add all eligable participants to the match table
	for (auto judoka : m_StandingData.GetAllJudokas())
	{
		if (judoka && NewMatchTable->IsElgiable(*judoka))
			NewMatchTable->AddParticipant(judoka);
	}

	if (NewMatchTable->GetScheduleIndex() < 0)//No schedule index?
		NewMatchTable->SetScheduleIndex(GetFreeScheduleIndex());//Take the first empty slot

	//Inform new match table about the lottery result
	NewMatchTable->OnLotteryPerformed();

	//Find a new color for this match table
	Color match_table_color = Color::Name::Blue;
	for (auto table : m_MatchTables)
	{
		if (table->GetColor() >= match_table_color)
			match_table_color = table->GetColor() + 1;
	}
	NewMatchTable->SetColor(match_table_color);

	m_MatchTables.push_back(NewMatchTable);

	//Copy over matches
	for (size_t i = 0; i < NewMatchTable->GetSchedule().size(); i++)
	{
		auto match = NewMatchTable->GetSchedule()[i];
		if (!match->IsEmptyMatch())
			m_Schedule.emplace_back(NewMatchTable, i);
	}

	OnUpdateMatchTable(*NewMatchTable);
}



bool Tournament::OnUpdateParticipant(const UUID& UUID)
{
	if (IsReadonly())
		return false;
	if (GetStatus() != Status::Scheduled)
		return false;

	auto judoka = FindParticipant(UUID);

	if (!judoka)
		return false;

	auto guard = LockWriteForScope();

	//Is the current age group still fine?
	auto current_age_group = GetAgeGroupOfJudoka(judoka);
	if (current_age_group && !current_age_group->IsElgiable(*judoka, GetDatabase()))
	{
		m_JudokaToAgeGroup.erase(judoka->GetUUID());//Remove judoka to the age group he currently belongs to
		FindAgeGroupForJudoka(*judoka);//Try to find a new one
	}
	else if (!current_age_group)
		FindAgeGroupForJudoka(*judoka);//Try to find one

	for (auto table : m_MatchTables)
	{
		if (!table) continue;

		if (!table->IsElgiable(*judoka))//No longer eligable?
			table->RemoveParticipant(judoka);
		else//eligable?
			table->AddParticipant(judoka);
	}

	GenerateSchedule();

	return true;
}



bool Tournament::OnUpdateMatchTable(const UUID& UUID)
{
	if (IsReadonly())
		return false;
	if (GetStatus() == Status::Concluded)
		return false;

	auto guard = LockWriteForScope();

	auto matchTable = FindMatchTable(UUID);

	if (!matchTable)
		return false;

	if (matchTable->GetStatus() == Status::Scheduled)//Can safely recalculate the match table
	{
		auto participants = matchTable->GetParticipants();
		for (auto judoka : participants)
			if (judoka && !matchTable->IsElgiable(*judoka))//No longer eligable?
				matchTable->RemoveParticipant(judoka);

		for (auto judoka : m_StandingData.GetAllJudokas())
		{
			if (judoka && matchTable->IsElgiable(*judoka))
				matchTable->AddParticipant(judoka);
		}

		matchTable->GenerateSchedule();
	}

	//Optimize master schedule entries
	OrganizeMasterSchedule();

	//Sort
	std::sort(m_MatchTables.begin(), m_MatchTables.end(), [](auto a, auto b) {
		//Sort by filter
		if (a->GetFilter() && !b->GetFilter())
			return true;
		if (!a->GetFilter() && b->GetFilter())
			return false;

		/*if (a->GetFilter() && b->GetFilter() && a->GetFilter()->GetType() == IFilter::Type::Weightclass && b->GetFilter()->GetType() != IFilter::Type::Weightclass)
			return true;
		if (a->GetFilter() && b->GetFilter() && a->GetFilter()->GetType() != IFilter::Type::Weightclass && b->GetFilter()->GetType() == IFilter::Type::Weightclass)
			return false;*/

		//Both weightclasses?
		if (a->GetFilter() && b->GetFilter() && a->GetFilter()->GetType() == IFilter::Type::Weightclass && b->GetFilter()->GetType() == IFilter::Type::Weightclass)
		{
			auto weightclassA = (const Weightclass*)a->GetFilter();
			auto weightclassB = (const Weightclass*)b->GetFilter();

			//Sort by age group
			if (weightclassA->GetAgeGroup() && weightclassB->GetAgeGroup() && weightclassA->GetAgeGroup()->GetMinAge() != weightclassB->GetAgeGroup()->GetMinAge())
				return weightclassA->GetAgeGroup()->GetMinAge() < weightclassB->GetAgeGroup()->GetMinAge();

			//Sort by gender
			if (weightclassA->GetGender() != weightclassB->GetGender())
				return (int)weightclassA->GetGender() < (int)weightclassB->GetGender();

			//Sort by weight
			if (weightclassA->GetMinWeight() != weightclassB->GetMinWeight())
				return weightclassA->GetMinWeight() < weightclassB->GetMinWeight();
		}

		if (a->GetName() != b->GetName())
			return a->GetName() < b->GetName();

		return a->GetUUID() < b->GetUUID();
	});

	if (matchTable->GetScheduleIndex() != GetMaxScheduleIndex())//No need to rebuild schedule
		BuildSchedule();

	return true;
}



bool Tournament::RemoveMatchTable(const UUID& UUID)
{
	if (IsReadonly())
		return false;

	auto guard = LockWriteForScope();

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

	GenerateSchedule();

	return true;
}



bool Tournament::AddAgeGroup(AgeGroup* NewAgeGroup)
{
	if (IsReadonly())
		return false;
	if (!NewAgeGroup)
		return false;

	auto guard = LockWriteForScope();

	if (m_StandingData.FindAgeGroup(NewAgeGroup->GetUUID()))
		return false;

	if (!m_StandingData.AddAgeGroup(NewAgeGroup))
		return false;

	if (NewAgeGroup->GetRuleSet())
		m_StandingData.AddRuleSet(const_cast<RuleSet*>(NewAgeGroup->GetRuleSet()));

	for (auto judoka : m_StandingData.GetAllJudokas())
	{
		auto age_group = GetAgeGroupOfJudoka(judoka);

		//Not assigned to any age group and eligable for this new one?
		if (!age_group && NewAgeGroup->IsElgiable(*judoka, GetDatabase()))
		{
			//Add him to his new age group
			m_JudokaToAgeGroup.insert({ judoka->GetUUID(), NewAgeGroup->GetUUID() });
		}
	}

	ScheduleSave();
	return true;
}



bool Tournament::RemoveAgeGroup(const UUID& UUID)
{
	if (IsReadonly())
		return false;

	auto guard = LockWriteForScope();

	auto age_group_to_remove = m_StandingData.FindAgeGroup(UUID);

	if (!age_group_to_remove)
		return false;

	if (!m_StandingData.RemoveAgeGroup(UUID))
		return false;

	//Remove all assignments from this age group
	for (auto it = m_JudokaToAgeGroup.begin(); it != m_JudokaToAgeGroup.end();)
	{
		if (it->second == UUID)
			it = m_JudokaToAgeGroup.erase(it);//Remove the assignment
		else
			++it;
	}

	//Assign not-assigned judoka to a age group if possible
	for (auto judoka : m_StandingData.GetAllJudokas())
	{
		//Not assigned to any age group?
		if (judoka && !GetAgeGroupOfJudoka(judoka))
			FindAgeGroupForJudoka(*judoka);
	}

	ScheduleSave();
	return true;
}



bool Tournament::AssignJudokaToAgeGroup(const Judoka* Judoka, const AgeGroup* AgeGroup)
{
	if (IsReadonly())
		return false;

	if (!Judoka || !AgeGroup)
		return false;

	auto guard = LockWriteForScope();

	if (!IsParticipant(*Judoka))
		return false;

	if (!m_StandingData.FindAgeGroup(AgeGroup->GetUUID()))
		return false;

	if (!AgeGroup->IsElgiable(*Judoka, m_StandingData))
		return false;

	//Remove judoka to the age group he currently belongs to
	m_JudokaToAgeGroup.erase(Judoka->GetUUID());

	//Add him to his new age group
	m_JudokaToAgeGroup.insert({ Judoka->GetUUID(), AgeGroup->GetUUID() });

	ScheduleSave();
	return true;
}



std::vector<const AgeGroup*> Tournament::GetEligableAgeGroupsOfJudoka(const Judoka* Judoka) const
{
	std::vector<const AgeGroup*> ret;

	auto guard = LockReadForScope();

	for (auto age_group : m_StandingData.GetAgeGroups())
	{
		if (Judoka && age_group->IsElgiable(*Judoka, GetDatabase()))
			ret.emplace_back(age_group);
	}

	return ret;
}



std::vector<const AgeGroup*> Tournament::GetAgeGroups() const
{
	std::vector<const AgeGroup*> ret;

	auto guard = LockReadForScope();

	for (auto age_group : m_StandingData.GetAgeGroups())
		ret.emplace_back(age_group);

	return ret;
}



void Tournament::GetAgeGroupInfo(YAML::Emitter& Yaml, const AgeGroup* AgeGroup) const
{
	if (!AgeGroup)
		return;

	auto guard = LockReadForScope();

	auto rules = AgeGroup->GetRuleSet();
	if (rules)
	{
		Yaml << YAML::Key << "rules_name" << YAML::Value << rules->GetName();
		Yaml << YAML::Key << "rules_uuid" << YAML::Value << (std::string)rules->GetUUID();
	}

	size_t num_match_tables = 0;
	size_t num_matches = 0;
	for (auto match_table : m_MatchTables)
	{
		if (match_table->GetAgeGroup() && match_table->GetAgeGroup()->GetUUID() == AgeGroup->GetUUID())
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
		if (age_group_id == AgeGroup->GetUUID())
			num_participants++;
	}

	Yaml << YAML::Key << "num_participants" << YAML::Value << num_participants;
}



bool Tournament::MoveScheduleEntryUp(const UUID& UUID)
{
	if (IsReadonly())
		return false;
	if (GetStatus() == Status::Concluded)
		return false;

	auto guard = LockWriteForScope();

	uint32_t index = 0;
	for (; index < m_MatchTables.size(); ++index)
		if (m_MatchTables[index]->GetUUID() == UUID)
			break;

	if (index >= m_MatchTables.size() || !m_MatchTables[index])
		return false;

	if (m_MatchTables[index]->GetScheduleIndex() < 0)
		return false;

	if (m_MatchTables[index]->GetScheduleIndex() == 0)
	{//Special case, move everything else down
		for (uint32_t i = 0; i < m_MatchTables.size(); ++i)
			if (i != index)
				m_MatchTables[i]->SetScheduleIndex(m_MatchTables[i]->GetScheduleIndex() + 1);
	}
	else
		m_MatchTables[index]->SetScheduleIndex(m_MatchTables[index]->GetScheduleIndex() - 1);

	//Optimize master schedule entries
	OrganizeMasterSchedule();

	GenerateSchedule();
	return true;
}



bool Tournament::MoveScheduleEntryDown(const UUID& UUID)
{
	if (IsReadonly())
		return false;
	if (GetStatus() == Status::Concluded)
		return false;

	auto guard = LockWriteForScope();

	uint32_t index = 0;
	for (; index < m_MatchTables.size(); ++index)
		if (m_MatchTables[index]->GetUUID() == UUID)
			break;

	if (index >= m_MatchTables.size() || !m_MatchTables[index])
		return false;

	auto entry = m_MatchTables[index];

	//Is already at the last position and alone?
	if (entry->GetScheduleIndex() == GetMaxScheduleIndex() && GetMaxEntriesAtScheduleIndex(0, entry->GetScheduleIndex()) == 1)
		return false;

	if (entry->GetStatus() != Status::Scheduled)//Don't move if already started
		return false;

	m_MatchTables[index]->SetScheduleIndex(m_MatchTables[index]->GetScheduleIndex() + 1);

	//Optimize master schedule entries
	OrganizeMasterSchedule();

	GenerateSchedule();
	return true;
}



std::vector<WeightclassDescCollection> Tournament::GenerateWeightclasses(int Min, int Max, int Diff, const std::vector<const AgeGroup*>& AgeGroups, bool SplitGenders) const
{
	std::vector<WeightclassDescCollection> ret;

	auto guard = LockReadForScope();

	if (AgeGroups.empty())
	{
		for (Gender gender = Gender::Male; gender <= Gender::Female; ++gender)
		{
			if (gender == Gender::Female && !SplitGenders)
				continue;//Only do one loop for both genders

			std::vector<Weight> weights;

			for (const auto judoka : m_StandingData.GetAllJudokas())
			{
				//Filter for correct gender
				if (SplitGenders && judoka->GetGender() != gender)
					continue;

					weights.emplace_back(judoka->GetWeight());
			}

			if (weights.empty())
				continue;

			std::sort(weights.begin(), weights.end());

			Generator gen;
			gen.m_Min = Min;
			gen.m_Max = Max;
			gen.m_Diff = Diff;

			//Perform splitting algorithm
			auto result = gen.split(weights);

#ifdef _DEBUG
			std::string line;
			for (auto w : weights)
				line += w.ToString() + " ";
			ZED::Log::Debug(line);

			ZED::Log::Debug("Min:  " + std::to_string(Min));
			ZED::Log::Debug("Max:  " + std::to_string(Max));
			ZED::Log::Debug("Diff: " + std::to_string(Diff));

			for (auto r : result.m_Collection)
				ZED::Log::Info(r.m_Min.ToString() + " - " + r.m_Max.ToString() + "   #" + std::to_string(r.m_NumParticipants));
#endif

			//Add additional metadata
			if (SplitGenders)
				result.m_Gender = gender;

			ret.emplace_back(result);
		}
	}


	else
	{
		for (auto age_group : AgeGroups)
		{
			for (Gender gender = Gender::Male; gender <= Gender::Female; ++gender)
			{
				if (gender == Gender::Female && !SplitGenders)
					continue;//Only do one loop for both genders

				std::vector<Weight> weights;

				for (const auto judoka : m_StandingData.GetAllJudokas())
				{
					//Filter for correct gender
					if (SplitGenders && judoka->GetGender() != gender)
						continue;

					auto age_group_of_judoka = GetAgeGroupOfJudoka(judoka);
					if (age_group_of_judoka && age_group_of_judoka->GetUUID() == age_group->GetUUID())
						weights.emplace_back(judoka->GetWeight());
				}

				std::sort(weights.begin(), weights.end());

				Generator gen;
				gen.m_Min = Min;
				gen.m_Max = Max;
				gen.m_Diff = Diff;

				//Perform splitting algorithm
				auto result = gen.split(weights);

#ifdef _DEBUG
				std::string line;
				for (auto w : weights)
					line += w.ToString() + " ";
				ZED::Log::Debug(line);

				ZED::Log::Debug("Min:  " + std::to_string(Min));
				ZED::Log::Debug("Max:  " + std::to_string(Max));
				ZED::Log::Debug("Diff: " + std::to_string(Diff));

				for (auto r : result.m_Collection)
					ZED::Log::Info(r.m_Min.ToString() + " - " + r.m_Max.ToString() + "   #" + std::to_string(r.m_NumParticipants));
#endif

				//Add additional metadata
				result.m_AgeGroup = age_group;
				if (SplitGenders)
					result.m_Gender = gender;

				ret.emplace_back(result);
			}
		}
	}

	return ret;
}



bool Tournament::ApplyWeightclasses(const std::vector<WeightclassDescCollection>& Descriptors)
{
	if (IsReadonly())
		return false;

	auto guard = LockWriteForScope();

	//Remove old weightclasses
	for (const auto& desc : Descriptors)
	{
		if (desc.m_AgeGroup)
		{
			//Remove all weight classes with this age group
			auto temp_copy = m_MatchTables;//We are iterating while removing
			for (auto match_table : temp_copy)
			{
				if (match_table->GetType() == MatchTable::Type::RoundRobin &&
					match_table->GetAgeGroup() &&
					*match_table->GetAgeGroup() == *desc.m_AgeGroup)
					RemoveMatchTable(match_table->GetUUID());
			}
		}

		else//Not associated to an age group? Remove all weight classes
		{
			auto temp_copy = m_MatchTables;//We are iterating while removing
			for (auto match_table : temp_copy)
			{
				if (match_table->GetType() == MatchTable::Type::RoundRobin)
					RemoveMatchTable(match_table->GetUUID());
			}
		}
	}


	for (const auto& desc : Descriptors)
	{	
		for (auto weight_desc : desc.m_Collection)
		{
			auto new_weightclass = new RoundRobin(weight_desc.m_Min, weight_desc.m_Max, desc.m_Gender, this);

			new_weightclass->SetAgeGroup(desc.m_AgeGroup);
			new_weightclass->SetMatID(1);

			AddMatchTable(new_weightclass);
		}
	}

	ScheduleSave();

	return true;
}



bool Tournament::IsDisqualified(const Judoka& Judoka) const
{
	LockRead();

	bool ret = m_DisqualifiedJudoka.find(Judoka.GetUUID()) != m_DisqualifiedJudoka.cend();

	UnlockRead();

	return ret;
}



void Tournament::Disqualify(const Judoka& Judoka)
{
	if (IsReadonly())
		return;

	auto guard = LockWriteForScope();

	m_DisqualifiedJudoka.insert(Judoka.GetUUID());

	auto schedule = GetSchedule();
	for (auto match : schedule)
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
			if (match->GetWinner() && *match->GetWinner() == Judoka)
			{
				match->SetResult(Match::Result(Winner::Draw, Match::Score::Draw));
			}
		}
	}

	ScheduleSave();
}



void Tournament::RevokeDisqualification(const Judoka& Judoka)
{
	if (IsReadonly())
		return;
	if (!IsDisqualified(Judoka))
		return;

	auto guard = LockWriteForScope();

	m_DisqualifiedJudoka.erase(Judoka.GetUUID());

	auto schedule = GetSchedule();
	for (auto match : schedule)
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

	ScheduleSave();
}



bool Tournament::PerformLottery()
{
	if (IsReadonly())
		return false;
	if (GetStatus() != Status::Scheduled)
		return false;

	LockWrite();

	std::unordered_set<UUID> clubsforlottery;
	auto organizer_level = 5;//Default organizer level
	if (m_Organizer)
		organizer_level = m_Organizer->GetLevel();

	auto lottery_level = organizer_level + 1;//Default
	if (m_LotteryTier > 0 && (int)m_LotteryTier > organizer_level)//Is valid?
		lottery_level = m_LotteryTier;

	for (auto judoka : GetDatabase().GetAllJudokas())
	{
		const Association* club = judoka->GetClub();

		//Move up the tree till we are on the right level
		while (club && club->GetLevel() > lottery_level)
			club = club->GetParent();

		if (club && club->GetLevel() == lottery_level)
			clubsforlottery.insert(*club);
	}

	m_AssociationToLotNumber.clear();
	auto max_lot = clubsforlottery.size();
	std::random_device rd;

	for (size_t lot = 0; lot < max_lot; ++lot)
	{
		auto index = rd() % clubsforlottery.size();

		auto it = clubsforlottery.cbegin();
		for (size_t i = 0; i < index; ++i)
			++it;

		auto assoc = *it;
		m_AssociationToLotNumber.emplace_back(assoc, lot);
		clubsforlottery.erase(it);
	}

	std::sort(m_AssociationToLotNumber.begin(), m_AssociationToLotNumber.end(),
		[](const auto& a, const auto& b) { return a.second < b.second; });

	UnlockWrite();

	for (auto table : m_MatchTables)
		table->OnLotteryPerformed();

	GenerateSchedule();

	return true;
}



size_t Tournament::GetLotOfAssociation(const UUID& UUID) const
{
	auto guard = LockReadForScope();

	for (auto [assoc, lot] : m_AssociationToLotNumber)
		if (assoc == UUID)
			return lot;

	return SIZE_MAX;
}



const std::string Tournament::Schedule2String(bool ImportantOnly, int Mat) const
{
	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	LockRead();
	auto schedule = GetSchedule();
	Match* prev = nullptr;
	int serialized_matches = 0;

	for (auto match : schedule)
	{
		if (match)
		{
			if (Mat >= 1 && match->GetMatID() != Mat)
				continue;

			if (!ImportantOnly)//Default, serialize all matches
				match->ToString(ret);
			else//Serialize only the "important" matches
			{
				if ((serialized_matches == 0) && (match->IsRunning() || match->IsScheduled()))
				{
					if (prev)
						prev->ToString(ret);
					serialized_matches = 1;
				}

				if (serialized_matches >= 1 && serialized_matches <= 7)
				{
					match->ToString(ret);
					serialized_matches++;
				}
			}

			prev = match;
		}
	}
	UnlockRead();

	ret << YAML::EndSeq;
	return ret.c_str();
}



const std::string Tournament::MasterSchedule2String() const
{
	YAML::Emitter ret;
	ret << YAML::BeginMap;

	LockRead();

	const auto highest_matID = GetHighestMatIDUsed();
	ret << YAML::Key << "highest_mat_id" << YAML::Value << highest_matID;
	ret << YAML::Key << "max_index"      << YAML::Value << GetMaxScheduleIndex();

	ret << YAML::Key << "max_widths" << YAML::Value;
	ret << YAML::BeginSeq;

	for (uint32_t matID = 1; matID <= highest_matID; matID++)
	{
		int max = 0;
		for (int32_t index = 0; index <= GetMaxScheduleIndex(matID); index++)
		{
			int width = GetMaxEntriesAtScheduleIndex(matID, index);
			if (width > max)
				max = width;
		}

		ret << max;
	}

	ret << YAML::EndSeq;

	ret << YAML::Key << "master_schedule" << YAML::Value;
	ret << YAML::BeginSeq;

	for (int32_t index = 0; index <= GetMaxScheduleIndex(); index++)//For all times
	{
		ret << YAML::BeginSeq;

		for (uint32_t matID = 1; matID <= highest_matID; matID++)
		{
			uint32_t max = 0;
			for (int32_t i = 0; i <= GetMaxScheduleIndex(matID); i++)
				if (GetMaxEntriesAtScheduleIndex(matID, i) > max)
					max = GetMaxEntriesAtScheduleIndex(matID, i);

			std::vector<std::pair<uint32_t, MatchTable*>> entries;
			for (uint32_t it = 0; it < m_MatchTables.size(); ++it)//For all schedule entries
			{
				auto entry = m_MatchTables[it];

				if (!entry || entry->GetScheduleIndex() != index || entry->GetMatID() != matID)
					continue;

				entries.push_back({ it, entry });
			}

			ret << YAML::BeginMap;
			ret << YAML::Key << "max" << YAML::Value << max;
			ret << YAML::Key << "entries" << YAML::Value;
			ret << YAML::BeginSeq;

			for (auto [index, entry] : entries)
			{
				ret << YAML::BeginMap;
				ret << YAML::Key << "uuid"        << YAML::Value << (std::string)entry->GetUUID();
				ret << YAML::Key << "color"       << YAML::Value << entry->GetColor().ToHexString();
				ret << YAML::Key << "editable"    << YAML::Value << entry->IsEditable();
				ret << YAML::Key << "mat_id"      << YAML::Value << entry->GetMatID();
				ret << YAML::Key << "description" << YAML::Value << entry->GetDescription();
				ret << YAML::Key << "schedule_size"     << YAML::Value << entry->GetSchedule().size();
				ret << YAML::Key << "participant_count" << YAML::Value << entry->GetParticipants().size();
				ret << YAML::EndMap;
			}

			ret << YAML::EndSeq;
			ret << YAML::EndMap;
		}

		ret << YAML::EndSeq;
	}

	UnlockRead();

	ret << YAML::EndMap;//master schedule
	ret << YAML::EndMap;

	return ret.c_str();
}



void Tournament::FindAgeGroupForJudoka(const Judoka& Judoka)
{
	//Find age groups this judoka can belong to
	std::vector<AgeGroup*> EligableAgeGroups;
	for (auto age_group : m_StandingData.GetAgeGroups())
	{
		if (age_group && age_group->IsElgiable(Judoka, GetDatabase()))
			EligableAgeGroups.emplace_back(age_group);
	}

	//Only one choice?
	if (EligableAgeGroups.size() == 1)//Add judoka to age group
		m_JudokaToAgeGroup.insert({ Judoka.GetUUID(), EligableAgeGroups[0]->GetUUID() });
}



int32_t Tournament::GetFreeScheduleIndex(uint32_t Mat) const
{
	int32_t ret = 0;

	for (; true; ++ret)
	{
		bool is_free = true;

		for (auto entry : m_MatchTables)
		{
			if (entry && entry->GetScheduleIndex() == ret)
			{
				is_free = false;
				break;
			}
		}

		if (is_free)
			return ret;
	}

	return -1;
}



int32_t Tournament::GetMaxScheduleIndex(uint32_t Mat) const
{
	int32_t ret = -1;

	for (auto entry : m_MatchTables)
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

	for (auto entry : m_MatchTables)
	{
		if (entry && (ScheduleIndex < 0 || entry->GetScheduleIndex() == ScheduleIndex) )
			if (MatID == 0 || entry->GetMatID() == MatID)
				count++;
	}

	return count;
}



void Tournament::GenerateSchedule()
{
	if (IsReadonly())
		return;

	auto guard = LockWriteForScope();

	if (GetStatus() == Status::Scheduled)//Can safely recalculate all match tables?
	{
		for (auto table : m_MatchTables)
		{
			if (table)
				table->GenerateSchedule();
		}
	}

	OrganizeMasterSchedule();
	BuildSchedule();
}



void Tournament::OrganizeMasterSchedule()
{
	//Check if there is a schedule index that is not used
	//so that we can move match tables up

	for (int32_t index = 0; index <= GetMaxScheduleIndex(); index++)
	{
		//Is the previous index empty?
		if (index > 0 && GetMaxEntriesAtScheduleIndex(0, index-1) == 0)
		{
			for (auto table : m_MatchTables)
				if (table && table->GetScheduleIndex() == index)
					table->SetScheduleIndex(index-1);
		}
	}
}



void Tournament::BuildSchedule()
{
	auto guard = LockWriteForScope();

	m_Schedule.clear();

	//For all master schedule entries
	for (int32_t index = 0; index <= GetMaxScheduleIndex(); index++)
	{
		//std::vector<std::pair<uint32_t, std::vector<Match*>>> Plan;
		std::vector<std::pair<MatchTable*, size_t>> Plan;

		//For each match table at this index
		for (auto entry : m_MatchTables)
		{
			if (!entry || entry->GetScheduleIndex() != index)
				continue;

			//auto schedule = entry->GetSchedule();
			//Plan.push_back({ entry->GetRecommendedNumMatchesBeforeBreak(), schedule });
			Plan.push_back({ entry, 0 });
		}

		std::sort(Plan.begin(), Plan.end(), [](auto& a, auto& b) {
			return a.first->GetSchedule().size() > b.first->GetSchedule().size();
		});

		while (true)
		{
			bool done = true;

			for (auto& [table, index] : Plan)
			{
				auto num = table->GetRecommendedNumMatchesBeforeBreak();

				for (uint32_t i = 0; i < num;)
				{
					if (index < table->GetSchedule().size())
					{
						auto match = table->GetSchedule()[index];
						if (!match->IsEmptyMatch())
						{
							m_Schedule.push_back({ table, index });
							i++;
							done = false;
						}

						index++;
					}
					else
						break;
				}
			}

			if (done)
				break;
		}
	}

	ScheduleSave();
}