#include <cassert>
#include <array>
#include "md5.h"
#include "tournament.h"
#include "weightclass.h"
#include "single_elimination.h"
#include "double_elimination.h"
#include "pool.h"
#include "age_group.h"
#include "../ZED/include/log.h"
#include "../ZED/include/file.h"



using namespace Judoboard;



MD5::MD5(const std::string& Filename)
{
	ZED::File file(Filename);

	if (file)
		Parse(ZED::Blob(file));
	else
		ZED::Log::Warn("Could not open file " + Filename);
}



MD5::MD5(const Tournament& Tournament)
{
	m_Description = Tournament.GetName();
	m_FileDate = "Datum: 01.01." + std::to_string(Tournament.GetDatabase().GetYear());

	m_DateStart = "D" + std::to_string(Tournament.GetDatabase().GetYear()) + "-01-01-00:00:00:0000";
	m_DateEnd   = m_DateStart;

	std::unordered_map<UUID, int> UUID2ID;
	int id = 1;
	std::unordered_map<int, void*> ID2PTR;

	auto uuid2id = [&](const UUID& UUID) {
		auto it = UUID2ID.find(UUID);
		if (it != UUID2ID.end())
			return it->second;
		return -1;
	};

	auto id2ptr = [&](int id) -> void* {
		auto it = ID2PTR.find(id);
		if (it != ID2PTR.end())
			return it->second;
		return nullptr;
	};


	//Convert associations

	for (auto assoc : Tournament.GetDatabase().GetAllAssociations())
	{
		Association* new_assoc = new Association;

		new_assoc->ID = id++;
		new_assoc->Description = assoc->GetName();
		new_assoc->ShortName   = new_assoc->Description.substr(0, 5);
		new_assoc->Tier        = assoc->GetLevel() + 2;//International starts a 2 in MD5 files

		if (assoc->GetParent())
		{
			new_assoc->NextAsscociationID = uuid2id(assoc->GetParent()->GetUUID());
			new_assoc->NextAsscociation   = (Association*)id2ptr(new_assoc->NextAsscociationID);
		}
		else
		{
			new_assoc->NextAsscociationID = new_assoc->ID;
			new_assoc->NextAsscociation   = new_assoc;
		}

		m_Associations.emplace_back(new_assoc);
		UUID2ID.insert({ assoc->GetUUID(), id - 1 });
		ID2PTR.insert({ id - 1, new_assoc });

		//Check if we can find the organizer
		if ( (Tournament.GetOrganizer() && *Tournament.GetOrganizer() == *assoc) ||
		    (!Tournament.GetOrganizer() && assoc->GetLevel() == 0) )//No organizer, assoc should be 'international'
		{
			m_AssociationID      = new_assoc->ID;
			m_TierToDisplay      = new_assoc->Tier;
			m_AssociationLevelID = new_assoc->Tier + 1;
			m_LotteryTierID      = new_assoc->Tier + 1;//Set lottery to same tier as organizer
		}
	}

	//Convert clubs

	for (auto club : Tournament.GetDatabase().GetAllClubs())
	{
		Club* new_club = new Club;

		new_club->ID = id++;
		new_club->Name            = club->GetName();
		new_club->Name_ForSorting = club->GetName();

		m_Clubs.emplace_back(new_club);
		UUID2ID.insert({ club->GetUUID(), id - 1 });
		ID2PTR.insert({ id - 1, new_club });

		//Create relation table entries
		for (auto parent = club->GetParent(); parent ; parent = parent->GetParent())
		{
			RelationClubAssociation new_rel;

			new_rel.ClubID        = new_club->ID;
			new_rel.AssociationID = uuid2id(parent->GetUUID());
			new_rel.Tier          = parent->GetLevel() + 2;

			m_ClubRelations.emplace_back(new_rel);

			new_club->AssociationID = uuid2id(parent->GetUUID());
			new_club->Association   = (Association*)id2ptr(new_club->AssociationID);
		}

		//Check if we can find the organizer
		if (Tournament.GetOrganizer())
		{
			if (*Tournament.GetOrganizer() == *club)
			{
				m_AssociationID = new_club->ID;
				if (club->GetParent())
				{
					m_AssociationLevelID = club->GetParent()->GetLevel() + 3;
					m_TierToDisplay      = club->GetParent()->GetLevel() + 2;
				}
			}
		}
	}

	//Convert judoka

	for (auto judoka : Tournament.GetParticipants())
	{
		Participant* new_judoka = new Participant;

		new_judoka->ID = id++;
		new_judoka->Firstname = judoka->GetFirstname();
		new_judoka->Lastname  = judoka->GetLastname();
		new_judoka->WeightInGrams = (uint32_t)judoka->GetWeight();
		new_judoka->Birthyear     = judoka->GetBirthyear();
		if (new_judoka->Birthyear == 0)
			new_judoka->Birthyear = -1;
		//new_judoka->Rank = 1;//DEBUG
		new_judoka->GKParticipantID = new_judoka->ID;

		if (judoka->GetClub())
		{
			new_judoka->ClubID = uuid2id(judoka->GetClub()->GetUUID());
			new_judoka->Club   = (Club*)id2ptr(new_judoka->ClubID);

			assert(new_judoka->Club);
			if (new_judoka->Club)
				new_judoka->ClubFullname = new_judoka->Club->Name;

			new_judoka->AssociationShortname = new_judoka->ClubFullname.substr(0, 6);//6 characters only
		}

		//if (Tournament.GetStatus() != Status::Scheduled)//Tournament has started
			new_judoka->HasBeenWeighted = true;//Mark judoka as being weighted

		m_Participants.emplace_back(new_judoka);
		UUID2ID.insert({ judoka->GetUUID(), id - 1 });
		ID2PTR.insert({ id - 1, new_judoka });
	}

	for (auto age_group : Tournament.GetAgeGroups())
	{
		AgeGroup* new_age_group = new AgeGroup;

		new_age_group->ID = id++;
		new_age_group->Name         = age_group->GetName();
		new_age_group->Gender       = age_group->GetGender();

		if (age_group->GetMaxAge() == 0)
			new_age_group->MinBirthyear = Tournament.GetDatabase().GetYear() - 100;
		else
			new_age_group->MinBirthyear = Tournament.GetDatabase().GetYear() - age_group->GetMaxAge();

		new_age_group->MaxBirthyear = Tournament.GetDatabase().GetYear() - age_group->GetMinAge();

		m_AgeGroups.emplace_back(new_age_group);
		UUID2ID.insert({ age_group->GetUUID(), id - 1 });
		ID2PTR.insert({ id - 1, new_age_group });
	}

	//Convert match tables

	for (auto match_table : Tournament.GetMatchTables())
	{
		auto filter = match_table->GetFilter();
		if (!filter)
			continue;

		Weightclass* new_weightclass = nullptr;		

		if (filter->GetType() != IFilter::Type::Weightclass)
			continue;

		
		const auto weightclass = std::dynamic_pointer_cast<Judoboard::Weightclass>(filter);
		new_weightclass = new Weightclass;

		new_weightclass->WeightLargerThan          = (uint32_t)weightclass->GetMinWeight() / 1000;
		new_weightclass->WeightInGrammsLargerThan  = (uint32_t)weightclass->GetMinWeight() % 1000;
		new_weightclass->WeightSmallerThan         = (uint32_t)weightclass->GetMaxWeight() / 1000;
		new_weightclass->WeightInGrammsSmallerThan = (uint32_t)weightclass->GetMaxWeight() % 1000;
		

		if (match_table->GetType() == MatchTable::Type::RoundRobin)
			new_weightclass->FightSystemID = 16;//Round robin
		else if (match_table->GetType() == MatchTable::Type::SingleElimination)
		{
			const auto single_elimination = std::dynamic_pointer_cast<Judoboard::SingleElimination>(match_table);

			new_weightclass->FightSystemID = 19;
			if (match_table->GetParticipants().size() > 16)
				new_weightclass->FightSystemID = 20;

			new_weightclass->MatchForThirdPlace = single_elimination->IsThirdPlaceMatch();
			new_weightclass->MatchForFifthPlace = single_elimination->IsFifthPlaceMatch();
		}
		else if (match_table->GetType() == MatchTable::Type::DoubleElimination)
		{
			const auto double_elimination = std::dynamic_pointer_cast<Judoboard::DoubleElimination>(match_table);

			new_weightclass->FightSystemID = 1;
			//if (match_table->GetParticipants().size() > 16)
				//new_weightclass->FightSystemID = 2;

			new_weightclass->MatchForThirdPlace = double_elimination->IsThirdPlaceMatch();
			new_weightclass->MatchForFifthPlace = double_elimination->IsFifthPlaceMatch();
		}
		else if (match_table->GetType() == MatchTable::Type::Pool)
		{
			const auto pool = std::dynamic_pointer_cast<Judoboard::Pool>(match_table);

			new_weightclass->FightSystemID = 24;
			new_weightclass->MatchForThirdPlace = pool->IsThirdPlaceMatch();
			new_weightclass->MatchForFifthPlace = pool->IsFifthPlaceMatch();
		}
		else
		{
			ZED::Log::Error("Can not export match table!");
			continue;
		}

		new_weightclass->ID   = id++;
		new_weightclass->Date = m_DateStart;

		if (match_table->GetName().length() > 0)
			new_weightclass->Description = match_table->GetName();
		else
			new_weightclass->Description = match_table->GetDescription();


		if (match_table->HasConcluded())
			new_weightclass->Status = 4;//Completed
		else
			new_weightclass->Status = 3;//Scheduled

		if (match_table->GetAgeGroup())
		{
			new_weightclass->AgeGroupID = uuid2id(match_table->GetAgeGroup()->GetUUID());
			new_weightclass->AgeGroup   = (AgeGroup*)id2ptr(new_weightclass->AgeGroupID);
		}

		m_Weightclasses.emplace_back(new_weightclass);
		UUID2ID.insert({ match_table->GetUUID(), id - 1 });
		ID2PTR.insert({ id - 1, new_weightclass });

		//Convert participants
		for (auto judoka : match_table->GetParticipants())
		{
			Participant* md5_judoka = (Participant*)id2ptr(uuid2id(judoka->GetUUID()));
			if (md5_judoka)
			{
				md5_judoka->WeightclassID = uuid2id(match_table->GetUUID());
				md5_judoka->Weightclass   = (Weightclass*)id2ptr(md5_judoka->WeightclassID);

				md5_judoka->AgeGroupID = new_weightclass->AgeGroupID;
				md5_judoka->AgeGroup   = new_weightclass->AgeGroup;
			}
		}

		if (match_table->GetParticipants().empty())//match table doesn't have any participants yet
			new_weightclass->Status = 0;//input phase
		else if (match_table->HasConcluded())
		{
			//Convert results

			auto results = match_table->CalculateResults();
			int rank = 1;
			for (const auto& result : results)
			{
				Result new_result;
				new_result.Weightclass   = new_weightclass;
				new_result.WeightclassID = new_weightclass->ID;
				new_result.AgeGroup   = new_weightclass->AgeGroup;
				new_result.AgeGroupID = new_weightclass->AgeGroupID;

				if (result.Judoka)
					new_result.ParticipantID = uuid2id(result.Judoka->GetUUID());
				new_result.Participant = (Participant*)id2ptr(new_result.ParticipantID);
				new_result.PointsPlus  = result.Wins;
				new_result.ScorePlus   = result.Score;

				new_result.RankNo = rank++;
				new_result.RankID = id++;

				//new_result.Participant->Rank = new_result.RankID;

				if (result.Judoka)
					m_Results.emplace_back(new_result);
			}
		}
	}

	//Create relations table
	
	for (auto age_group : m_AgeGroups)
	{
		for (auto weightclass : m_Weightclasses)
		{
			int startNo = 1;

			for (auto judoka : m_Participants)
			{
				if (judoka->AgeGroupID != age_group->ID)
					continue;
				if (judoka->WeightclassID != weightclass->ID)
					continue;

				RelationParticipantMatchTable new_relation;
				new_relation.AgeGroupID    = age_group->ID;
				new_relation.WeightclassID = weightclass->ID;
				new_relation.ParticipantID = judoka->ID;
				new_relation.StartNo       = startNo++;

				m_Relations.emplace_back(new_relation);
			}
		}
	}

	//Convert lots
	for (auto [assoc_id, lot] : Tournament.GetLots())
	{
		Lottery new_lot;
		new_lot.AssociationID = uuid2id(assoc_id);
		new_lot.StartNo       = (int)lot;
		m_Lottery.push_back(new_lot);
	}

	//Convert matches

	for (auto table : Tournament.GetMatchTables())
		for (auto match : table->GetSchedule())
	{
		Match new_match;

		if (match->GetFighter(Fighter::White))
			new_match.WhiteID = uuid2id(match->GetFighter(Fighter::White)->GetUUID());
		if (match->GetFighter(Fighter::Blue))
			new_match.RedID = uuid2id(match->GetFighter(Fighter::Blue)->GetUUID());

		new_match.White = (Participant*)id2ptr(new_match.WhiteID);
		new_match.Red   = (Participant*)id2ptr(new_match.RedID);

		auto match_table = match->GetMatchTable();

		//Calculate match number
		if (match_table)
		{
			new_match.MatchNo = (int)match_table->FindMatchIndex(*match) + 1;

			if (match_table->GetType() == MatchTable::Type::SingleElimination)
			{
				auto table = (SingleElimination*)match_table;
				//16 system
				if (match_table->GetParticipants().size() > 8 && match_table->GetParticipants().size() <= 16)
				{
					if (!table->IsThirdPlaceMatch() && !table->IsFifthPlaceMatch())
					{
						//if (new_match.MatchNo == 15)
							//new_match.MatchNo = 19;
						if (match->GetTag().finals)
							new_match.MatchNo = 19;
					}

					else if (table->IsThirdPlaceMatch() && table->IsFifthPlaceMatch())
					{
						//if (new_match.MatchNo == 15)
							//;//wrong export
						//if (new_match.MatchNo == 16)
							//;//wrong export
						/*if (new_match.MatchNo == 17)//Fifth
							new_match.MatchNo = 21;
						if (new_match.MatchNo == 18)//Third
							new_match.MatchNo = 20;
						if (new_match.MatchNo == 19)//Final
							new_match.MatchNo = 19;*/

						Judoboard::Match::Tag fifth_final = Judoboard::Match::Tag::Fifth() && Judoboard::Match::Tag::Finals();
						if (match->GetTag() == fifth_final)
							new_match.MatchNo = 21;
						else if (match->GetTag().third)
							new_match.MatchNo = 20;
						else if (match->GetTag().finals)
							new_match.MatchNo = 19;
					}
				}

				//32 system
				if (match_table->GetParticipants().size() > 16 && match_table->GetParticipants().size() <= 32)
				{
					if (new_match.MatchNo >= 31)
						new_match.MatchNo = 37;
				}
			}

			else if (match_table->GetType() == MatchTable::Type::DoubleElimination)
			{
				auto de = (DoubleElimination*)match_table;
				if (de->GetWinnerBracket().FindMatch(*match))
					new_match.AreaID = 0;
				else
					new_match.AreaID = 1;
			}
		}

		if (match_table && match_table->GetAgeGroup())
		{
			new_match.Weightclass = FindWeightclass(uuid2id(match_table->GetAgeGroup()->GetUUID()),
													uuid2id(match_table->GetUUID()));
			if (new_match.Weightclass)
				new_match.WeightclassID = new_match.Weightclass->ID;

			new_match.AgeGroupID = uuid2id(match_table->GetAgeGroup()->GetUUID());
			new_match.AgeGroup   = (AgeGroup*)id2ptr(new_match.AgeGroupID);

			//Find start numbers
			new_match.StartNoWhite = FindStartNo(new_match.AgeGroupID, new_match.WeightclassID, new_match.WhiteID);
			new_match.StartNoRed   = FindStartNo(new_match.AgeGroupID, new_match.WeightclassID, new_match.RedID);
		}

		if (match->HasConcluded())
		{//Convert result
			new_match.Status = 3;//Completed

			if (match->GetResult().m_Winner == Winner::White)
			{
				new_match.WinnerID = new_match.WhiteID;
				new_match.LoserID  = new_match.RedID;
			}
			else if (match->GetResult().m_Winner == Winner::Blue)
			{
				new_match.WinnerID = new_match.RedID;
				new_match.LoserID  = new_match.WhiteID;
			}
			else
			{
				//TODO draw can not be converted
			}

			new_match.Result = 1;//Result is available
			new_match.ScoreWinner = (int)match->GetResult().m_Score;
			new_match.ScoreLoser  = 0;
			new_match.Time        = match->GetResult().m_Time / 1000;
		}
		else//Not concluded
		{
			new_match.Status = 1;//Ready
		}

		m_Matches.emplace_back(new_match);
	}

	m_NumClubs        = (int)m_Clubs.size();
	m_NumParticipants = (int)m_Participants.size();
	m_NumAssociations = (int)m_Associations.size();

	m_IsValid = true;
}



MD5::~MD5()
{
	for (auto association : m_Associations)
		delete association;
	for (auto club : m_Clubs)
		delete club;
	for (auto age_group : m_AgeGroups)
		delete age_group;
	for (auto weightclass : m_Weightclasses)
		delete weightclass;
	for (auto participant : m_Participants)
		delete participant;
}



bool MD5::Save(const std::string& Filename) const
{
	ZED::File file(Filename, true);

	if (!file)
	{
		ZED::Log::Warn("Could not open file " + Filename);
		return false;
	}

	auto Write_String = [&](const std::string& Line) {
		file.Write((uint8_t)Line.length());
		file.Write(Line);
	};

	auto Write_Line = [&](const std::string& Line) {
		file.Write((uint8_t)Line.length());
		file.Write(Line);
		file.Write((uint8_t)0x00);
	};

	auto Write_Int = [&](int32_t Num) {
		std::string line(std::to_string(Num));
		file.Write((uint8_t)line.length());
		file.Write(line);
		file.Write((uint8_t)0x00);
	};

	auto Write_IntRaw = [&](int64_t Num) {
		file.Write((uint8_t)Num);
		file.Write((uint8_t)0x00);
	};

	auto Write_0D0A = [&]() {
		const uint8_t data[] = { 0x0D, 0x0A };
		file.Write(data, 2);
	};

	auto Write_0D0A00 = [&]() {
		const uint8_t data[] = { 0x0D, 0x0A, 0x00 };
		file.Write(data, 3);
	};

	file.Write((uint8_t)0x00);

	Write_String("MMW98");
	Write_0D0A00();

	file.Write("3");
	file.Write((uint8_t)0x00);

	Write_String("Version 51");
	Write_0D0A00();

	Write_String(GetFileDate());
	Write_0D0A00();

	file.Write((uint8_t)0x00);
	Write_0D0A00();

	{
		Write_String("Turnier");
		Write_0D0A00();

		std::array rows{ "Bezeichnung", "VorzugsschemaPK", "Ort", "DatumVon", "DatumBis", "LosEbenePK", "VerbandPK", "VerbandEbenePK", "KuerzelEbenePK", "MAXJGJ", "KampfumPlatz3", "KampfUmPlatz5", "SportlicheLeitung", "AnzWeitermelden", "LOSVERFAHREN", "AktVereinPK", "AktTNPK", "AktVerbandPK", "Meldegeld", "Meldegelderhoeht", "JGJIgnoreNegativeUnterbew" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(1);//Number of columns

		Write_Line(GetDescription());
		Write_Int(m_SchemaID);
		Write_Line(m_Place);
		Write_Line(m_DateStart);
		Write_Line(m_DateEnd);
		Write_Int(m_LotteryTierID);
		Write_Int(m_AssociationID);
		Write_Int(m_AssociationLevelID);
		Write_Int(m_TierToDisplay);
		Write_Int(m_MAXJGJ);
		Write_Int(m_ThirdPlaceMatch ? 1 : 0);
		Write_Int(m_FifthPlaceMatch ? 1 : 0);
		Write_Line(m_SportAdministrator);
		Write_Int(m_NumOfRelays);
		Write_Int(m_LotteryProcess);
		Write_Int(m_NumClubs);
		Write_Int(m_NumParticipants);
		Write_Int(m_NumAssociations);
		Write_Int(m_Money);
		Write_Int(m_MoneyIncreased);
		Write_Int(m_IgnoreNegativeScores ? 0 : -1);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//Rank -> Points
		Write_String("Vereinsw");
		Write_0D0A00();

		std::array rows{ "PlatzPK", "Punkte" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_RankToPoints.size());//Number of columns

		for (auto& rank2points : m_RankToPoints)
		{
			Write_Int(rank2points.RankID);
			Write_Int(rank2points.Points);
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//Age groups
		Write_String("AktAltersgruppe");
		Write_0D0A00();

		std::array rows{ "AltersgruppePK", "Bezeichnung", "MinJahrgang", "MaxJahrgang", "Geschlecht", "Aufruecken", "Toleranz", "GewichtAnWaageAendern", "LosverfahrenPK", "Allkategorie", "Kata", "Meldegeld", "MeldegeldKata", "MeldegeldAllkategorie", "Meldegelderhoeht", "MeldegeldKataerhoeht", "MeldegeldAllkategorieerho", "Poolsystem", "AlleTNinErgebnisliste", "Mannschaft" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_AgeGroups.size());//Number of columns

		for (auto& age_group : m_AgeGroups)
		{
			Write_Int(age_group->ID);
			Write_Line(UTF8ToLatin1(age_group->Name));
			Write_Int(age_group->MinBirthyear);
			Write_Int(age_group->MaxBirthyear);
			Write_Line((age_group->Gender == Gender::Male) ? "m" : "w");
			Write_Int(age_group->MoveUp ? 1 : 0);
			Write_Int(age_group->Tolerance);
			Write_Int(age_group->ChangeWeightAtScale);
			Write_Int(age_group->LotterySchemaID);
			Write_Line(age_group->AllCategories ? "T" : "F");
			Write_Line(age_group->Kata ? "T" : "F");

			Write_Int(age_group->Money);
			Write_Int(age_group->MoneyKata);
			Write_Int(age_group->MoneyAllCategories);
			Write_Int(age_group->MoneyIncreased);
			Write_Int(age_group->MoneyKataIncreased);
			Write_Int(age_group->MoneyAllCategoriesIncreased);

			Write_Line(age_group->PoolSystem ? "T" : "F");
			Write_Line(age_group->AllParticipantsInResultTable ? "T" : "F");
			if (age_group->Team != -1)
				Write_Int(age_group->Team);
			else
				Write_Line("");
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//Weightclasses
		Write_String("AktGewichtsklasse");
		Write_0D0A00();

		std::array rows{ "AltersgruppePK", "GewichtsklassePK", "GewichtGroesser", "GewichtKleiner", "Bezeichnung", "Status", "WettkampfsystemPK", "WettkampfsystemTypPK", "KampfUmPlatz3", "KampfUmPlatz5", "Datum", "Weitermelden", "MaxJGJ", "Identifikation", "ForReference", "GewichtGroesserGramm", "GewichtKleinerGramm", "MaxVorgepoolt" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_Weightclasses.size());//Number of columns

		for (auto& weightclass : m_Weightclasses)
		{
			Write_Int(weightclass->AgeGroupID);
			Write_Int(weightclass->ID);

			if (weightclass->WeightLargerThan < 0)
				Write_Line("");
			else
				Write_Int(weightclass->WeightLargerThan);

			if (weightclass->WeightSmallerThan < 0)
				Write_Line("");
			else
				Write_Int(weightclass->WeightSmallerThan);

			Write_Line(UTF8ToLatin1(weightclass->Description));
			Write_Int(weightclass->Status);

			if (weightclass->FightSystemID < 0)
				Write_Line("");
			else
				Write_Int(weightclass->FightSystemID);

			Write_Int(weightclass->FightSystemTypeID);

			Write_Int(weightclass->MatchForThirdPlace ? -1 : 1);
			Write_Int(weightclass->MatchForFifthPlace ? -1 : 1);
			Write_Line(weightclass->Date);
			Write_Int(weightclass->Relay);
			Write_Int(weightclass->MaxJGJ);
			Write_Line(weightclass->Identifier);
			Write_Line(weightclass->ForReference);

			if (weightclass->WeightInGrammsLargerThan < 0)
				Write_Line("");
			else
				Write_Int(weightclass->WeightInGrammsLargerThan);
			if (weightclass->WeightInGrammsSmallerThan < 0)
				Write_Line("");
			else
				Write_Int(weightclass->WeightInGrammsSmallerThan);

			if (weightclass->MaxPooled < 0)
				Write_Line("");
			else
				Write_Int(weightclass->MaxPooled);
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//ImportVerband
		Write_String("ImportVerband");
		Write_0D0A00();

		std::array rows{ "VerbandPK", "EbenePK", "Bezeichnung", "Kuerzel", "Nummer", "NaechsteEbenePK", "Aktiv" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_Associations.size());//Number of columns

		for (auto& association : m_Associations)
		{
			Write_Int(association->ID);
			Write_Int(association->Tier);
			Write_Line(UTF8ToLatin1(association->Description));
			Write_Line(UTF8ToLatin1(association->ShortName));
			Write_Line(association->Number);
			Write_Int(association->NextAsscociationID);
			Write_Int(association->Active);
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//Verein
		Write_String("Verein");
		Write_0D0A00();

		std::array rows{ "VereinPK", "Sortierbezeichnung", "Bezeichnung", "Nachname", "Vorname", "Strasse", "Ort", "PLZ", "Telp", "Teld", "Handy", "email", "fax", "Vereinsnummer", "StatusAenderung" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_Clubs.size());//Number of columns

		for (auto& club : m_Clubs)
		{
			Write_Int(club->ID);
			Write_Line(UTF8ToLatin1(club->Name_ForSorting));
			Write_Line(UTF8ToLatin1(club->Name));

			Write_Line(UTF8ToLatin1(club->Representative_Lastname));
			Write_Line(UTF8ToLatin1(club->Representative_Firstname));
			Write_Line(UTF8ToLatin1(club->Representative_Street));
			Write_Line(UTF8ToLatin1(club->Representative_Place));
			Write_Line(club->Representative_ZipCode);
			Write_Line(club->Representative_TelPrivate);
			Write_Line(club->Representative_TelProfessional);
			Write_Line(club->Representative_TelMobil);
			Write_Line(club->Representative_Email);
			Write_Line(club->Representative_Fax);

			if (club->OfficialClubNo <= 0)
				Write_Line("");
			else
				Write_Int(club->OfficialClubNo);

			if (club->StatusChanged < 0)
				Write_Int(club->StatusChanged);
			else
				Write_Line("");
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//VereinVerband
		Write_String("VereinVerband");
		Write_0D0A00();

		std::array rows{ "VereinPK", "EbenePK", "VerbandPK" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_ClubRelations.size());//Number of columns

		for (auto& rel : m_ClubRelations)
		{
			Write_Int(rel.ClubID);
			Write_Int(rel.Tier);
			Write_Int(rel.AssociationID);			
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//AktLosschema
		Write_String("AktLosschema");
		Write_0D0A00();

		std::array rows{ "LosSchemaPK", "Bezeichnung", "VerbandPK", "EbenePK"};

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_LotterySchemas.size());//Number of columns

		for (auto& schema : m_LotterySchemas)
		{
			Write_Int(schema.ID);
			Write_Line(UTF8ToLatin1(schema.Description));
			Write_Int(schema.AssociationID);
			Write_Int(schema.TierID);			
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//AktLosschemaZeile
		Write_String("AktLosschemaZeile");
		Write_0D0A00();

		std::array rows{ "LosschemaPK", "PosPK", "VerbandPK", "PlatzPK"};

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_LotterySchemaLines.size());//Number of columns

		for (auto& schema : m_LotterySchemaLines)
		{
			Write_Int(schema.LotterySchemaID);
			Write_Int(schema.PositionID);
			Write_Int(schema.AssociationID);
			Write_Int(schema.RankID);
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//Teilnehmer
		Write_String("Teilnehmer");
		Write_0D0A00();

		std::array rows{ "TeilnehmerPK", "AltersgruppePK", "VereinPK", "Nachname", "Vorname", "GradPK", "GewichtsklassePK", "gewogen", "Geburtsjahr", "StartNR", "PlatzPK", "StatusAenderung", "REDAusgeschrieben", "REDKuerzel", "AllkategorieTNPK", "KataTNPK", "GKTNPK", "Meldegelderhoeht", "Gewichtgramm" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_Participants.size());//Number of columns

		for (auto& judoka : m_Participants)
		{
			Write_Int(judoka->ID);
			Write_Int(judoka->AgeGroupID);
			Write_Int(judoka->ClubID);
			Write_Line(UTF8ToLatin1(judoka->Lastname));
			Write_Line(UTF8ToLatin1(judoka->Firstname));
			Write_Int(judoka->Graduation);
			Write_Int(judoka->WeightclassID);
			Write_Line(judoka->HasBeenWeighted ? "x" : "");

			if (judoka->Birthyear < 0)
				Write_Line("");
			else
				Write_Int(judoka->Birthyear);

			if (judoka->StartNo < 0)
				Write_Line("");
			else
				Write_Int(judoka->StartNo);

			Write_Int(judoka->QualificationRank);
			Write_Line(judoka->StatusChanged ? "1" : "");
			Write_Line(UTF8ToLatin1(judoka->ClubFullname));
			Write_Line(UTF8ToLatin1(judoka->AssociationShortname));
			Write_Int(judoka->AllCategoriesParticipantID);
			Write_Int(judoka->KataParticipantID);
			Write_Int(judoka->GKParticipantID);
			Write_Line(judoka->MoneyIncreased ? "T" : "F");

			if (judoka->WeightInGrams < 0)
				Write_Line("");
			else
				Write_Int(judoka->WeightInGrams);
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//Lose
		Write_String("Lose");
		Write_0D0A00();

		std::array rows{ "VerbandPK", "LosNR" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_Lottery.size());//Number of columns

		for (auto& lot : m_Lottery)
		{
			Write_Int(lot.AssociationID);
			Write_Int(lot.StartNo);			
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//SystemZuordnung
		Write_String("SystemZuordnung");
		Write_0D0A00();

		std::array rows{ "AltersgruppePK", "GewichtsklassePK", "StartNR", "TeilnehmerPK" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_Relations.size());//Number of columns

		for (auto& rel : m_Relations)
		{
			Write_Int(rel.AgeGroupID);
			Write_Int(rel.WeightclassID);
			Write_Int(rel.StartNo);
			Write_Int(rel.ParticipantID);
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//Fortfuehrung
		Write_String("Fortfuehrung");
		Write_0D0A00();

		std::array rows{ "AltersgruppePK", "GewichtsklassePK", "KampfNR", "StartNRRot", "RotPK", "RotAusKampfNR", "RotTyp", "StartNRWeiss", "WeissPK", "WeissAusKampfNR", "WeissTyp", "SiegerPK", "SiegerKampfNR", "SiegerFarbe", "VerliererPK", "VerliererKampfNR", "VerliererFarbe", "WarteAufSiegerAusKampf", "Zeit", "Bewertung", "UnterbewertungSieger", "UnterbewertungVerlierer", "Status", "RotAusgeschiedenKampfNR", "WeissAusgeschiedenKampfNR", "Pool", "DritterKampfNR", "DritterFarbe", "BereichPK" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_Matches.size());//Number of columns

		for (auto& match : m_Matches)
		{
			Write_Int(match.AgeGroupID);
			Write_Int(match.WeightclassID);
			Write_Int(match.MatchNo);

			Write_Int(match.StartNoRed);
			Write_Int(match.RedID);
			if (match.RedFromMatch < 0)
				Write_Line("");
			else
				Write_Int(match.RedFromMatch);
			Write_Int(match.RedTyp);

			Write_Int(match.StartNoWhite);
			Write_Int(match.WhiteID);
			if (match.WhiteFromMatch < 0)
				Write_Line("");
			else
				Write_Int(match.WhiteFromMatch);
			Write_Int(match.WhiteTyp);

			if (match.WinnerID < 0)
				Write_Line("");
			else
				Write_Int(match.WinnerID);
			if (match.WinnerMatchNo < 0)
				Write_Line("");
			else
				Write_Int(match.WinnerMatchNo);
			Write_Int(match.WinnerColor);

			if (match.LoserID < 0)
				Write_Line("");
			else
				Write_Int(match.LoserID);
			if (match.LoserMatchNo < 0)
				Write_Line("");
			else
				Write_Int(match.LoserMatchNo);
			Write_Int(match.LoserColor);

			Write_Int(match.WaitingForWinnerFromMatch);

			if (match.Time < 0)
				Write_Line("");
			else
				Write_Int(match.Time);

			if (match.Result < 0)
				Write_Line("");
			else
				Write_Int(match.Result);

			if (match.ScoreWinner < 0)
				Write_Line("");
			else
				Write_Int(match.ScoreWinner);

			if (match.ScoreLoser < 0)
				Write_Line("");
			else
				Write_Int(match.ScoreLoser);

			Write_Int(match.Status);

			if (match.RedOutMatchNo < 0)
				Write_Line("");
			else
				Write_Int(match.RedOutMatchNo);

			if (match.WhiteOutMatchNo < 0)
				Write_Line("");
			else
				Write_Int(match.WhiteOutMatchNo);

			Write_Int(match.Pool);
			Write_Int(match.ThirdMatchNo);
			Write_Int(match.ThirdColor);
			Write_Int(match.AreaID);
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	{//Ergebnis
		Write_String("Ergebnis");
		Write_0D0A00();

		std::array rows{ "AltersgruppePK", "GewichtsklassePK", "PlatzPK", "Pool", "PlatzNR", "KampfNR", "Platztyp", "TeilnehmerPK", "PunktePl", "PunkteMi", "UnterbewertungPl", "UnterbewertungMi", "Weitermelden", "AusPool" };

		Write_IntRaw(rows.size());//Number of rows

		for (auto& row : rows)
			Write_Line(row);

		file.Seek(-1);//Delete last \0
		Write_0D0A00();

		Write_IntRaw(m_Results.size());//Number of columns

		for (auto& result : m_Results)
		{
			Write_Int(result.AgeGroupID);
			Write_Int(result.WeightclassID);
			Write_Int(result.RankID);

			Write_Int(result.Pool);
			Write_Int(result.RankNo);
			if (result.MatchNo < 0)
				Write_Line("");
			else
				Write_Int(result.MatchNo);
			if (result.RankType < 0)
				Write_Line("");
			else
				Write_Int(result.RankType);

			Write_Int(result.ParticipantID);

			if (result.PointsPlus < 0)
				Write_Line("");
			else
				Write_Int(result.PointsPlus);

			if (result.PointsMinus < 0)
				Write_Line("");
			else
				Write_Int(result.PointsMinus);

			if (result.ScorePlus < 0)
				Write_Line("");
			else
				Write_Int(result.ScorePlus);

			if (result.ScoreMinus < 0)
				Write_Line("");
			else
				Write_Int(result.ScoreMinus);

			Write_Int(result.Relay);
			Write_Int(result.FromPool);		
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	Write_String("\\\\end");

	return true;
}



MD5::Association* MD5::FindAssociation(int AssociationID)
{
	if (AssociationID <= -1)
		return nullptr;

	for (auto association : m_Associations)
		if (association && association->ID == AssociationID)
			return association;
	return nullptr;
}



MD5::Association* MD5::FindAssociation(int AssociationID) const
{
	if (AssociationID <= -1)
		return nullptr;

	for (auto association : m_Associations)
		if (association && association->ID == AssociationID)
			return association;
	return nullptr;
}



MD5::Association* MD5::FindAssociationByName(const std::string& Name) const
{
	for (auto association : m_Associations)
		if (association && association->Description == Name)
			return association;
	return nullptr;
}



MD5::Club* MD5::FindClub(int ClubID)
{
	if (ClubID <= -1)
		return nullptr;

	for (auto club : m_Clubs)
		if (club && club->ID == ClubID)
			return club;
	return nullptr;
}



MD5::Club* MD5::FindClubByName(const std::string& Name) const
{
	for (auto club : m_Clubs)
		if (club && club->Name == Name)
			return club;
	return nullptr;
}



MD5::Participant* MD5::FindParticipant(int ParticipantID)
{
	if (ParticipantID <= 0)
		return nullptr;

	for (auto judoka : m_Participants)
		if (judoka && judoka->ID == ParticipantID)
			return judoka;
	return nullptr;
}



std::vector<const MD5::Participant*> MD5::FindParticipantsOfWeightclass(int AgeGroupID, int WeightclassID) const
{
	std::vector<const MD5::Participant*> ret;

	for (auto judoka : m_Participants)
		if (judoka && judoka->AgeGroupID == AgeGroupID && judoka->WeightclassID == WeightclassID)
			ret.push_back(judoka);
	return ret;
}



std::vector<MD5::Match> MD5::FindMatchesOfWeightclass(int AgeGroupID, int WeightclassID) const
{
	std::vector<MD5::Match> ret;

	for (const auto& match : m_Matches)
		if (match.AgeGroupID == AgeGroupID && match.WeightclassID == WeightclassID)
			ret.push_back(match);
	return ret;
}



std::vector<MD5::Match> MD5::FindMatchesOfWeightclass(const std::string& AgeGroup, const std::string& Weightclass) const
{
	std::vector<MD5::Match> ret;

	for (const auto& match : m_Matches)
		if (match.AgeGroup && match.AgeGroup->Name == AgeGroup && match.Weightclass && match.Weightclass->Description == Weightclass)
			ret.push_back(match);
	return ret;
}



MD5::AgeGroup* MD5::FindAgeGroup(int AgeGroupID)
{
	if (AgeGroupID <= -1)
		return nullptr;

	for (auto age_group : m_AgeGroups)
		if (age_group && age_group->ID == AgeGroupID)
			return age_group;
	return nullptr;
}



MD5::Weightclass* MD5::FindWeightclass(int AgeGroupID, int WeightclassID)
{
	if (WeightclassID <= -1)
		return nullptr;

#ifdef _DEBUG
	bool found = false;
	for (auto weightclass : m_Weightclasses)
	{
		if (weightclass && weightclass->AgeGroupID == AgeGroupID && weightclass->ID == WeightclassID)
		{
			if (found)
				assert(false);//Crash when the ID exists twice
			found = true;
		}
	}
#endif

	for (auto weightclass : m_Weightclasses)
		if (weightclass && weightclass->AgeGroupID == AgeGroupID && weightclass->ID == WeightclassID)
			return weightclass;
	return nullptr;
}



const MD5::Weightclass* MD5::FindWeightclass(const std::string& AgeGroup, const std::string& Weightclass) const
{
	for (auto table : m_Weightclasses)
		if (table->AgeGroup && table->AgeGroup->Name == AgeGroup && table->Description == Weightclass)
			return table;
	return nullptr;
}



const MD5::Result* MD5::FindResult(int AgeGroupID, int WeightclassID, int Rank) const
{
	assert(Rank >= 1);
	for (auto& result : m_Results)
		if (result.AgeGroupID == AgeGroupID && result.WeightclassID == WeightclassID && result.RankNo == Rank)
			return &result;
	return nullptr;
}



const MD5::Result* MD5::FindResult(const std::string& AgeGroup, const std::string& Weightclass, int Rank) const
{
	assert(Rank >= 1);
	for (auto& result : m_Results)
		if (result.AgeGroup && result.AgeGroup->Name == AgeGroup && result.Weightclass && result.Weightclass->Description == Weightclass && result.RankNo == Rank)
			return &result;
	return nullptr;
}



std::vector<const MD5::Result*> MD5::FindResults(int AgeGroupID, int WeightclassID) const
{
	std::vector<const Result*> ret;
	for (int rank = 1; auto result = FindResult(AgeGroupID, WeightclassID, rank); rank++)
		ret.emplace_back(result);
	return ret;
}



int MD5::FindStartNo(int AgeGroupID, int WeightclassID, int ParticipantID) const
{
	for (auto relation : m_Relations)
	{
		if (relation.AgeGroupID == AgeGroupID && relation.WeightclassID == WeightclassID &&
			relation.ParticipantID == ParticipantID)
			return relation.StartNo;
	}

	return -1;
}



int MD5::FindLotOfAssociation(int AssociationID) const
{
	for (auto lot : m_Lottery)
	{
		if (lot.AssociationID == AssociationID)
			return lot.StartNo;
	}

	return -1;
}



void MD5::Dump() const
{
	//Dump associations
	ZED::Log::Info("--- Associations ---");
	for (auto assoc : m_Associations)
	{
		std::string line = std::to_string(assoc->ID) + "   " + assoc->Description + "  ->  " + std::to_string(assoc->NextAsscociationID) + " Tier: " + std::to_string(assoc->Tier);

		ZED::Log::Info(line);
	}


	//Dump relation
	ZED::Log::Info("\n\n--- Club -> Association ---");
	for (auto rel : m_ClubRelations)
	{
		std::string line = std::to_string(rel.ClubID) + "  ->  " + std::to_string(rel.AssociationID) + "  Tier:  " + std::to_string(rel.Tier);

		ZED::Log::Info(line);
	}

	//Dump clubs
	ZED::Log::Info("\n\n--- Clubs ---");
	for (auto club : m_Clubs)
	{
		std::string line;

		if (!club->Association)
			line = club->Name + "  ->  " + std::to_string(club->AssociationID) + "  Name:  NONE!!!";
		else
			line = club->Name + "  ->  " + std::to_string(club->AssociationID) + "  Name:  " + club->Association->Description;

		ZED::Log::Info(line);
	}


	ZED::Log::Info("\n\n--- Judokas ---");
	for (auto judoka : m_Participants)
	{
		std::string line;
		line += judoka->Firstname + " " + judoka->Lastname;
		line += "   ID: "      + std::to_string(judoka->ID);
		line += "   StartNo: " + std::to_string(judoka->StartNo);
		ZED::Log::Info(line);
	}

	//Dump weightclasses
	ZED::Log::Info("\n\n--- Weightclasses ---");
	for (auto table : m_Weightclasses)
	{
		std::string line = table->Description;
		line += "   FightSystemID: "        + std::to_string(table->FightSystemID);
		line += "   FightSystemTypeID: "    + std::to_string(table->FightSystemTypeID);
		line += "   AgeGroupID: "           + std::to_string(table->AgeGroupID);
		line += "   "    + table->Description;
		ZED::Log::Info(line);
	}

	ZED::Log::Info("\n\n--- Matches ---");
	for (const auto& match : m_Matches)
	{
		std::string line;
		line += "   RedID: "         + std::to_string(match.RedID);
		line += "   WhiteID: "       + std::to_string(match.WhiteID);
		line += "   StartNoRed: "    + std::to_string(match.StartNoRed);
		line += "   StartNoWhite: "  + std::to_string(match.StartNoWhite);
		line += "   Status: "        + std::to_string(match.Status);
		line += "   Pool: "          + std::to_string(match.Pool);
		line += "   AreaID: "        + std::to_string(match.AreaID);
		line += "   MatchNo: "       + std::to_string(match.MatchNo);
		line += "   WeightclassID: " + std::to_string(match.WeightclassID);
		line += "   AgeGroupID: "    + std::to_string(match.AgeGroupID);
		ZED::Log::Info(line);
	}

	//Dump results
	ZED::Log::Info("\n\n--- Results ---");
	for (auto age_group : m_AgeGroups)
	{
		std::string separator(age_group->Name.length(), '-');

		ZED::Log::Info(separator);
		ZED::Log::Info(age_group->Name);
		ZED::Log::Info(separator);


		for (auto weightclass : m_Weightclasses)
		{
			if (weightclass->AgeGroupID != age_group->ID)
				continue;
			if (weightclass->ID == -1)
				continue;

			auto results = FindResults(weightclass->AgeGroupID, weightclass->ID);

			if (results.empty())
				continue;

			ZED::Log::Info(weightclass->Description);

			for (auto result : results)
			{
				std::string line = std::to_string(result->RankNo) + ". Place\t";
				if (result->Participant)
				{
					std::string name = result->Participant->Firstname + " " + result->Participant->Lastname;
					line += name;

					size_t tab_count = 3;
					tab_count -= name.length() / 8;

					for (size_t i = 0; i < tab_count; i++)
						line += "\t";

					line += std::to_string(result->Participant->Birthyear);
					if (result->Participant->Club)
						line += "\t" + result->Participant->Club->Name;

					line += "\tMatchNo: " + std::to_string(result->MatchNo);
				}

				ZED::Log::Info(line);
			}
		}
	}
}



bool MD5::Parse(ZED::Blob&& Data)
{
	if (Data.ReadByte() != 0x00)
	{
		ZED::Log::Warn("Data is not a MD5 file");
		return false;
	}

	if (ReadLine(Data) != "MMW98")
	{
		ZED::Log::Warn("Data is not a MD5 file");
		return false;
	}

	int unknown = Data.ReadByte();//'3' in MD5, 'F' in MD7

	if (unknown != 0x33 && unknown != 0x46)
	{
		ZED::Log::Warn("Data is not a MD5 file");
		return false;
	}

	if (Data.ReadByte() != 0x00)
	{
		ZED::Log::Warn("Data is not a MD5 file");
		return false;
	}

	auto version = ReadLine(Data);
	if (version != "Version 51" && version != "Version 70")
	{
		ZED::Log::Warn("Data is not a MD5 file");
		return false;
	}

	m_FileDate = ReadLine(Data);

	if (ReadLine(Data) != "")
	{
		ZED::Log::Warn("Data is not a MD5 file");
		return false;
	}

	bool is_ok = true;
	bool found_end = false;

	while (!Data.EndReached() && is_ok)
	{
		bool newline;
		auto line = ReadLine(Data, &newline);

		if (line == "Turnier")
			is_ok &= ReadTournamentData(Data);
		else if (line == "Vereinsw")
			is_ok &= ReadRankScore(Data);
		else if (line == "AktAltersgruppe")
			is_ok &= ReadAgeGroups(Data);
		else if (line == "AktGewichtsklasse")
			is_ok &= ReadWeightclasses(Data);
		else if (line == "ImportVerband")
			is_ok &= ReadAssociation(Data);
		else if (line == "Verein")
			is_ok &= ReadClubs(Data);
		else if (line == "VereinVerband")
			is_ok &= ReadRelationClubAssociation(Data);
		else if (line == "AktLosschema")
			is_ok &= ReadLotteryScheme(Data);
		else if (line == "AktLosschemaZeile")
			is_ok &= ReadLotterySchemaLine(Data);
		else if (line == "Teilnehmer")
			is_ok &= ReadParticipants(Data);
		else if (line == "Lose")
			is_ok &= ReadLottery(Data);
		else if (line == "SystemZuordnung")
			is_ok &= ReadRelationParticipantMatchTable(Data);
		else if (line == "Fortfuehrung")
			is_ok &= ReadMatchData(Data);
		else if (line == "Ergebnis")
			is_ok &= ReadResult(Data);

		//For Version 7
		else if (line == "PlanungsElement")
			is_ok &= ReadTable(Data);
		else if (line == "Pause")
			is_ok &= ReadTable(Data);
		else if (line == "InfoSeite")
			is_ok &= ReadTable(Data);
		else if (line == "Kampflog")
			is_ok &= ReadTable(Data);
		else if (line == "Matte")
			is_ok &= ReadTable(Data);
		
		else if (line == "\\\\end")
		{
			found_end = true;
			break;
		}
		else
		{
			ZED::Log::Warn("Unknown chunk: " + line);
			is_ok = false;
			assert(is_ok);
			break;
		}

		assert(is_ok);
	}

	if (!found_end)//Did we find the \\end tag at the end of the file?
		is_ok = false;

	if (is_ok)
	{
		//Resolve dependencies
		for (auto association : m_Associations)
		{
			association->NextAsscociation = FindAssociation(association->NextAsscociationID);
		}

		//Resolve club -> association connection
		for (auto rel : m_ClubRelations)
		{
			auto club  = FindClub(rel.ClubID);
			auto assoc = FindAssociation(rel.AssociationID);

			if (club && assoc)
			{
				if (club->Association)//Already connected?
				{
					//Reconnect if lower tier
					if (assoc->Tier < club->Association->Tier)
						continue;//Skip
				}

				club->AssociationID = assoc->ID;
				club->Association   = assoc;
			}
		}

		for (auto& age_group : m_AgeGroups)
		{
			//weightclass->AgeGroup = FindAgeGroup(weightclass->AgeGroupID);
		}

		for (auto& weightclass : m_Weightclasses)
		{
			//Doing it like this is actually wrong, since weightclasses for different age groups are actually NOT saved separateley!!
			weightclass->AgeGroup = FindAgeGroup(weightclass->AgeGroupID);
		}

		for (auto& judoka : m_Participants)
		{
			judoka->AgeGroup    = FindAgeGroup(judoka->AgeGroupID);
			judoka->Weightclass = FindWeightclass(judoka->AgeGroupID, judoka->WeightclassID);
			judoka->Club        = FindClub(judoka->ClubID);
		}

		for (auto& match : m_Matches)
		{
			match.AgeGroup    = FindAgeGroup(match.AgeGroupID);
			match.Weightclass = FindWeightclass(match.AgeGroupID, match.WeightclassID);

			match.White = FindParticipant(match.WhiteID);
			match.Red   = FindParticipant(match.RedID);
		}

		for (auto& result : m_Results)
		{
			result.Participant = FindParticipant(result.ParticipantID);
			result.AgeGroup    = FindAgeGroup(result.AgeGroupID);
			result.Weightclass = FindWeightclass(result.AgeGroupID, result.WeightclassID);
		}
	}

	m_IsValid = is_ok;

	assert(is_ok);
	return is_ok;
}



bool MD5::ReadTournamentData(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		auto Line = ReadLine(Data);

		data.emplace_back(RemoveControlCharacters(Line));

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			for (size_t i = 0; i < header.size(); i++) //-V1008
			{
				if (header[i] == "Bezeichnung")
					m_Description = Latin1ToUTF8(data[i]);
				else if (header[i] == "VorzugsschemaPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_SchemaID) != 1)
						ZED::Log::Warn("Could not parse schema id");
				}
				else if (header[i] == "Ort")
					m_Place = Latin1ToUTF8(data[i]);
				else if (header[i] == "DatumVon")
					m_DateStart = data[i];
				else if (header[i] == "DatumBis")
					m_DateEnd = data[i];
				else if (header[i] == "LosEbenePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_LotteryTierID) != 1)
						ZED::Log::Warn("Could not parse schema LotteryLevelID");
				}
				else if (header[i] == "VerbandPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_AssociationID) != 1)
						ZED::Log::Warn("Could not parse schema AssociationID");
				}
				else if (header[i] == "VerbandEbenePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_AssociationLevelID) != 1)
						ZED::Log::Warn("Could not parse schema AssociationLevelID");
				}
				else if (header[i] == "KuerzelEbenePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_TierToDisplay) != 1)
						ZED::Log::Warn("Could not parse schema LevelShortID");
				}
				else if (header[i] == "MAXJGJ")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_MAXJGJ) != 1)
						ZED::Log::Warn("Could not parse schema MAXJGJ");
				}
				else if (header[i] == "KampfumPlatz3")
					m_ThirdPlaceMatch = data[i] == "1";
				else if (header[i] == "KampfUmPlatz5")
					m_FifthPlaceMatch = data[i] == "1";
				else if (header[i] == "SportlicheLeitung")
					m_SportAdministrator = Latin1ToUTF8(data[i]);
				else if (header[i] == "AnzWeitermelden")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_NumOfRelays) != 1)
						ZED::Log::Warn("Could not parse schema NumOfRelays");
				}
				else if (header[i] == "LOSVERFAHREN")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_LotteryProcess) != 1)
						ZED::Log::Warn("Could not parse schema LotteryProcess");
				}
				else if (header[i] == "AktVereinPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_NumClubs) != 1)
						ZED::Log::Warn("Could not parse schema m_NumClubs");
				}
				else if (header[i] == "AktTNPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_NumParticipants) != 1)
						ZED::Log::Warn("Could not parse m_NumParticipants");
				}
				else if (header[i] == "AktVerbandPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_NumAssociations) != 1)
						ZED::Log::Warn("Could not parse m_NumAssociations");
				}
				else if (header[i] == "Meldegeld")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_Money) != 1)
						ZED::Log::Warn("Could not parse Money");
				}
				else if (header[i] == "Meldegelderhoeht")
				{
					if (sscanf_s(data[i].c_str(), "%d", &m_MoneyIncreased) != 1)
						ZED::Log::Warn("Could not parse Money Increased");
				}
				else if (header[i] == "JGJIgnoreNegativeUnterbew")
					m_IgnoreNegativeScores = data[i] == "0";
			}

			return true;
		}
	}

	return false;
}



bool MD5::ReadRankScore(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			RankToPoints new_ranktopoints;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "PlatzPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_ranktopoints.RankID) != 1)
						ZED::Log::Warn("Could not read rank of relation table rank -> points");
				}
				else if (header[i] == "Punkte")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_ranktopoints.Points) != 1)
						ZED::Log::Warn("Could not read points of relation table rank -> points");
				}
			}

			m_RankToPoints.emplace_back(new_ranktopoints);

			data.clear();//Clear block	
		}

		if (newline)//Last data entry read?
			return m_RankToPoints.size() == data_count;
	}

	return false;
}



bool MD5::ReadAgeGroups(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(RemoveControlCharacters(Line));

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			AgeGroup age_group;

			for (size_t i = 0; i < data.size(); i++)
			{
				if (header[i] == "AltersgruppePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.ID) != 1)
						ZED::Log::Warn("Could not read id of age group");
				}
				else if (header[i] == "Bezeichnung")
					age_group.Name = Latin1ToUTF8(data[i]);
				else if (header[i] == "MinJahrgang")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.MinBirthyear) != 1)
						ZED::Log::Warn("Could not read MinBirthyear of age group");
				}
				else if (header[i] == "MaxJahrgang")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.MaxBirthyear) != 1)
						ZED::Log::Warn("Could not read MinBirthyear of age group");
				}
				else if (header[i] == "Geschlecht")
					age_group.Gender = data[i] == "m" ? Gender::Male : Gender::Female;
				else if (header[i] == "Aufruecken")
					age_group.MoveUp = data[i] == "1";
				else if (header[i] == "Toleranz")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.Tolerance) != 1)
						ZED::Log::Warn("Could not read Tolerance of age group");
				}
				else if (header[i] == "GewichtAnWaageAendern")
					age_group.ChangeWeightAtScale = data[i] == "1";
				else if (header[i] == "LosverfahrenPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.LotterySchemaID) != 1)
						ZED::Log::Warn("Could not read LotterySchemaID of age group");
				}
				else if (header[i] == "Allkategorie")
					age_group.AllCategories = data[i] != "F";
				else if (header[i] == "Kata")
					age_group.Kata = data[i] != "F";
				else if (header[i] == "Meldegeld")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.Money) != 1)
						ZED::Log::Warn("Could not read Money of age group");
				}
				else if (header[i] == "MeldegeldKata")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.MoneyKata) != 1)
						ZED::Log::Warn("Could not read MoneyKata of age group");
				}
				else if (header[i] == "MeldegeldAllkategorie")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.MoneyAllCategories) != 1)
						ZED::Log::Warn("Could not read MoneyAllCategories of age group");
				}
				else if (header[i] == "Meldegelderhoeht")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.MoneyIncreased) != 1)
						ZED::Log::Warn("Could not read MoneyIncreased of age group");
				}
				else if (header[i] == "MeldegeldKataerhoeht")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.MoneyKataIncreased) != 1)
						ZED::Log::Warn("Could not read MoneyKataIncreased of age group");
				}
				else if (header[i] == "MeldegeldAllkategorieerho")
				{
					if (sscanf_s(data[i].c_str(), "%d", &age_group.MoneyAllCategoriesIncreased) != 1)
						ZED::Log::Warn("Could not read MoneyKataIncreased of age group");
				}
				else if (header[i] == "Poolsystem")
					age_group.PoolSystem = data[i] == "T";
				else if (header[i] == "AlleTNinErgebnisliste")
					age_group.AllParticipantsInResultTable = data[i] == "T";
				else if (header[i] == "Mannschaft")
					age_group.Team = data[i] == "T";
			}

			m_AgeGroups.emplace_back(new AgeGroup(age_group));
			data.clear();
		}

		if (newline)
			return m_AgeGroups.size() == data_count;
	}

	return false;
}



bool MD5::ReadWeightclasses(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);
			
		data.emplace_back(RemoveControlCharacters(Line));

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			Weightclass new_weightclass;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "AltersgruppePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.AgeGroupID) != 1)
						ZED::Log::Warn("Could not read age group id of weightclass");
				}
				else if (header[i] == "GewichtsklassePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.ID) != 1)
						ZED::Log::Warn("Could not read id of weightclass");
				}
				else if (header[i] == "GewichtGroesser")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.WeightLargerThan) != 1)
						ZED::Log::Warn("Could not read WeightLargerThan of weightclass");
				}
				else if (header[i] == "GewichtKleiner")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.WeightSmallerThan) != 1)
						ZED::Log::Warn("Could not read WeightSmallerThan of weightclass");
				}
				else if (header[i] == "Bezeichnung")
					new_weightclass.Description = Latin1ToUTF8(data[i]);
				else if (header[i] == "Status")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.Status) != 1)
						ZED::Log::Warn("Could not read status of weightclass");
				}
				else if (header[i] == "WettkampfsystemPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.FightSystemID) != 1)
						ZED::Log::Warn("Could not read FightSystemID of weightclass");
				}
				else if (header[i] == "WettkampfsystemTypPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.FightSystemTypeID) != 1)
						ZED::Log::Warn("Could not read FightSystemTypeID of weightclass");
				}
				else if (header[i] == "KampfUmPlatz3")
					new_weightclass.MatchForThirdPlace = data[i] == "-1";
				else if (header[i] == "KampfUmPlatz5")
					new_weightclass.MatchForFifthPlace = data[i] == "-1";
				else if (header[i] == "Datum")
					new_weightclass.Date = data[i];
				else if (header[i] == "Weitermelden")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.Relay) != 1)
						ZED::Log::Warn("Could not read Relay of weightclass");
				}
				else if (header[i] == "MaxJGJ")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.MaxJGJ) != 1)
						ZED::Log::Warn("Could not read MaxJGJ of weightclass");
				}
				else if (header[i] == "Identifikation")
					new_weightclass.Identifier = data[i];
				else if (header[i] == "ForReference")
					new_weightclass.ForReference = data[i];
				else if (header[i] == "GewichtGroesserGramm")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.WeightInGrammsLargerThan) != 1)
						ZED::Log::Warn("Could not read WeightInGrammsLargerThan of weightclass");
				}
				else if (header[i] == "GewichtKleinerGramm")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.WeightInGrammsSmallerThan) != 1)
						ZED::Log::Warn("Could not read WeightInGrammsSmallerThan of weightclass");
				}
				else if (header[i] == "MaxVorgepoolt")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_weightclass.MaxPooled) != 1)
						ZED::Log::Warn("Could not read MaxPooled of weightclass");
				}

				//Version 7

				else if (header[i] == "BestOfThree")
					new_weightclass.BestOfThree = data[i] != "1";
			}

			m_Weightclasses.emplace_back(new Weightclass(new_weightclass));
			data.clear();
		}

		if (newline)
			return m_Weightclasses.size() == data_count;
	}

	return false;
}



bool MD5::ReadRelationClubAssociation(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			RelationClubAssociation new_relation;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "VereinPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_relation.ClubID) != 1)
						ZED::Log::Warn("Could not read club id of club relations table");
				}
				else if (header[i] == "EbenePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_relation.Tier) != 1)
						ZED::Log::Warn("Could not read tier of club relations table");
				}
				else if (header[i] == "VerbandPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_relation.AssociationID) != 1)
						ZED::Log::Warn("Could not read association id of club relations table");
				}
			}

			m_ClubRelations.emplace_back(new_relation);

			data.clear();
		}

		if (newline)
			return m_ClubRelations.size() == data_count;
	}

	return false;
}



bool MD5::ReadLottery(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			Lottery new_lottery;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "VerbandPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_lottery.AssociationID) != 1)
						ZED::Log::Warn("Could not read AssociationID of lottery table");
				}
				else if (header[i] == "LosNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_lottery.StartNo) != 1)
						ZED::Log::Warn("Could not read StartNo of lottery table");
				}
			}

			m_Lottery.emplace_back(new_lottery);
			data.clear();
		}

		if (newline)
			return m_Lottery.size() == data_count;
	}

	return false;
}



bool MD5::ReadLotteryScheme(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			LotterySchema new_lotteryschema;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "LosSchemaPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_lotteryschema.ID) != 1)
						ZED::Log::Warn("Could not read id of lottery schema");
				}
				else if (header[i] == "Bezeichnung")
					new_lotteryschema.Description = Latin1ToUTF8(data[i]);
				else if (header[i] == "VerbandPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_lotteryschema.AssociationID) != 1)
						ZED::Log::Warn("Could not read association of lottery schema");
				}
				else if (header[i] == "EbenePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_lotteryschema.TierID) != 1)
						ZED::Log::Warn("Could not read tier id of lottery schema");
				}
			}

			m_LotterySchemas.emplace_back(new_lotteryschema);
			data.clear();
		}

		if (newline)
			return m_LotterySchemas.size() == data_count;
	}

	return false;
}



bool MD5::ReadLotterySchemaLine(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			LotterySchemaLine new_lotteryschemaline;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "LosschemaPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_lotteryschemaline.LotterySchemaID) != 1)
						ZED::Log::Warn("Could not read lottery schema id of lottery schema line");
				}
				else if (header[i] == "PosPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_lotteryschemaline.PositionID) != 1)
						ZED::Log::Warn("Could not read position of lottery schema line");
				}
				else if (header[i] == "VerbandPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_lotteryschemaline.AssociationID) != 1)
						ZED::Log::Warn("Could not read association id of lottery schema line");
				}
				else if (header[i] == "PlatzPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_lotteryschemaline.RankID) != 1)
						ZED::Log::Warn("Could not read rank of lottery schema line");
				}
			}

			m_LotterySchemaLines.emplace_back(new_lotteryschemaline);
			data.clear();
		}

		if (newline)
			return m_LotterySchemaLines.size() == data_count;
	}

	return false;
}



bool MD5::ReadRelationParticipantMatchTable(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			RelationParticipantMatchTable new_relation;
			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "AltersgruppePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_relation.AgeGroupID) != 1)
						ZED::Log::Warn("Could not read age group id of relation table");
				}
				else if (header[i] == "GewichtsklassePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_relation.WeightclassID) != 1)
						ZED::Log::Warn("Could not read weight class id of relation table");
				}
				else if (header[i] == "StartNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_relation.StartNo) != 1)
						ZED::Log::Warn("Could not read start number of relation table");
				}
				else if (header[i] == "TeilnehmerPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_relation.ParticipantID) != 1)
						ZED::Log::Warn("Could not read participant id of relation table");
				}
			}

			m_Relations.emplace_back(new_relation);
			data.clear();
		}

		if (newline)
			return m_Relations.size() == data_count;
	}

	return false;
}



bool MD5::ReadMatchData(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			Match new_match;
			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "AltersgruppePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.AgeGroupID) != 1)
						ZED::Log::Warn("Could not read age group id of match");
				}
				else if (header[i] == "GewichtsklassePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.WeightclassID) != 1)
						ZED::Log::Warn("Could not read weight class id of match");
				}
				else if (header[i] == "KampfNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.MatchNo) != 1)
						ZED::Log::Warn("Could not read MatchNo of match");
				}
				else if (header[i] == "StartNRRot")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.StartNoRed) != 1)
						ZED::Log::Warn("Could not read StartNoRed of match");
				}
				else if (header[i] == "RotPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.RedID) != 1)
						ZED::Log::Warn("Could not read RedID of match");
				}
				else if (header[i] == "RotAusKampfNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.RedFromMatch) != 1)
						ZED::Log::Warn("Could not read RedFromMatch of match");
				}
				else if (header[i] == "RotTyp")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.RedTyp) != 1)
						ZED::Log::Warn("Could not read RedTyp of match");
				}
				else if (header[i] == "StartNRWeiss")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.StartNoWhite) != 1)
						ZED::Log::Warn("Could not read StartNoWhite of match");
				}
				else if (header[i] == "WeissPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.WhiteID) != 1)
						ZED::Log::Warn("Could not read WhiteID of match");
				}
				else if (header[i] == "WeissAusKampfNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.WhiteFromMatch) != 1)
						ZED::Log::Warn("Could not read WhiteFromMatch of match");
				}
				else if (header[i] == "WeissTyp")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.WhiteTyp) != 1)
						ZED::Log::Warn("Could not read WhiteTyp of match");
				}
				else if (header[i] == "SiegerPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.WinnerID) != 1)
						ZED::Log::Warn("Could not read WinnerID of match");
				}
				else if (header[i] == "SiegerKampfNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.WinnerMatchNo) != 1)
						ZED::Log::Warn("Could not read WinnerMatchNo of match");
				}
				else if (header[i] == "SiegerFarbe")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.WinnerColor) != 1)
						ZED::Log::Warn("Could not read WinnerMatchNo of match");
				}
				else if (header[i] == "VerliererPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.LoserID) != 1)
						ZED::Log::Warn("Could not read LoserColor of match");
				}
				else if (header[i] == "VerliererKampfNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.LoserMatchNo) != 1)
						ZED::Log::Warn("Could not read LoserMatchNo of match");
				}
				else if (header[i] == "VerliererFarbe")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.LoserColor) != 1)
						ZED::Log::Warn("Could not read LoserColor of match");
				}
				else if (header[i] == "WarteAufSiegerAusKampf")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.WaitingForWinnerFromMatch) != 1)
						ZED::Log::Warn("Could not read WaitingForWinnerFromMatch of match");
				}
				else if (header[i] == "Zeit")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.Time) != 1)
						ZED::Log::Warn("Could not read Time of match");
				}
				else if (header[i] == "Bewertung")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.Result) != 1)
						ZED::Log::Warn("Could not read Result of match");
				}
				else if (header[i] == "UnterbewertungSieger")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.ScoreWinner) != 1)
						ZED::Log::Warn("Could not read Result of match");
				}
				else if (header[i] == "UnterbewertungVerlierer")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.ScoreLoser) != 1)
						ZED::Log::Warn("Could not read ScoreLoser of match");
				}
				else if (header[i] == "Status")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.Status) != 1)
						ZED::Log::Warn("Could not read Status of match");
				}
				else if (header[i] == "RotAusgeschiedenKampfNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.RedOutMatchNo) != 1)
						ZED::Log::Warn("Could not read RedOutMatchNo of match");
				}
				else if (header[i] == "WeissAusgeschiedenKampfNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.WhiteOutMatchNo) != 1)
						ZED::Log::Warn("Could not read WhiteOutMatchNo of match");
				}
				else if (header[i] == "Pool")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.Pool) != 1)
						ZED::Log::Warn("Could not read Pool of match");
				}
				else if (header[i] == "DritterKampfNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.ThirdMatchNo) != 1)
						ZED::Log::Warn("Could not read ThirdMatchNo of match");
				}
				else if (header[i] == "DritterFarbe")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.ThirdColor) != 1)
						ZED::Log::Warn("Could not read ThirdMatchNo of match");
				}
				else if (header[i] == "BereichPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_match.AreaID) != 1)
						ZED::Log::Warn("Could not read AreaID of match");
				}
			}

			m_Matches.emplace_back(new_match);
			data.clear();
		}

		if (newline)
			return m_Matches.size() == data_count;
	}

	return false;
}



bool MD5::ReadResult(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			Result new_result;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "AltersgruppePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.AgeGroupID) != 1)
						ZED::Log::Warn("Could not read age group id of result");
				}
				else if (header[i] == "GewichtsklassePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.WeightclassID) != 1)
						ZED::Log::Warn("Could not read weight class group id of result");
				}
				else if (header[i] == "PlatzPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.RankID) != 1)
						ZED::Log::Warn("Could not read RankID of result");
				}
				else if (header[i] == "Pool")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.Pool) != 1)
						ZED::Log::Warn("Could not read Pool of result");
				}
				else if (header[i] == "PlatzNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.RankNo) != 1)
						ZED::Log::Warn("Could not read RankNo of result");
				}
				else if (header[i] == "KampfNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.MatchNo) != 1)
						ZED::Log::Warn("Could not read MatchNo of result");
				}
				else if (header[i] == "Platztyp")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.RankType) != 1)
						ZED::Log::Warn("Could not read RankType of result");
				}
				else if (header[i] == "TeilnehmerPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.ParticipantID) != 1)
						ZED::Log::Warn("Could not read ParticipantID of result");
				}
				else if (header[i] == "PunktePl")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.PointsPlus) != 1)
						ZED::Log::Warn("Could not read PointsPlus of result");
				}
				else if (header[i] == "PunkteMi")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.PointsMinus) != 1)
						ZED::Log::Warn("Could not read PointsMinus of result");
				}
				else if (header[i] == "UnterbewertungPl")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.ScorePlus) != 1)
						ZED::Log::Warn("Could not read ScorePlus of result");
				}
				else if (header[i] == "UnterbewertungMi")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_result.ScoreMinus) != 1)
						ZED::Log::Warn("Could not read ScoreMinus of result");
				}
				else if (header[i] == "Weitermelden")
					new_result.Relay = data[i] == "1";
				else if (header[i] == "AusPool")
					new_result.FromPool = data[i] != "0";
			}

			if (new_result.ParticipantID > 0)//Filter invalid result data
				m_Results.emplace_back(new_result);
			else
				data_count--;

			data.clear();
		}

		if (newline)
			return m_Results.size() == data_count;
	}

	return false;
}



bool MD5::ReadTable(ZED::Blob& Data, std::function<bool(std::vector<std::string>&&, std::vector<std::string>&&)> Parse)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;
	int data_written = 0;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			if (Parse && Parse(std::move(header), std::move(data)))
				data_written++;

			data.clear();
		}

		if (newline)
			return data_written == data_count;
	}

	return false;
}



bool MD5::ReadParticipants(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);

	if (data_count == 0)
		return true;

	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);
		
		data.emplace_back(RemoveControlCharacters(Line));

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			Participant new_participant;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "TeilnehmerPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.ID) != 1)
						ZED::Log::Warn("Could not read id of participant");
				}
				else if (header[i] == "AltersgruppePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.AgeGroupID) != 1)
						ZED::Log::Warn("Could not read age group id of participant");
				}
				else if (header[i] == "VereinPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.ClubID) != 1)
						ZED::Log::Warn("Could not read club id of participant");
				}
				else if (header[i] == "Nachname")
					new_participant.Lastname  = Latin1ToUTF8(data[i]);
				else if (header[i] == "Vorname")
					new_participant.Firstname = Latin1ToUTF8(data[i]);
				else if (header[i] == "GradPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.Graduation) != 1)
						ZED::Log::Warn("Could not read graduation data of participant");
				}
				else if (header[i] == "GewichtsklassePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.WeightclassID) != 1)
						ZED::Log::Warn("Could not read weight class id of participant");
				}
				else if (header[i] == "gewogen")
					new_participant.HasBeenWeighted = data[i] == "x";
				else if (header[i] == "Geburtsjahr")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.Birthyear) != 1)
						ZED::Log::Warn("Could not read birthyear of participant");
				}
				else if (header[i] == "StartNR")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.StartNo) != 1)
						ZED::Log::Warn("Could not read start number of participant");
				}
				else if (header[i] == "PlatzPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.QualificationRank) != 1)
						ZED::Log::Warn("Could not read rank of participant");
				}
				else if (header[i] == "StatusAenderung")
					new_participant.StatusChanged = data[i] == "1";
				else if (header[i] == "REDAusgeschrieben")
					new_participant.ClubFullname = Latin1ToUTF8(data[i]);
				else if (header[i] == "REDKuerzel")
					new_participant.AssociationShortname = Latin1ToUTF8(data[i]);
				else if (header[i] == "AllkategorieTNPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.AllCategoriesParticipantID) != 1)
						ZED::Log::Warn("Could not read AllCategoriesParticipantID of participant");
				}
				else if (header[i] == "KataTNPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.KataParticipantID) != 1)
						ZED::Log::Warn("Could not read KataParticipantID of participant");
				}
				else if (header[i] == "GKTNPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.GKParticipantID) != 1)
						ZED::Log::Warn("Could not read GKParticipantID of participant");
				}
				else if (header[i] == "Meldegelderhoeht")
					new_participant.MoneyIncreased = data[i] != "F";
				else if (header[i] == "Gewichtgramm")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_participant.WeightInGrams) != 1)
						ZED::Log::Warn("Could not read WeightInGramm of participant");
				}
			}

			if (new_participant.ID <= 0)//To filter dummy participants
				data_count--;
			else
				m_Participants.emplace_back(new Participant(new_participant));

			data.clear();
		}

		if (newline)
			return m_Participants.size() == data_count;
	}

	return false;
}



bool MD5::ReadAssociation(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);
	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(RemoveControlCharacters(Line));

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			Association new_association;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "VerbandPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_association.ID) != 1)
						ZED::Log::Warn("Could not read id of association");
				}
				else if (header[i] == "EbenePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_association.Tier) != 1)
						ZED::Log::Warn("Could not read tier of association");
				}
				else if (header[i] == "Bezeichnung")
					new_association.Description = Latin1ToUTF8(data[i]);
				else if (header[i] == "Kuerzel")
					new_association.ShortName   = Latin1ToUTF8(data[i]);
				else if (header[i] == "Nummer")
					new_association.Number = data[i];
				else if (header[i] == "NaechsteEbenePK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_association.NextAsscociationID) != 1)
						ZED::Log::Warn("Could not read NextAsscociationID of association");
				}
				else if (header[i] == "Aktiv")
					new_association.Active = data[i] == "1";
			}

			m_Associations.emplace_back(new Association(new_association));
			data.clear();
		}

		if (newline)
			return m_Associations.size() == data_count;
	}

	return false;
}



bool MD5::ReadClubs(ZED::Blob& Data)
{
	int data_count;
	auto header = ReadHeader(Data, data_count);
	std::vector<std::string> data;

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		data.emplace_back(Line);

		if (data.size() >= header.size())//Have we read the entire data block?
		{
			Club new_club;

			for (size_t i = 0; i < header.size(); i++)
			{
				if (header[i] == "VereinPK")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_club.ID) != 1)
						ZED::Log::Warn("Could not read club id");
				}
				else if (header[i] == "Sortierbezeichnung")
					new_club.Name_ForSorting = Latin1ToUTF8(data[i]);
				else if (header[i] == "Bezeichnung")
					new_club.Name = Latin1ToUTF8(data[i]);
				else if (header[i] == "Nachname")
					new_club.Representative_Lastname  = Latin1ToUTF8(data[i]);
				else if (header[i] == "Vorname")
					new_club.Representative_Firstname = Latin1ToUTF8(data[i]);
				else if (header[i] == "Strasse")
					new_club.Representative_Street = Latin1ToUTF8(data[i]);
				else if (header[i] == "Ort")
					new_club.Representative_Place = Latin1ToUTF8(data[i]);
				else if (header[i] == "PLZ")
					new_club.Representative_ZipCode = data[i];
				else if (header[i] == "Telp")
					new_club.Representative_TelPrivate = data[i];
				else if (header[i] == "Teld")
					new_club.Representative_TelProfessional = data[i];
				else if (header[i] == "Handy")
					new_club.Representative_TelMobil = data[i];
				else if (header[i] == "email")
					new_club.Representative_Email = data[i];
				else if (header[i] == "fax")
					new_club.Representative_Fax = data[i];
				else if (header[i] == "Vereinsnummer")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_club.OfficialClubNo) != 1)
						ZED::Log::Warn("Could not read club number");
				}
				else if (header[i] == "StatusAenderung")
				{
					if (sscanf_s(data[i].c_str(), "%d", &new_club.StatusChanged) != 1)
						ZED::Log::Info("Could not read club StatusChanged");
				}
			}

			if (new_club.ID <= 0 || new_club.Name.empty())//Valid club?
			{
				data_count--;
				m_NumClubs--;//Fix header
			}
			else
				m_Clubs.emplace_back(new Club(new_club));
			data.clear();
		}

		if (newline)
			return m_Clubs.size() == data_count;
	}

	return false;
}



std::vector<std::string> MD5::ReadHeader(ZED::Blob& Data, int& DataCount)
{
	std::vector<std::string> header;

	int headerCount = ReadInt(Data);//Read number of header entries

	while (!Data.EndReached())
	{
		bool newline;
		auto Line = ReadLine(Data, &newline);

		header.emplace_back(Line);
		if (newline)
		{
			assert(header.size() == headerCount);
			DataCount = ReadInt(Data);//Read number of data entries
			return header;
		}
	}

	assert(false);
	DataCount = -1;
	header.clear();
	return header;
}



std::string MD5::ReadLine(ZED::Blob& Data, bool* pNewLine)
{
	if (pNewLine)
		*pNewLine = false;

	bool carry_return = false;
	//bool eof          = false;

	int length = Data.ReadByte();

	std::string Line;
	while (!Data.EndReached())
	{
		char c = Data.ReadByte();//Returns 0x00 when the end of the data stream is reached

		//End of string reached without null-terminator?
		if (c != '\0' && pNewLine && *pNewLine && Line.length() >= length)
		{
			Data.SeekReadCursor(-1);
			break;
		}

		if (c == '\0'/* && Line.length() >= 1*/)
			break;
		/*else if (c == '\0')
		{
			if (eof)
				return "";
			eof = true;
			continue;
		}*/
		else if (c == 0x01)//Start of Heading
			continue;
		else if (c == 0x02)//Start of text
			continue;
		else if (c == 0x03)//End of Text
			continue;
		else if (c == 0x04)//End of Transmission
			continue;
		else if (c == 0x05)//Enquiry
			continue;
		else if (c == 0x06)//Acknowledgement
			continue;
		else if (c == 0x07)//Bell
			continue;
		else if (c == 0x08)//Backspace
			continue;
		else if (c == 0x09)//Horizontal tab
			continue;
		else if (c == 0x0B)//Vertical tab
			continue;
		else if (c == 0x0C)//Form Feed
			continue;
		else if (c == 0x0E)//Shift Out
			continue;
		else if (c == 0x0F)//Shift in
			continue;
		else if (c == 0x10)//Data Link Escape
			continue;
		else if (c == 0x11)//Device Control 1
			continue;
		else if (c == 0x12)//Device Control 2
			continue;
		else if (c == 0x13)//Device Control 3
			continue;
		else if (c == 0x14)//Device Control 4
			continue;
		else if (c == 0x15)//Negative Acknowledgement
			continue;
		else if (c == 0x16)//Synchronous Idle
			continue;
		else if (c == 0x17)//End of Transmission Block
			continue;
		else if (c == 0x18)//Cancel
			continue;
		else if (c == 0x19)//End of Medium
			continue;
		else if (c == 0x1A)//Substitute
			continue;
		else if (c == 0x1B)//Escape
			continue;
		else if (c == 0x1C)//File Separator
			continue;
		else if (c == 0x1D)//Group Separator
			continue;
		else if (c == 0x1E)//Record Separator
			continue;
		else if (c == 0x1F)//Unit Separator
			continue;
		else if (c == '\r')
		{
			carry_return = true;
			//if (Line.empty())
				//Line += c;
		}
		else if (c == '\n')
		{
			if (pNewLine && carry_return)
				*pNewLine = true;
		}
		else//Printable character
		{
			//if (Line.length() == 1 && Line[0] == '\r')
				//Line.clear();
			Line += c;
		}
	}

	assert(Line.length() == length);

	return Line;
}



int MD5::ReadInt(ZED::Blob& Data, bool* pNewLine)
{
	int ret = 0;
	bool carry_return = false;

	while (!Data.EndReached())
	{
		unsigned char c = Data.ReadByte();//Returns 0x00 when the end of the data stream is reached

		if (ret == 0 && c == '\r')
			carry_return = true;
		else if (ret == 0 && c == '\n' && carry_return)
		{
			if (pNewLine)
				*pNewLine = true;
		}
		else if (c == '\0' && (ret != 0 || carry_return))
			return ret;
		else
			ret = (ret << 8) + c;
	}

	return -1;
}



std::string MD5::RemoveControlCharacters(std::string& Str)
{
	std::string ret = Str;
	unsigned char charactersToRemove[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0B, 0x0C, 0x0F, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, '\r', '\n',
										   0xFF, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F };

	for (auto c : charactersToRemove)
		ret.erase(remove(ret.begin(), ret.end(), c), ret.end());
	return ret;
}