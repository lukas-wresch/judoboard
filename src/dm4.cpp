#include "../ZED/include/file.h"
#include "../ZED/include/blob.h"
#include "../ZED/include/log.h"
#include "dm4.h"



using namespace Judoboard;



DM4::DM4(const std::string& Filename)
{
	ZED::File file(Filename);

	if (!file)
	{
		ZED::Log::Warn("Could not open file " + Filename);
		return;
	}

	ZED::Blob data(file);

	//Read line by line
	std::string line;
	for (size_t i = 0; i < data.GetSize(); i++)
	{
		if (data[i] == '\n')
			continue;

		else if (data[i] == '\r')
		{
			if (!ParseLine(line))
			{
				ZED::Log::Warn("Parsing error on line: " + line);
				return;
			}

			line = "";
			continue;
		}

		line += data[i];
	}

	m_IsValid = true;
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
			break;
		}

		case Chunk::Clubs:
		{
			break;
		}

		case Chunk::Participants:
		{
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
	auto pos = Line.rfind(Key, 0);
	if (pos != std::string::npos)
	{
		Result = Line.substr(pos + Key.length());
		return true;
	}
	return false;
}