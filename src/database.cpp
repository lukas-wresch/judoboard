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

	m_CurrentTournament.clear();

	for (auto account : m_Accounts)
		delete account;

	m_Accounts.clear();
	m_OpenNonces.clear();
	m_ClosedNonces.clear();

	//No rule set defined and no age groups defined
	if (m_RuleSets.empty() && m_AgeGroups.empty())
	{
		auto children = new RuleSet(Localizer::Translate("Children"),		2*60, 0,    20, 10, false, false, false, 2*60);
		auto youth    = new RuleSet(Localizer::Translate("Youth") + " U13", 3*60, 0,    20, 10, false, false, false, 3*60);
		auto youth2   = new RuleSet(Localizer::Translate("Youth") + " U15", 3*60, 3*60, 20, 10, false, false, false, 3*60);
		auto adults   = new RuleSet(Localizer::Translate("Adults"),         4*60, -1,   20, 10, false, false, false, 4*60);

		AddRuleSet(children);
		AddRuleSet(youth);
		AddRuleSet(adults);

		AddAgeGroup(new AgeGroup("U11", 8,  10, children));
		AddAgeGroup(new AgeGroup("U13", 10, 12, youth));
		AddAgeGroup(new AgeGroup("U15", 12, 14, youth2));
		AddAgeGroup(new AgeGroup("U18", 15, 17, adults));
		AddAgeGroup(new AgeGroup("U21", 17, 20, adults));
		AddAgeGroup(new AgeGroup(Localizer::Translate("Seniors"), 17, 0, adults));
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

	if (yaml["last_tournament_name"])
		m_CurrentTournament = yaml["last_tournament_name"].as<std::string>();
	if (yaml["language"])
		Localizer::SetLanguage((Language)yaml["language"].as<int>());
	if (yaml["port"])
		SetServerPort(yaml["port"].as<int>());
	if (yaml["ippon_style"])
		SetIpponStyle((Mat::IpponStyle)yaml["ippon_style"].as<int>());
	if (yaml["timer_style"])
		SetTimerStyle((Mat::TimerStyle)yaml["timer_style"].as<int>());
	if (yaml["name_style"])
		SetNameStyle((NameStyle)yaml["name_style"].as<int>());

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

	yaml << YAML::Key << "last_tournament_name" << YAML::Value << m_CurrentTournament;
	yaml << YAML::Key << "language" << YAML::Value << (int)Localizer::GetLanguage();
	yaml << YAML::Key << "port" << YAML::Value << GetServerPort();
	yaml << YAML::Key << "ippon_style" << YAML::Value << (int)GetIpponStyle();
	yaml << YAML::Key << "timer_style" << YAML::Value << (int)GetTimerStyle();
	yaml << YAML::Key << "name_style"  << YAML::Value << (int)GetNameStyle();
	
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



Judoka* Database::UpdateOrAdd(const JudokaData& NewJudoka, bool ParseOnly, std::string& Output)
{
	auto old_judoka = FindJudoka_ExactMatch(NewJudoka);

	if (old_judoka)//Exact match
		return old_judoka;

	else//No exact match
	{
		/*old_judoka = FindJudoka_SameName(NewJudoka);

		if (old_judoka)//Found someone with the right name but incorrect club/birthyear
		{
			Output += "Updating information of judoka: " + old_judoka->GetName(NameStyle::GivenName) + "<br/>";

			if (!ParseOnly)
			{
				if (FindClubByName(NewJudoka.m_ClubName))
					old_judoka->SetClub(FindClubByName(NewJudoka.m_ClubName));
				if (NewJudoka.m_Birthyear > 0)
					old_judoka->SetBirthyear(NewJudoka.m_Birthyear);
				if (NewJudoka.m_Weight > 0)
					old_judoka->SetWeight(NewJudoka.m_Weight);
			}

			return old_judoka;
		}*/

		//else//We don't have a judoka with this name
		{
			//Add to database
			Output += "Adding judoka: " + NewJudoka.m_Firstname + " " + NewJudoka.m_Lastname + "<br/>";

			if (!ParseOnly)
			{
				auto new_judoka = new Judoka(NewJudoka, this);
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