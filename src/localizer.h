#pragma once
#include <string>
#include <map>



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
		static void SetLanguage(Language NewLanguage) {
			s_CurrentLanguage = NewLanguage;
			Initialize(NewLanguage);
		}

	private:
		static void Initialize(Language NewLanguage);

		static Language s_CurrentLanguage;
		static std::unordered_map<std::string, std::string> s_de;
	};
}