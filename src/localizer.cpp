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

			//German
			s_de["Mat"] = "Matte";
			s_de["Weightclass"] = "Gewichtsklasse";
			s_de["Next Match"] = u8"Nächster Kampf";
			s_de["Following Matches"] = u8"Nachfolgende Kämpfe";
			s_de["Following Match"] = "Nachfolgender Kampf";
			s_de["Default"] = "Standard";
			
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