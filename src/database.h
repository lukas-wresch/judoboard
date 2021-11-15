#pragma once
#include <string>
#include <unordered_map>
#include "judoka.h"
#include "account.h"
#include "standing_data.h"



namespace Judoboard
{
	class Database : public StandingData
	{
	public:
		Database() {
			Reset();
			AddRuleSet(new RuleSet);//Add default rule set
		}

		void Reset();
		bool Load(const std::string& Filename);
		bool Save(const std::string& Filename) const;
		bool Save() const {
			if (!m_AutoSave) return false;
			return Save(m_Filename);
		}

		void EnableAutoSave(bool Enable = true) { m_AutoSave = Enable; }

		//Judoka
		Judoka* UpdateOrAdd(const MD5::Participant& NewJudoka);
		Judoka* UpdateOrAdd(const DM4::Participant& NewJudoka, bool ParseOnly, std::string& Output);

		//Accounts
		const Account* AddAccount(const Account& NewAccount);
		size_t GetNumAccounts() const { return m_Accounts.size(); }
		const Account* GetAccount(size_t Index) const { if (Index >= m_Accounts.size()) return nullptr; return m_Accounts[Index]; }
		Account* SetAccount(size_t Index) { if (Index >= m_Accounts.size()) return nullptr; return m_Accounts[Index]; }
		const std::vector<Account*>& GetAccounts() const { return m_Accounts; }
		const Account* FindAccount(const std::string& Username) const;
		bool DeleteAccount(const std::string& Username);

		//Logins
		const Account::Nonce& CreateNonce(uint32_t IP, uint16_t RemotePort);
		bool DoLogin(const std::string& Username, uint32_t IP, const std::string& Response);
		const Account* IsLoggedIn(uint32_t IP, const std::string& Response) const;
		const std::vector<std::pair<Account::Nonce, const Account*>> GetNonces();

	private:
		mutable std::string m_Filename;
		bool m_AutoSave = true;

		std::vector<Account*> m_Accounts;
		std::vector<Account::Nonce> m_OpenNonces;
		mutable std::unordered_map<std::string, std::pair<Account::Nonce, const Account*>> m_ClosedNonces;
	};
}