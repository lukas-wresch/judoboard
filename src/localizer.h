#pragma once
#include <string>
#include <map>
#include "judoboard.h"



namespace Judoboard 
{
	enum class Language
	{
		English, German
	};



	class Localizer
	{
	public:
		static std::string Translate(const std::string& English);
		static std::string Gender2ShortForm(Gender Gender);
		static void SetLanguage(Language NewLanguage) {
			s_CurrentLanguage = NewLanguage;
			Initialize(NewLanguage);
		}
		static Language GetLanguage() {
			return s_CurrentLanguage;
		}

	private:
		static void Initialize(Language NewLanguage);

		static Language s_CurrentLanguage;
		static std::unordered_map<std::string, std::string> s_de;
	};
}