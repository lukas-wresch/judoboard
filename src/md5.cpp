#include <cassert>
#include <array>
#include "md5.h"
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

	auto Write_IntRaw = [&](int32_t Num) {
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
		Write_Int(m_LotteryLevelID);
		Write_Int(m_AssociationID);
		Write_Int(m_AssociationLevelID);
		Write_Int(m_LevelShortID);		
		Write_Int(m_MAXJGJ);		
		Write_Int(m_ThirdPlaceMatch ? 0 : -1);		
		Write_Int(m_FifthPlaceMatch ? 0 : -1);		
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
			Write_Int(rank2points.Rank);
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
			Write_Line(age_group->Name);
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
			Write_Line(age_group->Team ? "T" : "");//Format unclear (TODO)
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

		Write_IntRaw(m_AgeGroups.size());//Number of columns

		for (auto& weightclass : m_Weightclasses)
		{
			Write_Int(weightclass->AgeGroupID);
			Write_Int(weightclass->ID);
			Write_Int(weightclass->WeightLargerThan);
			Write_Int(weightclass->WeightSmallerThan);

			Write_Line(weightclass->Description);
			Write_Int(weightclass->Status);
			Write_Int(weightclass->FightSystemID);
			Write_Int(weightclass->FightSystemTypeID);

			Write_Int(weightclass->MatchForThirdPlace);
			Write_Int(weightclass->MatchForFifthPlace);
			Write_Line(weightclass->Date);
			Write_Int(weightclass->Relay);
			Write_Int(weightclass->MaxJGJ);
			Write_Line(weightclass->Identifier);
			Write_Line(weightclass->ForReference);

			Write_Int(weightclass->WeightInGrammsLargerThan);
			Write_Int(weightclass->WeightInGrammsSmallerThan);
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
			Write_Int(association->TierID);
			Write_Line(association->Description);
			Write_Line(association->ShortName);
			Write_Int(association->Number);
			Write_Int(association->NextAsscociationID);
			Write_Int(association->Active);
		}

		file.Seek(-1);//Delete last \0
		Write_0D0A00();
	}

	Write_Line("\\\\end");

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



MD5::Club* MD5::FindClub(int ClubID)
{
	if (ClubID <= -1)
		return nullptr;

	for (auto club : m_Clubs)
		if (club && club->ID == ClubID)
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



const MD5::Result* MD5::FindResult(int AgeGroupID, int WeightclassID, int Rank) const
{
	for (auto& result : m_Results)
		if (result.AgeGroupID == AgeGroupID && result.WeightclassID == WeightclassID && result.RankNo == Rank)
			return &result;
	return nullptr;
}



const MD5::Result* MD5::FindResult(const std::string& AgeGroup, const std::string& Weightclass, int Rank) const
{
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



void MD5::Dump() const
{
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
				}

				ZED::Log::Info(line);
			}
		}
	}
}



bool MD5::Parse(ZED::Blob&& Data)
{
	if (ReadLine(Data) != "MMW98")
	{
		ZED::Log::Warn("Data is not a MD5 file");
		return false;
	}

	if (ReadLine(Data) != "3")
	{
		ZED::Log::Warn("Data is not a MD5 file");
		return false;
	}

	if (ReadLine(Data) != "Version 51")
	{
		ZED::Log::Warn("Data is not a MD5 file");
		return false;
	}

	m_FileDate = ReadLine(Data);

	bool is_ok = true;
	bool found_end = false;

	while (!Data.EndReached() && is_ok)
	{
		auto line = ReadLine(Data);

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

		else if (line == "\r\n")
			continue;
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
	}

	if (!found_end)//Did we find the \\end tag at the end of the file?
		is_ok = false;

	if (is_ok)
	{
		//Resolve dependencies
		for (auto& association : m_Associations)
		{
			association->NextAsscociation = FindAssociation(association->NextAsscociationID);
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
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading;
		auto Line = ReadLine(Data, &start_of_heading);

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(RemoveControlCharacters(Line));
		else
		{
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
						if (sscanf_s(data[i].c_str(), "%d", &m_LotteryLevelID) != 1)
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
						if (sscanf_s(data[i].c_str(), "%d", &m_LevelShortID) != 1)
							ZED::Log::Warn("Could not parse schema LevelShortID");
					}
					else if (header[i] == "MAXJGJ")
					{
						if (sscanf_s(data[i].c_str(), "%d", &m_MAXJGJ) != 1)
							ZED::Log::Warn("Could not parse schema MAXJGJ");
					}
					else if (header[i] == "KampfumPlatz3")
						m_ThirdPlaceMatch = data[i] != "-1";
					else if (header[i] == "KampfUmPlatz5")
						m_FifthPlaceMatch = data[i] != "-1";
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
							ZED::Log::Warn("Could not parse schema m_NumParticipants");
					}
					else if (header[i] == "AktVerbandPK")
					{
						if (sscanf_s(data[i].c_str(), "%d", &m_NumAssociations) != 1)
							ZED::Log::Warn("Could not parse schema m_NumAssociations");
					}
					else if (header[i] == "Meldegeld")
					{
						if (sscanf_s(data[i].c_str(), "%d", &m_Money) != 1)
							ZED::Log::Warn("Could not parse schema Money");
					}
					else if (header[i] == "Meldegelderhoeht")
					{
						if (sscanf_s(data[i].c_str(), "%d", &m_MoneyIncreased) != 1)
							ZED::Log::Warn("Could not parse schema Money");
					}
					else if (header[i] == "JGJIgnoreNegativeUnterbew")
						m_IgnoreNegativeScores = data[i] == "0";
				}

				return true;
			}
		}
	}

	return false;
}



bool MD5::ReadRankScore(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_data_part = false;
	bool end_of_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (are_in_data_part && newline)//Read all data blocks?
			end_of_data_part = true;

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(Line);
		else//We are reading actual data
		{
			data.emplace_back(Line);

			if (data.size() >= header.size())//Have we read the entire data block?
			{
				RankToPoints new_ranktopoints;

				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "PlatzPK")
					{
						if (sscanf_s(data[i].c_str(), "%d", &new_ranktopoints.Rank) != 1)
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

				if (end_of_data_part)//Last data entry read?
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadAgeGroups(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline, doublezero;
		auto Line = ReadLine(Data, &start_of_heading, &newline, &doublezero);

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(RemoveControlCharacters(Line));
		else
		{
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

				if (Line == "\r\n")
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadWeightclasses(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_header_part = true;
	bool are_in_data_part   = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);		
			

		if (are_in_header_part)//We are reading the header
		{
			header.emplace_back(RemoveControlCharacters(Line));

			if (newline)
				are_in_header_part = false;
		}
		else if (start_of_heading)
			are_in_data_part = true;


		if (are_in_data_part)
		{
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
						new_weightclass.MatchForThirdPlace = data[i] != "-1";
					else if (header[i] == "KampfUmPlatz5")
						new_weightclass.MatchForFifthPlace = data[i] != "-1";
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
				}

				m_Weightclasses.emplace_back(new Weightclass(new_weightclass));
				data.clear();

				if (newline)
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadRelationClubAssociation(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		//if (Line == "\r\n")
			//return true;

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(Line);
		else
		{
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
						if (sscanf_s(data[i].c_str(), "%d", &new_relation.TierID) != 1)
							ZED::Log::Warn("Could not read club id of club relations table");
					}
					else if (header[i] == "VerbandPK")
					{
						if (sscanf_s(data[i].c_str(), "%d", &new_relation.AssociationID) != 1)
							ZED::Log::Warn("Could not read club id of club relations table");
					}
				}

				m_ClubRelations.emplace_back(new_relation);

				data.clear();

				if (newline)
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadLottery(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (Line == "\r\n")
			return true;

		//if (start_of_heading)
			//are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
		{
			header.emplace_back(Line);
			if (newline)
				are_in_data_part = true;
		}
		else
		{
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

				if (newline)
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadLotteryScheme(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading;
		auto Line = ReadLine(Data, &start_of_heading);

		if (Line == "\r\n")
			return true;

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(Line);
		else
		{
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
		}
	}

	return false;
}



bool MD5::ReadLotterySchemaLine(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading;
		auto Line = ReadLine(Data, &start_of_heading);

		if (Line == "\r\n")
			return true;

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(Line);
		else
		{
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
		}
	}

	return false;
}



bool MD5::ReadRelationParticipantMatchTable(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_header_part = true;
	bool are_in_data_part   = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (Line == "\r\n")
			return true;

		//if (start_of_heading)
			//are_in_data_part = true;

		if (are_in_header_part)//We are reading the header
		{
			header.emplace_back(Line);
			if (newline)
				are_in_header_part = false;
		}
		else if (start_of_heading)
			are_in_data_part = true;

		if (are_in_data_part)
		{
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

				if (newline)
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadMatchData(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_header_part = true;
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (Line == "\r\n")
			return true;

		if (are_in_header_part)//We are reading the header
		{
			header.emplace_back(Line);
			if (newline)
				are_in_header_part = false;
		}
		else if (start_of_heading)
			are_in_data_part = true;

		if (are_in_data_part)
		{
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
						if (sscanf_s(data[i].c_str(), "%d", &new_match.LoserColor) != 1)
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
						if (sscanf_s(data[i].c_str(), "%d", &new_match.RedOutMatchID) != 1)
							ZED::Log::Warn("Could not read RedOutMatchID of match");
					}
					else if (header[i] == "WeissAusgeschiedenKampfNR")
					{
						if (sscanf_s(data[i].c_str(), "%d", &new_match.WhiteOutMatchID) != 1)
							ZED::Log::Warn("Could not read WhiteOutMatchID of match");
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

				if (newline)
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadResult(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_header_part = true;
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (Line == "\r\n")
			return true;

		if (are_in_header_part)//We are reading the header
		{
			header.emplace_back(Line);
			if (newline)
				are_in_header_part = false;
		}
		else if (start_of_heading)
			are_in_data_part = true;

		if (are_in_data_part)
		{
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
				data.clear();

				if (newline)
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadParticipants(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_header_part = true;
	bool are_in_data_part   = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (are_in_header_part)//We are reading the header
		{
			header.emplace_back(RemoveControlCharacters(Line));

			if (newline)
				are_in_header_part = false;
		}
		else
		{
			if (start_of_heading)
				are_in_data_part = true;
		}

		if (are_in_data_part)
		{
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
						new_participant.Lastname = Latin1ToUTF8(data[i]);
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
						if (sscanf_s(data[i].c_str(), "%d", &new_participant.RankID) != 1)
							ZED::Log::Warn("Could not read rank id of participant");
					}
					else if (header[i] == "StatusAenderung")
						new_participant.StatusChanged = data[i] == "1";
					else if (header[i] == "REDAusgeschrieben")
						new_participant.ClubFullname = data[i];
					else if (header[i] == "REDKuerzel")
						new_participant.ClubShortname = data[i];
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
						if (sscanf_s(data[i].c_str(), "%d", &new_participant.WeightInGramm) != 1)
							ZED::Log::Warn("Could not read WeightInGramm of participant");
					}
				}

				if (new_participant.ID != 0)//To filter dummy participants
					m_Participants.emplace_back(new Participant(new_participant));

				data.clear();

				if (newline)
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadAssociation(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_header_part = true;
	bool are_in_data_part   = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);


		if (are_in_header_part)//We are reading the header
		{
			header.emplace_back(RemoveControlCharacters(Line));
			if (newline)
				are_in_header_part = false;
		}
		else if (start_of_heading)
			are_in_data_part = true;


		if (are_in_data_part)
		{
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
						if (sscanf_s(data[i].c_str(), "%d", &new_association.TierID) != 1)
							ZED::Log::Warn("Could not read tier id of association");
					}
					else if (header[i] == "Bezeichnung")
						new_association.Description = Latin1ToUTF8(data[i]);
					else if (header[i] == "Kuerzel")
						new_association.ShortName   = Latin1ToUTF8(data[i]);
					else if (header[i] == "Nummer")
					{
						if (sscanf_s(data[i].c_str(), "%d", &new_association.Number) != 1)
							ZED::Log::Warn("Could not read number of association");
					}
					else if (header[i] == "NaechsteEbenePK")
					{
						if (sscanf_s(data[i].c_str(), "%d", &new_association.NextAsscociationID) != 1)
							ZED::Log::Warn("Could not read NextTierID of association");
					}
					else if (header[i] == "Aktiv")
						new_association.Active = data[i] == "1";
				}

				m_Associations.emplace_back(new Association(new_association));
				data.clear();

				if (newline)
					return true;
			}
		}
	}

	return false;
}



bool MD5::ReadClubs(ZED::Blob& Data)
{
	std::vector<std::string> header;
	std::vector<std::string> data;
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(Line);
		else
		{
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
							ZED::Log::Warn("Could not read status changed of club");
					}
				}

				m_Clubs.emplace_back(new Club(new_club));
				data.clear();

				if (newline)
					return true;
			}
		}
	}

	return false;
}



std::string MD5::ReadLine(ZED::Blob& Data, bool* pStartOfHeading, bool* pNewLine, bool* pDoubleZero)
{
	if (pStartOfHeading)
		*pStartOfHeading = false;
	if (pNewLine)
		*pNewLine = false;
	if (pDoubleZero)
		*pDoubleZero = false;

	bool carry_return = false;
	bool newline      = false;
	bool eof          = false;

	std::string Line;
	while (!Data.EndReached())
	{
		char c = Data.ReadByte();//Returns 0x00 when the end of the data stream is reached

		if (c == '\0' && Line.length() >= 1)
			return Line;
		else if (c == '\0')
		{
			if (eof)
			{
				if (pDoubleZero)
					*pDoubleZero = true;
				return "";
			}
			eof = true;
			continue;
		}
		else if (c == 0x01)//Start of Heading
		{
			if (pStartOfHeading)
				*pStartOfHeading = true;
			continue;
		}
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
			if (Line.empty())
				Line += c;
		}
		else if (c == '\n')
		{
			newline = true;
			if (pNewLine && carry_return)
				*pNewLine = true;
			if (Line.length() == 1)
				Line += c;
		}
		else//Printable character
		{
			if (Line.length() == 1 && Line[0] == '\r')
				Line.clear();
			Line += c;
		}
	}

	return Line;
}



std::string MD5::RemoveControlCharacters(std::string& Str)
{
	std::string ret = Str;
	char charactersToRemove[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0B, 0x0C, 0x0F, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, '\r', '\n'};

	for (auto c : charactersToRemove)
		ret.erase(remove(ret.begin(), ret.end(), c), ret.end());
	return ret;
}