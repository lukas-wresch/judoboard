#pragma once
#include <cassert>
#include "md5.h"
#include "../ZED/include/log.h"



using namespace Judoboard;



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

	while (!Data.EndReached())
	{
		auto line = ReadLine(Data);

		if (line == "Turnier")
			is_ok &= ReadTournamentData(Data);
		else if (line == "AktAltersgruppe")
			is_ok &= ReadAgeGroups(Data);
		else if (line == "AktGewichtsklasse")
			is_ok &= ReadWeightclasses(Data);
		else if (line == "ImportVerband")
			is_ok &= ReadAssociation(Data);
		else if (line == "Verein")
			is_ok &= ReadClubs(Data);
		else
		{
			ZED::Log::Warn("Unknown chunk: " + line);
			is_ok = false;
			break;
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
			data.emplace_back(Line);\

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
						m_Place = data[i];
					else if (header[i] == "DatumBis")
						m_Place = data[i];
					else if (header[i] == "LosEbenePK")
						;
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
						;
				}

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
					if (header[i] == "AltersgruppePK")
						;
					else if (header[i] == "Bezeichnung")
						;
					else if (header[i] == "MinJahrgang")
						;
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
						;
				}

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
					if (header[i] == "AltersgruppePK")
						;
					else if (header[i] == "GewichtsklassePK")
						;
					else if (header[i] == "GewichtGroesser")
						;
					else if (header[i] == "GewichtKleiner")
						;
					else if (header[i] == "Bezeichnung")
						;
					else if (header[i] == "Status")
						;
					else if (header[i] == "WettkampfsystemPK")
						;
					else if (header[i] == "WettkampfsystemTypPK")
						;
					else if (header[i] == "KampfUmPlatz3")
						;
					else if (header[i] == "KampfUmPlatz5")
						;
					else if (header[i] == "Datum")
						;
					else if (header[i] == "Weitermelden")
						;
					else if (header[i] == "MaxJGJ")
						;
					else if (header[i] == "Identifikation")
						;
					else if (header[i] == "ForReference")
						;
					else if (header[i] == "GewichtGroesserGramm")
						;
					else if (header[i] == "GewichtKleinerGramm")
						;
					else if (header[i] == "MaxVorgepoolt")
						;
				}

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
					if (header[i] == "VerbandPK")
						;
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
						;
				}

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
					if (header[i] == "VereinPK")
						;
					else if (header[i] == "Sortierbezeichnung")
						;
					else if (header[i] == "Bezeichnung")
						;
					else if (header[i] == "Nachname")
						;
					else if (header[i] == "Vorname")
						;
					else if (header[i] == "Strasse")
						;
					else if (header[i] == "Ort")
						;
					else if (header[i] == "PLZ")
						;
					else if (header[i] == "Telp")
						;
					else if (header[i] == "Teld")
						;
					else if (header[i] == "Handy")
						;
					else if (header[i] == "email")
						;
					else if (header[i] == "fax")
						;
					else if (header[i] == "Vereinsnummer")
						;
					else if (header[i] == "StatusAenderung")
						;
				}

				return true;
			}
		}
	}

	return false;
}



std::string MD5::ReadLine(ZED::Blob& Data, bool* pStartOfHeading)
{
	if (pStartOfHeading)
		*pStartOfHeading = false;

	std::string Line;
	while (!Data.EndReached())
	{
		char c = Data.ReadByte();//Returns 0x00 when the end of the data stream is reached

		if (c == '\0' && Line.length() >= 1)
			return Line;
		else if (c == 0x01)//Start of Heading
		{
			if (pStartOfHeading)
				*pStartOfHeading = true;
			continue;
		}
		else if (c == 0x05)//Enquiry
			continue;
		else if (c == 0x09)//Horizontal tab
			continue;
		else if (c == 0x0F)//Shift in
			continue;
		else if (c == '\r' || c == '\n')
			continue;
		else
			Line += c;
	}

	return Line;
}