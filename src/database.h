#pragma once
#include <string>
#include <unordered_map>
#include "judoka.h"
#include "account.h"
#include "standing_data.h"
#include "mat.h"



namespace Judoboard
{
	class Database : public StandingData
	{
	public:
		Database() {
			Reset();
		}
		~Database() {
			Reset();
		}

		void Reset();
		bool Load(const std::string& Filename);
		bool Save(const std::string& Filename) const;
		bool Save() const {
			if (!m_AutoSave) return false;
			return Save(m_Filename);
		}

		void EnableAutoSave(bool Enable = true) { m_AutoSave = Enable; }

		auto GetLastTournamentName() const { return m_CurrentTournament; }
		void SetLastTournamentName(const std::string& Name) { m_CurrentTournament = Name; }

		auto GetServerPort() const { return m_Port; }
		void SetServerPort(uint16_t NewPort) { m_Port = NewPort; }

		//Judoka
		Judoka* UpdateOrAdd(const JudokaData& NewJudoka, bool ParseOnly, std::string& Output);

		//Accounts
		const Account* AddAccount(const Account& NewAccount) { return AddAccount(new Account(NewAccount)); }
		const Account* AddAccount(Account* NewAccount);
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

		//Config
		Mat::IpponStyle GetIpponStyle() const { return m_DefaultIpponStyle; }
		Mat::TimerStyle GetTimerStyle() const { return m_DefaultTimerStyle; }
		NameStyle       GetNameStyle()  const { return m_DefaultNameStyle; }

		void SetIpponStyle(Mat::IpponStyle NewStyle) { m_DefaultIpponStyle = NewStyle; }
		void SetTimerStyle(Mat::TimerStyle NewStyle) { m_DefaultTimerStyle = NewStyle; }
		void SetNameStyle(NameStyle NewStyle) { m_DefaultNameStyle = NewStyle; }

	private:
		mutable std::string m_Filename;
		bool m_AutoSave = true;

		std::string m_CurrentTournament;//Name of tournament that is currently open
		uint16_t m_Port = 8080;//Port of the webserver

		//Mat default settings
		Mat::IpponStyle m_DefaultIpponStyle = Mat::IpponStyle::DoubleDigit;
		Mat::TimerStyle m_DefaultTimerStyle = Mat::TimerStyle::OnlySeconds;
		NameStyle       m_DefaultNameStyle  = NameStyle::FamilyName;

		std::vector<Account*> m_Accounts;
		std::vector<Account::Nonce> m_OpenNonces;
		mutable std::unordered_map<std::string, std::pair<Account::Nonce, const Account*>> m_ClosedNonces;
	};
}