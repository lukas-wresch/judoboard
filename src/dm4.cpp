#include "../ZED/include/file.h"
#include "../ZED/include/log.h"
#include "../ZED/include/csv.h"
#include "dm4.h"



using namespace Judoboard;



DM4::DM4(const std::string& Filename)
{
	ZED::File file(Filename);

	if (file)
		Parse(ZED::Blob(file));
	else
		ZED::Log::Warn("Could not open file " + Filename);
}



bool DM4::Parse(ZED::Blob&& Data)
{
	m_IsValid = false;

	//Read line by line
	std::string line;
	for (size_t i = 0; i < Data.GetSize(); i++)
	{
		if (Data[i] == '\n')
			continue;

		else if (Data[i] == '\r')
		{
			if (!ParseLine(line))
			{
				ZED::Log::Warn("Parsing error on line: " + line);
				return false;
			}

			line.clear();
			continue;
		}

		line += Data[i];
	}

	m_IsValid = true;
	return m_IsValid;
}



DM4::~DM4()
{
	for (auto club : m_Clubs)
		delete club;
}



bool DM4::ParseLine(const std::string& Line)
{
	const bool isStartOfChunk = ParseStartOfChunk(Line);

	switch (m_CurrentChunk)
	{
		case Chunk::Unknow:
		{
			return isStartOfChunk;
		}

		case Chunk::Identification:
		{
			break;
		}

		case Chunk::Sender:
		{
			GetValue(Line, "Verband=", m_Sender_ClubName);
			GetValue(Line, "Name=",    m_Sender_Name);
			break;
		}

		case Chunk::Report:
		{
			GetValue(Line, "Turnier=", m_TournamentName);
			GetValue(Line, "Datum=",   m_TournamentDate);
			GetValue(Line, "Ort=",     m_TournamentPlace);
			GetValue(Line, "Altersgruppe=", m_AgeGroup);

			std::string gender;
			if (GetValue(Line, "Geschlecht=", gender))
			{
				if (gender == "m")
					m_Gender = Gender::Male;
				else if (gender == "w")
					m_Gender = Gender::Female;
				else
					ZED::Log::Warn("Could not parse gender at line: " + Line);
			}
			break;
		}

		case Chunk::Clubs:
		{
			if (isStartOfChunk)
				return true;

			std::string count;
			if (!GetValue(Line, "Anzahl=", count))
			{
				ZED::CSV data(Line);
				std::string id,   club,   unknown1,   club_no,   lastname,   firstname,   street,   zipcode,   place,   unknown3, unknown4, mobil, email, county, state_city, state, unknown5, country;
				data     >> id >> club >> unknown1 >> club_no >> lastname >> firstname >> street >> zipcode >> place >> unknown3 >> unknown4 >> mobil >> email >> county >> state_city >> state >> unknown5 >> country;

				Club new_club;

				RemoveCharFromString(id, '\"');
				if (sscanf_s(id.c_str(), "%d=", &new_club.ID) != 1)
					ZED::Log::Warn("Could not parse id of line:" + Line);

				new_club.Name                     = RemoveCharFromString(club, '\"');
				new_club.Representative_Firstname = RemoveCharFromString(firstname, '\"');
				new_club.Representative_Lastname  = RemoveCharFromString(lastname,  '\"');

				m_Clubs.emplace_back(new Club(new_club));
			}
			break;
		}

		case Chunk::Participants:
		{
			if (isStartOfChunk)
				return true;

			std::string count;
			if (!GetValue(Line, "Anzahl=", count))
			{
				ZED::CSV data(Line);
				std::string id,   lastname,   firstname,   unknown1,   weight,   unknown2,   birthyear,   unknown3,   unknown4,   unknown5,   unknown6,   unknown7;
				data     >> id >> lastname >> firstname >> unknown1 >> weight >> unknown2 >> birthyear >> unknown3 >> unknown4 >> unknown5 >> unknown6 >> unknown7;

				Participant new_participant;

				RemoveCharFromString(id, '\"');
				if (sscanf_s(id.c_str(), "%d=%d", &new_participant.ID, &new_participant.ClubID) != 2)
					ZED::Log::Warn("Could not parse id of line: " + Line);

				new_participant.Firstname = Latin1ToUTF8(RemoveCharFromString(firstname, '\"'));
				new_participant.Lastname  = Latin1ToUTF8(RemoveCharFromString(lastname,  '\"'));

				RemoveCharFromString(weight, '\"');
				if (sscanf_s(weight.c_str(), "%d", &new_participant.WeightInGrams) != 1)
					ZED::Log::Debug("Could not parse weight of line:" + Line);
				if (new_participant.WeightInGrams > 0)
					new_participant.WeightInGrams *= 1000;//Convert to gram

				RemoveCharFromString(birthyear, '\"');
				if (sscanf_s(birthyear.c_str(), "%d", &new_participant.Birthyear) != 1)
					ZED::Log::Debug("Could not parse birthyear of line:" + Line);

				//Find Club
				new_participant.Club = FindClubByID(new_participant.ClubID);
				new_participant.Gender = m_Gender;//Set gender

				m_Participants.emplace_back(new_participant);
			}
			break;
		}
	}

	return true;
}



bool DM4::ParseStartOfChunk(const std::string& Line)
{
	if (Line == "[Identifikation]")
		m_CurrentChunk = Chunk::Identification;
	else if (Line == "[Absender]")
		m_CurrentChunk = Chunk::Sender;
	else if (Line == "[Meldung]")
		m_CurrentChunk = Chunk::Report;
	else if (Line == "[Vereine]")
		m_CurrentChunk = Chunk::Clubs;
	else if (Line == "[Teilnehmer]")
		m_CurrentChunk = Chunk::Participants;
	else
		return false;

	return true;
}



bool DM4::GetValue(const std::string& Line, const std::string& Key, std::string& Result) const
{
	auto pos = Line.find(Key);
	if (pos != std::string::npos)
	{
		Result = Line.substr(pos + Key.length());
		return true;
	}
	return false;
}



std::string DM4::RemoveCharFromString(std::string& Str, char CharacterToRemove) const
{
	Str.erase(remove(Str.begin(), Str.end(), CharacterToRemove), Str.end());
	return Str;
}



const DM4::Club* DM4::FindClubByID(int ClubID) const
{
	for (auto club : m_Clubs)
		if (club && club->ID == ClubID)
			return club;
	return nullptr;
}