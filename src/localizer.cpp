#include <unordered_map>
#include "localizer.h"
#include "../ZED/include/log.h"



using namespace Judoboard;



Language Localizer::s_CurrentLanguage = Language::English;
std::unordered_map<std::string, std::string> Localizer::s_de;



void Localizer::Initialize(Language NewLanguage)
{
	switch (NewLanguage)
	{
		case Language::English:
			break;
		case Language::German:
		{
			if (!s_de.empty())
				break;

			//a umlaut \u00e4
			//A umlaut \u00c4
			//u umlaut \u00fc
			//U umlaut \u00dc
			//o umlaut \u00f6
			//O umlaut \u00d6
			//ss       \u00df

			//German
			s_de["No."]    = u8"Nr.";
			s_de["Name"]   = u8"Name";
			s_de["Mat"]    = u8"Matte";
			s_de["White"]  = u8"Wei\u00df";
			s_de["Blue"]   = u8"Blau";
			s_de["Result"] = u8"Ergebnis";
			s_de["Wins"]   = u8"Siege";
			s_de["Score"]  = u8"Unterwertung";
			s_de["Time"]   = u8"Zeit";

			s_de["Version"] = u8"Version";
			s_de["License"] = u8"Lizenz";
			s_de["Trial version"] = u8"Testversion";

			s_de["Pause"]  = u8"Pause";
			s_de["Friendly Match"]  = u8"Freundschaftskampf";

			s_de["Not logged in"]          = u8"Nicht eingeloggt";
			s_de["Not enough permissions"] = u8"Nicht genug Rechte f\u00fcr diesen Vorgang";
			s_de["Invalid id"]             = u8"Ung\u00fcrltige id";
			s_de["Could not find mat"]     = u8"Matte nicht geufunden";
			s_de["Application is shutting down"] = "Das Programm wird heruntergefahren";
			s_de["Internal error"]         = u8"Interner Fehler";
			s_de["Not found"]              = u8"Nicht gefunden";
			s_de["No tournament is open"]  = u8"Kein Turnier ist offen";
			s_de["Operation failed"]       = u8"Vorgang fehlgeschlagen";
			s_de["Invalid format"]         = u8"Ung\u00fcrltiges Format";
			s_de["Invalid input"]          = u8"Ung\u00fcrltige Eingabe";
			s_de["License insufficient for this feature"] = u8"Lizenz unzureichend f\u00fcr diesen Vorgang";

			s_de["Running"] = u8"am K\u00e4mpfen";

			s_de["Weightclass"]   = "Gewichtsklasse";
			s_de["Current Match"] = "Aktueller Kampf";
			s_de["Next Match"]    = u8"N\u00e4chster Kampf";
			s_de["Following Matches"] = u8"Nachfolgende K\u00e4mpfe";
			s_de["Following Match"]   = "Nachfolgender Kampf";
			s_de["Default"] = "Standard";
			s_de["Round"]   = "Runde";
			s_de["Finals"]  = "Finalrunde";
			s_de["3rd Place Match"] = "3. Platz";
			s_de["5th Place Match"] = "5. Platz";

			s_de["Children"] = "Kinder";
			s_de["Youth"]    = "Jugendliche";
			s_de["Adults"]   = "Erwachsene";
			s_de["Seniors"]  = "Senioren";

			s_de["Accept"]  = "Akzeptieren";
			s_de["Decline"] = "Ablehnen";
			
			break;
		}
	}
}



std::string Localizer::Translate(const std::string& English)
{
	switch (s_CurrentLanguage)
	{
		case Language::English:
			return English;
		case Language::German:
		{
			auto it = s_de.find(English);

			if (it == s_de.end())
				ZED::Log::Warn("Could not translate: " + English);
			assert(it != s_de.end());

			if (it != s_de.end())
				return it->second;
			break;
		}
	}

	return English;
}



std::string Localizer::Gender2ShortForm(Gender Gender)
{
	switch (s_CurrentLanguage)
	{
	case Language::German:
	{
		if (Gender == Gender::Male)
			return "m";
		else if (Gender == Gender::Female)
			return "w";
		return "";
	}

	case Language::English:
	default:
		if (Gender == Gender::Male)
			return "m";
		else if (Gender == Gender::Female)
			return "f";
		return "";
	}
}