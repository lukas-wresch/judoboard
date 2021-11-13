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
			is_ok &= ReadRelation(Data);
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
						;
					else if (header[i] == "Punkte")
						;
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
				for (size_t i = 0; i < data.size(); i++)
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
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "VerbandPK")
						;
					else if (header[i] == "LosNR")
						;
				}

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
						;
					else if (header[i] == "Bezeichnung")
						;
					else if (header[i] == "VerbandPK")
						;
					else if (header[i] == "EbenePK")
						;
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



bool MD5::ReadRelation(ZED::Blob& Data)
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
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "AltersgruppePK")
						;
					else if (header[i] == "GewichtsklassePK")
						;
					else if (header[i] == "StartNR")
						;
					else if (header[i] == "TeilnehmerPK")
						;
				}

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
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "AltersgruppePK")
						;
					else if (header[i] == "GewichtsklassePK")
						;
					else if (header[i] == "KampfNR")
						;
					else if (header[i] == "StartNRRot")
						;
					else if (header[i] == "RotPK")
						;
					else if (header[i] == "RotAusKampfNR")
						;
					else if (header[i] == "RotTyp")
						;
					else if (header[i] == "StartNRWeiss")
						;
					else if (header[i] == "WeissPK")
						;
					else if (header[i] == "WeissAusKampfNR")
						;
					else if (header[i] == "WeissTyp")
						;
					else if (header[i] == "SiegerPK")
						;
					else if (header[i] == "SiegerKampfNR")
						;
					else if (header[i] == "SiegerFarbe")
						;
					else if (header[i] == "VerliererPK")
						;
					else if (header[i] == "VerliererKampfNR")
						;
					else if (header[i] == "VerliererFarbe")
						;
					else if (header[i] == "WarteAufSiegerAusKampf")
						;
					else if (header[i] == "Zeit")
						;
					else if (header[i] == "Bewertung")
						;
					else if (header[i] == "UnterbewertungSieger")
						;
					else if (header[i] == "UnterbewertungVerlierer")
						;
					else if (header[i] == "Status")
						;
					else if (header[i] == "RotAusgeschiedenKampfNR")
						;
					else if (header[i] == "WeissAusgeschiedenKampfNR")
						;
					else if (header[i] == "Pool")
						;
					else if (header[i] == "DritterKampfNR")
						;
					else if (header[i] == "DritterFarbe")
						;
					else if (header[i] == "BereichPK")
						;
				}

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
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "AltersgruppePK")
						;
					else if (header[i] == "GewichtsklassePK")
						;
					else if (header[i] == "PlatzPK")
						;
					else if (header[i] == "Pool")
						;
					else if (header[i] == "PlatzNR")
						;
					else if (header[i] == "KampfNR")
						;
					else if (header[i] == "Platztyp")
						;
					else if (header[i] == "TeilnehmerPK")
						;
					else if (header[i] == "PunktePl")
						;
					else if (header[i] == "PunkteMi")
						;
					else if (header[i] == "UnterbewertungPl")
						;
					else if (header[i] == "UnterbewertungMi")
						;
					else if (header[i] == "Weitermelden")
						;
					else if (header[i] == "AusPool")
						;
				}

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
				for (size_t i = 0; i < header.size(); i++)
				{
					if (header[i] == "TeilnehmerPK")
						;
					else if (header[i] == "AltersgruppePK")
						;
					else if (header[i] == "VereinPK")
						;
					else if (header[i] == "Vorname")
						;
					else if (header[i] == "GradPK")
						;
					else if (header[i] == "GewichtsklassePK")
						;
					else if (header[i] == "gewogen")
						;
					else if (header[i] == "Geburtsjahr")
						;
					else if (header[i] == "StartNR")
						;
					else if (header[i] == "PlatzPK")
						;
					else if (header[i] == "StatusAenderung")
						;
					else if (header[i] == "REDAusgeschrieben")
						;
					else if (header[i] == "REDKuerzel")
						;
					else if (header[i] == "AllkategorieTNPK")
						;
					else if (header[i] == "KataTNPK")
						;
					else if (header[i] == "GKTNPK")
						;
					else if (header[i] == "Meldegelderhoeht")
						;
					else if (header[i] == "Gewichtgramm")
						;
				}

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
		else if (c == 0x0C)//Form Feed
			continue;
		else if (c == 0x0E)//Shift Out
			continue;
		else if (c == 0x0F)//Shift in
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
	char charactersToRemove[] = { 0x01, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0C, 0x11, 0x12, 0x13, 0x14, 0x0E, 0x0F, 0x15, 0x1D, '\r', '\n'};

	for (auto c : charactersToRemove)
		ret.erase(remove(ret.begin(), ret.end(), c), ret.end());
	return ret;
}