#include <fstream>
#include <random>
#include <ZED/include/log.h>
#include <ZED/include/sha512.h>
#include "database.h"



using namespace Judoboard;



void Database::Reset()
{
	StandingData::Reset();

	for (auto* account : m_Accounts)
		delete account;

	m_Accounts.clear();
	m_OpenNonces.clear();
	m_ClosedNonces.clear();
}



bool Database::Load(const std::string& Filename)
{
	m_Filename = Filename;
	std::ifstream File(Filename, std::ios::binary);

	if (!File)
	{
		ZED::Log::Error("Could not open file " + Filename);
		return false;
	}

	Reset();

	ZED::CSV csv(File);

	uint32_t version = 0;
	csv >> version;

	if (version != 1)
	{
		ZED::Log::Error("File format is too new for this application to read. Please update this application");
		return false;
	}

	StandingData::operator<<(csv);

	uint32_t accountCount = 0;
	csv >> accountCount;

	for (uint32_t i = 0; i < accountCount; i++)
	{
		std::string username, password;

		csv >> username >> password;

		Account::AccessLevel accessLevel = Account::AccessLevel::None;
		csv >> accessLevel;

		AddAccount(Account(username, password, accessLevel));
	}

	if (m_RuleSets.empty())//No rule set defined
		AddRuleSet(new RuleSet);//Add default rule set

	return true;
}



bool Database::Save(const std::string& Filename) const
{
	if (Filename[0] == '\0')
		return false;

	m_Filename = Filename;
	std::ofstream File(Filename, std::ios::binary);

	if (!File)
		return false;

	ZED::CSV csv;

	csv << 1;//Version
	
	StandingData::operator>>(csv);

	csv << m_Accounts.size();

	for (auto account : m_Accounts)
	{
		if (account)
		{
			csv << account->GetUsername();
			csv << account->GetPassword();
			csv << account->GetAccessLevel();
		}
	}

	csv >> File;
	return true;
}



void Database::AddAccount(const Account& NewAccount)
{
	m_Accounts.push_back(new Account(NewAccount));
	Save(m_Filename);
}



const Account* Database::FindAccount(const std::string& Username) const
{
	for (auto& account : m_Accounts)
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

	for (auto nonce = m_OpenNonces.cbegin(); nonce != m_OpenNonces.cend();)
	{
		if (nonce->HasExpired())
			nonce = m_OpenNonces.erase(nonce);

		else if (account->Verify(*nonce, Response))
		{
			m_ClosedNonces.insert({ Response, {*nonce, account} });
			m_OpenNonces.erase(nonce);

			return true;
		}
		else
			++nonce;
	}

	return false;
}



const Account* Database::CheckLogin(uint32_t IP, const std::string& Response) const
{
	auto item = m_ClosedNonces.find(Response);

	if (item == m_ClosedNonces.end())
		return nullptr;

	auto& nonce = item->second.first;
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