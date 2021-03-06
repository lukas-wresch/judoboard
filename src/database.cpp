#include <fstream>
#include <random>
#include "../ZED/include/log.h"
#include "../ZED/include/sha512.h"
#include "database.h"
#include "localizer.h"
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"



using namespace Judoboard;



void Database::Reset()
{
	StandingData::Reset();

	for (auto account : m_Accounts)
		delete account;

	m_Accounts.clear();
	m_OpenNonces.clear();
	m_ClosedNonces.clear();

	//No rule set defined and no age groups defined
	if (m_RuleSets.empty() && m_AgeGroups.empty())
	{
		auto childen = new RuleSet(Localizer::Translate("Children"), 2*60, 0, 20, 10);
		auto youth   = new RuleSet(Localizer::Translate("Youth"),    3*60, 0, 20, 10);
		auto adults  = new RuleSet(Localizer::Translate("Adults"),   4*60, 0, 20, 10);

		AddRuleSet(childen);
		AddRuleSet(youth);
		AddRuleSet(adults);

		AddAgeGroup(new AgeGroup("U11", 8,  10, childen, *this));
		AddAgeGroup(new AgeGroup("U13", 10, 12, youth,   *this));
		AddAgeGroup(new AgeGroup("U15", 12, 14, youth,   *this));
		AddAgeGroup(new AgeGroup("U18", 15, 17, adults,  *this));
		AddAgeGroup(new AgeGroup("U21", 17, 20, adults,  *this));
		AddAgeGroup(new AgeGroup(Localizer::Translate("Seniors"), 17, 0, adults,  *this));
	}
}



bool Database::Load(const std::string& Filename)
{
	m_Filename = Filename;
	std::ifstream file(Filename);
	if (!file)
	{
		ZED::Log::Warn("Could not open file " + Filename);
		return false;
	}

	YAML::Node yaml = YAML::LoadFile(Filename);

	if (!yaml)
	{
		ZED::Log::Warn("Could not open file " + Filename);
		return false;
	}

	Reset();

	if (!yaml["version"] || yaml["version"].as<int>() != 1)
	{
		ZED::Log::Error("File format is too new for this application to read. Please update this application");
		return false;
	}

	//Read standing data
	StandingData::operator <<(yaml);

	if (yaml["accounts"] && yaml["accounts"].IsSequence())
	{
		for (const auto& account : yaml["accounts"])
			AddAccount(Account(account));
	}

	return true;
}



bool Database::Save(const std::string& Filename) const
{
	if (Filename[0] == '\0')
		return false;

	std::ofstream file(Filename);

	if (!file)
	{
		ZED::Log::Error("Could not save file " + Filename);
		return false;
	}

	YAML::Emitter yaml;

	yaml << YAML::BeginMap;
	yaml << YAML::Key << "version" << YAML::Value << "1";
	
	StandingData::operator >>(yaml);

	yaml << YAML::Key << "accounts";
	yaml << YAML::Value;
	yaml << YAML::BeginSeq;

	for (auto account : m_Accounts)
	{
		if (account)
			*account >> yaml;
	}

	yaml << YAML::EndSeq;
	yaml << YAML::EndMap;
	file << yaml.c_str();
	return true;
}



Judoka* Database::UpdateOrAdd(const MD5::Participant& NewJudoka)
{
	DM4::Participant dm4_judoka(NewJudoka);

	std::string output;
	auto ret = UpdateOrAdd(dm4_judoka, false, output);
	ZED::Log::Debug(output);

	return ret;
}



Judoka* Database::UpdateOrAdd(const DM4::Participant& NewJudoka, bool ParseOnly, std::string& Output)
{
	auto old_judoka = FindJudoka_DM4_ExactMatch(NewJudoka);

	if (old_judoka)//Exact match
		return old_judoka;

	else//No exact match
	{
		old_judoka = FindJudoka_DM4_SameName(NewJudoka);

		if (old_judoka)//Found someone with the right name but incorrect club/birthyear
		{
			Output += "Updating information of judoka: " + old_judoka->GetName() + "<br/>";

			if (!ParseOnly)
			{
				if (NewJudoka.Club && FindClubByName(NewJudoka.Club->Name))
					old_judoka->SetClub(FindClubByName(NewJudoka.Club->Name));
				if (NewJudoka.Birthyear > 0)
					old_judoka->SetBirthyear(NewJudoka.Birthyear);
				if (NewJudoka.WeightInGrams > 0)
					old_judoka->SetWeight(NewJudoka.WeightInGrams);
			}

			return old_judoka;
		}

		else//We don't have a judoka with this name
		{
			//Add to database
			Output += "Adding judoka: " + NewJudoka.Firstname + " " + NewJudoka.Lastname + "<br/>";

			if (!ParseOnly)
			{
				auto new_judoka = new Judoka(NewJudoka);
				AddJudoka(new_judoka);
				return new_judoka;
			}
		}
	}

	return nullptr;
}



Judoka* Database::UpdateOrAdd(const Judoka& NewJudoka, bool ParseOnly, std::string& Output)
{
	auto old_judoka = FindJudoka_ExactMatch(NewJudoka);

	if (old_judoka)//Exact match
		return old_judoka;

	else//No exact match
	{
		old_judoka = FindJudoka_SameName(NewJudoka);

		if (old_judoka)//Found someone with the right name but incorrect club/birthyear
		{
			Output += "Updating information of judoka: " + old_judoka->GetName() + "<br/>";

			if (!ParseOnly)
			{
				if (NewJudoka.GetClub() && FindClubByName(NewJudoka.GetClub()->GetName()))
					old_judoka->SetClub(FindClubByName(NewJudoka.GetClub()->GetName()));
				if (NewJudoka.GetBirthyear() > 0)
					old_judoka->SetBirthyear(NewJudoka.GetBirthyear());
				if (NewJudoka.GetWeight() > 0)
					old_judoka->SetWeight(NewJudoka.GetWeight());
			}

			return old_judoka;
		}

		else//We don't have a judoka with this name
		{
			//Add to database
			Output += "Adding judoka: " + NewJudoka.GetFirstname() + " " + NewJudoka.GetLastname() + "<br/>";

			if (!ParseOnly)
			{
				auto new_judoka = new Judoka(NewJudoka);
				AddJudoka(new_judoka);
				return new_judoka;
			}
		}
	}

	return nullptr;
}



const Account* Database::AddAccount(Account* NewAccount)
{
	m_Accounts.push_back(NewAccount);

	Save();

	return NewAccount;
}



const Account* Database::FindAccount(const std::string& Username) const
{
	for (auto account : m_Accounts)
		if (account && account->GetUsername() == Username)
			return account;

	return nullptr;
}



bool Database::DeleteAccount(const std::string& Username)
{
	for (auto it = m_Accounts.begin(); it != m_Accounts.end(); ++it)
	{
		if (*it && (*it)->GetUsername() == Username)
		{
			m_Accounts.erase(it);
			Save(m_Filename);
			return true;
		}
	}

	return false;
}



const Account::Nonce& Database::CreateNonce(uint32_t IP, uint16_t RemotePort)
{
	m_OpenNonces.emplace_back(IP, RemotePort);
	return m_OpenNonces[m_OpenNonces.size()-1];
}



bool Database::DoLogin(const std::string& Username, uint32_t IP, const std::string& Response)
{
	auto account = FindAccount(Username);
	if (!account)
		return false;

	for (auto nonce_it = m_OpenNonces.cbegin(); nonce_it != m_OpenNonces.cend();)
	{
		if (nonce_it->HasExpired())
			nonce_it = m_OpenNonces.erase(nonce_it);

		else if (account->Verify(*nonce_it, Response))
		{
			m_ClosedNonces.insert({ Response, {*nonce_it, account} });
			m_OpenNonces.erase(nonce_it);

			return true;
		}
		else
			++nonce_it;
	}

	return false;
}



const Account* Database::IsLoggedIn(uint32_t IP, const std::string& Response) const
{
	auto item = m_ClosedNonces.find(Response);

	if (item == m_ClosedNonces.end())
		return nullptr;

	auto& nonce   = item->second.first;
	auto& account = item->second.second;

	if (nonce.HasExpired())
	{
		m_ClosedNonces.erase(Response);
		return nullptr;
	}

	if (nonce.GetIP() != IP)
		return nullptr;

	nonce.Refresh();

	return account;
}



const std::vector<std::pair<Account::Nonce, const Account*>> Database::GetNonces()
{
	std::vector<std::pair<Account::Nonce, const Account*>> ret;

	for (auto nonce = m_OpenNonces.cbegin(); nonce != m_OpenNonces.cend();)
	{
		if (nonce->HasExpired())
			nonce = m_OpenNonces.erase(nonce);
		else
		{
			ret.push_back({ *nonce, nullptr });
			++nonce;
		}
	}

	for (auto item = m_ClosedNonces.cbegin(); item != m_ClosedNonces.cend();)
	{
		if (item->second.first.HasExpired())
			item = m_ClosedNonces.erase(item);
		else
		{
			ret.push_back({ item->second.first, item->second.second });
			++item;
		}
	}

	return ret;
}