#include <unordered_map>
#include "localizer.h"



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
			s_de["Mat"]    = u8"Matte";
			s_de["White"]  = u8"Wei\u00df";
			s_de["Blue"]   = u8"Blau";
			s_de["Result"] = u8"Ergebnis";
			s_de["Wins"]   = u8"Siege";
			s_de["Score"]  = u8"Unterwertung";
			s_de["Time"]   = u8"Zeit";

			s_de["Weightclass"] = "Gewichtsklasse";
			s_de["Next Match"]  = u8"N\u00e4chster Kampf";
			s_de["Following Matches"] = u8"Nachfolgende K\u00e4mpfe";
			s_de["Following Match"] = "Nachfolgender Kampf";
			s_de["Default"] = "Standard";

			s_de["Children"] = "Kinder";
			s_de["Youth"]    = "Jugendliche";
			s_de["Adults"]   = "Erwachsene";
			s_de["Seniors"]  = "Senioren";
			
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
			return  "m";
		else if (Gender == Gender::Female)
			return "w";
		return "";
	}

	case Language::English:
	default:
		if (Gender == Gender::Male)
			return  "m";
		else if (Gender == Gender::Female)
			return "f";
		return "";
	}
}