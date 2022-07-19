#include "../ZED/include/file.h"
#include "../ZED/include/log.h"
#include "../ZED/include/csv.h"
#include "dmf.h"
#include "md5.h"



using namespace Judoboard;



DMF::DMF(const std::string& Filename)
{
	ZED::File file(Filename);

	if (file)
		Parse(ZED::Blob(file));
	else
		ZED::Log::Warn("Could not open file " + Filename);
}



std::string DMF::ReadLine(ZED::Blob& Data)
{
	bool carry_return = false;
	bool newline      = false;
	bool eof          = false;

	std::string Line;
	while (!Data.EndReached())
	{
		unsigned char c = Data.ReadByte();//Returns 0x00 when the end of the data stream is reached

		if (c == '\0' && Line.length() >= 1)
			return Line;
		else if (c == '\0')
		{
			if (eof)
				return "";
			eof = true;
			continue;
		}
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
		else if (c >= 0x80 && c <= 0x9F)//Latin1 control character
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



bool DMF::Parse(ZED::Blob&& Data)
{
	if (ReadLine(Data) != "DiskMelderDataFile")
	{
		ZED::Log::Warn("Data is not a dmf file");
		return false;
	}

	ReadLine(Data);//Empty

	m_TournamentName = ReadLine(Data);
	m_AgeGroup = ReadLine(Data);
	m_TournamentPlace = ReadLine(Data);
	m_TournamentDate  = ReadLine(Data);

	//Parse gender
	if (m_AgeGroup.length() >= 2 && m_AgeGroup[m_AgeGroup.length() - 2] == 'w')//Female?
		m_Gender = Gender::Female;

	ReadLine(Data);//Empty (number of clubs?)

	m_Club.Name    = ReadLine(Data);
	m_Club.Kreis   = ReadLine(Data);
	m_Club.Bezirk  = ReadLine(Data);
	m_Club.Country = ReadLine(Data);

	m_Sender_Name   = ReadLine(Data);
	m_Sender_Street = ReadLine(Data);
	m_Sender_Place  = ReadLine(Data);
	m_Sender_Tel    = ReadLine(Data);

	ReadLine(Data);//Empty
	ReadLine(Data);//Empty

	//Read participants
	while (true)
	{
		Participant new_participant;
		new_participant.Lastname  = ReadLine(Data);
		new_participant.Firstname = ReadLine(Data);

		//Invalid name?
		if (new_participant.Firstname.empty() || new_participant.Lastname.empty())
			break;//Probably end of file

		std::string line;
		line = ReadLine(Data);
		if (sscanf_s(line.c_str(), "%d", &new_participant.Birthyear) != 1)
			ZED::Log::Warn("Could not parse birthyear");

		line = ReadLine(Data);
		if (sscanf_s(line.c_str(), "-%d", &new_participant.WeightInGrams) != 1)
			ZED::Log::Warn("Could not parse weight");
		new_participant.WeightInGrams *= 1000;//Convert to gram

		m_Participants.emplace_back(new_participant);

		ReadLine(Data);//Empty
	}	

	m_IsValid = true;
	return m_IsValid;
}