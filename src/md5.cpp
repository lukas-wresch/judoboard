#pragma once
#include <cassert>
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
	for (auto age_group : m_AgeGroups)
		delete age_group;
	for (auto weightclass : m_Weightclasses)
		delete weightclass;
	for (auto club : m_Clubs)
		delete club;
	for (auto participant : m_Participants)
		delete participant;
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



MD5::Weightclass* MD5::FindWeightclass(int WeightclassID)
{
	if (WeightclassID <= -1)
		return nullptr;

	for (auto weightclass : m_Weightclasses)
		if (weightclass && weightclass->ID == WeightclassID)
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

			auto& results = FindResults(weightclass->AgeGroupID, weightclass->ID);

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

					int tab_count = 3;
					tab_count -= name.length() / 8;

					for (int i = 0; i < tab_count; i++)
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
		for (auto& weightclass : m_Weightclasses)
		{
			weightclass->AgeGroup = FindAgeGroup(weightclass->AgeGroupID);
		}

		for (auto& judoka : m_Participants)
		{
			judoka->Weightclass = FindWeightclass(judoka->WeightclassID);
			judoka->Club        = FindClub(judoka->ClubID);
		}

		for (auto& result : m_Results)
		{
			result.Participant = FindParticipant(result.ParticipantID);
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
			header.emplace_back(Line);
		else
		{
			data.emplace_back(Line);

			if (data.size() >= header.size())//Have we read the entire data block?
			{
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "Bezeichnung")
						m_Description = data[i];
					else if (header[i] == "VorzugsschemaPK")
					{
						if (sscanf_s(data[i].c_str(), "%d", &m_SchemaID) != 1)
						ZED::Log::Warn("Could not parse schema id");
					}
					else if (header[i] == "Ort")
						m_Place = data[i];
					else if (header[i] == "DatumVon")
						m_DateStart = data[i];
					else if (header[i] == "DatumBis")
						m_DateEnd = data[i];
					else if (header[i] == "LosEbenePK")
						;//TODO
					else if (header[i] == "VerbandPK")
						;
					else if (header[i] == "VerbandEbenePK")
						;
					else if (header[i] == "KuerzelEbenePK")
						;
					else if (header[i] == "MAXJGJ")
						;
					else if (header[i] == "KampfumPlatz3")
						;
					else if (header[i] == "KampfUmPlatz5")
						;
					else if (header[i] == "SportlicheLeitung")
						;
					else if (header[i] == "AnzWeitermelden")
						;
					else if (header[i] == "LOSVERFAHREN")
						;
					else if (header[i] == "AktVereinPK")
						;
					else if (header[i] == "AktTNPK")
						;
					else if (header[i] == "AktVerbandPK")
						;
					else if (header[i] == "Meldegeld")
						;
					else if (header[i] == "Meldegelderhoeht")
						;
					else if (header[i] == "JGJIgnoreNegativeUnterbew")
						;//TODO
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

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (are_in_data_part && newline)//Read all data blocks?
			return true;

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(Line);
		else//We are reading actual data
		{
			data.emplace_back(Line);

			if (data.size() >= header.size())//Have we read the entire data block?
			{
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "PlatzPK")
						;//TODO
					else if (header[i] == "Punkte")
						;//TODO
				}

				data.clear();//Clear block
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
						age_group.Name = data[i];
					else if (header[i] == "MinJahrgang")
						;//TODO
					else if (header[i] == "MaxJahrgang")
						;
					else if (header[i] == "Geschlecht")
						;
					else if (header[i] == "Aufruecken")
						;
					else if (header[i] == "Toleranz")
						;
					else if (header[i] == "GewichtAnWaageAendern")
						;
					else if (header[i] == "LosverfahrenPK")
						;
					else if (header[i] == "Allkategorie")
						;
					else if (header[i] == "Kata")
						;
					else if (header[i] == "Meldegeld")
						;
					else if (header[i] == "MeldegeldKata")
						;
					else if (header[i] == "MeldegeldAllkategorie")
						;
					else if (header[i] == "Meldegelderhoeht")
						;
					else if (header[i] == "MeldegeldKataerhoeht")
						;
					else if (header[i] == "MeldegeldAllkategorieerho")
						;
					else if (header[i] == "Poolsystem")
						;
					else if (header[i] == "AlleTNinErgebnisliste")
						;
					else if (header[i] == "Mannschaft")
						;//TODO
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
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(RemoveControlCharacters(Line));
		else
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
						new_weightclass.Description = data[i];
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
						new_weightclass.Date = header[i];
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
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "VereinPK")
						;
					else if (header[i] == "EbenePK")
						;
					else if (header[i] == "VerbandPK")
						;
				}

				data.clear();
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
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "LosSchemaPK")
						;//TODO
					else if (header[i] == "Bezeichnung")
						;
					else if (header[i] == "VerbandPK")
						;
					else if (header[i] == "EbenePK")
						;//TODO
				}

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
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "LosschemaPK")
						;
					else if (header[i] == "PosPK")
						;
					else if (header[i] == "VerbandPK")
						;
					else if (header[i] == "PlatzPK")
						;
				}

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
						new_participant.Lastname = data[i];
					else if (header[i] == "Vorname")
						new_participant.Firstname = data[i];
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
	bool are_in_data_part = false;

	while (!Data.EndReached())
	{
		bool start_of_heading, newline;
		auto Line = ReadLine(Data, &start_of_heading, &newline);

		if (start_of_heading)
			are_in_data_part = true;

		if (!are_in_data_part)//We are reading the header
			header.emplace_back(RemoveControlCharacters(Line));
		else
		{
			data.emplace_back(RemoveControlCharacters(Line));

			if (data.size() >= header.size())//Have we read the entire data block?
			{
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "VerbandPK")
						;//TODO
					else if (header[i] == "EbenePK")
						;
					else if (header[i] == "Bezeichnung")
						;
					else if (header[i] == "Kuerzel")
						;
					else if (header[i] == "Nummer")
						;
					else if (header[i] == "NaechsteEbenePK")
						;
					else if (header[i] == "Aktiv")
						;//TODO
				}

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
						new_club.Name_ForSorting = data[i];
					else if (header[i] == "Bezeichnung")
						new_club.Name = data[i];
					else if (header[i] == "Nachname")
						new_club.Representative_Lastname = data[i];
					else if (header[i] == "Vorname")
						new_club.Representative_Firstname = data[i];
					else if (header[i] == "Strasse")
						new_club.Representative_Street = data[i];
					else if (header[i] == "Ort")
						new_club.Representative_Place = data[i];
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
		else if (c == 0x1D)//Group Separator
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
		else if (c == '@')
			continue;
		else if (c == -109)
			continue;
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
	char charactersToRemove[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0B, 0x0C, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x0E, 0x0F, 0x15, 0x1D, '\r', '\n'};

	for (auto c : charactersToRemove)
		ret.erase(remove(ret.begin(), ret.end(), c), ret.end());
	return ret;
}