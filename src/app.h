#pragma once
#include <string>
#include <mutex>
#include "database.h"
#include "tournament.h"
#include "account.h"
#include "imat.h"
#include "error.h"
#include "HttpServer/HttpServer.h"
#include <ZED/include/csv.h>



namespace Judoboard
{
	class Tournament;
	class Match;



	class Application
	{
	public:
		Application() : m_StartupTimestamp(Timer::GetTimestamp()) {
			m_TempTournament.EnableAutoSave(false);
			m_CurrentTournament = &m_TempTournament;
		}
		Application(uint16_t Port);
		~Application();

		bool IsRunning() const { return m_Running; }
		bool IsMaster()  const { return m_Mode == Mode::Master; }
		bool IsSlave()   const { return m_Mode == Mode::Slave; }

		Database& GetDatabase() { return m_Database; }

		bool LoadDataFromDisk();

		//Tournaments
		const Tournament* GetTournament() const { return m_CurrentTournament; }//Returns the tournament that is currently open, return null pointer if no tournament is open
		Tournament* GetTournament() { return m_CurrentTournament; }//Returns the tournament that is currently open, return null pointer if no tournament is open
		const auto& GetTournamentList() const { return m_Tournaments; }
		auto& SetTournamentList() { return m_Tournaments; }
		bool OpenTournament(uint32_t Index);
		bool CloseTournament();
		bool AddTournament(Tournament* NewTournament);
		const Tournament* FindTournament(const std::string& Name) const;
		int FindTournamentIndex(const std::string& Name) const;

		//Mats
		const std::vector<IMat*>& GetMats() const { return m_Mats; }
		std::vector<IMat*>& SetMats() { return m_Mats; }
		uint32_t FindDefaultMatID() const { if (GetDefaultMat()) return GetDefaultMat()->GetMatID(); return 0; }//If only one mat is available the ID of that mat will be returned. 0 otherwise

		IMat* GetDefaultMat() const;
		IMat* FindMat(uint32_t ID);
		const IMat* FindMat(uint32_t ID) const;
		uint32_t GetHighestMatID() const;

		bool StartLocalMat(uint32_t ID);
		bool CloseMat(uint32_t ID);

		std::vector<const Match*> GetNextMatches(uint32_t MatID) const;

		bool ConnectToMaster(const std::string& Hostname, uint16_t Port = 8080);

		void Run();
		void Shutdown() { m_Running = false; }

		//AJAX requests
		ZED::CSV Ajax_GetMats() const;

		Error Ajax_OpenMat(  const HttpServer::Request& Request);
		Error Ajax_CloseMat( const HttpServer::Request& Request);
		Error Ajax_UpdateMat(const HttpServer::Request& Request);

		ZED::CSV Ajax_ListClubs();

		ZED::CSV Ajax_Uptime();

		//Serialization
		[[deprecated]]
		ZED::CSV Mats2String() const;

		static const std::string Name;
		static const std::string Version;
		static constexpr uint16_t DefaultPort = 8080;
		static bool NoWindow;

	private:
		bool IsTournamentOpen() const { return m_CurrentTournament; }
		const Account* IsLoggedIn(const HttpServer::Request& Request) const;

		Error CheckPermission(const HttpServer::Request& Request, Account::AccessLevel AccessLevel = Account::AccessLevel::User) const;

		bool SendCommandToMaster(const std::string& URL) const;

		class ScopedLock
		{
		public:
			ScopedLock(std::mutex& Mutex) : m_Mutex(Mutex) { Mutex.lock(); }
			~ScopedLock() { m_Mutex.unlock(); }

		private:
			std::mutex& m_Mutex;
		};

		ScopedLock LockTillScopeEnd() const { return ScopedLock(m_mutex); }


		enum class Mode
		{
			Master,//This application can send commands to other applications
			Slave//This application receives commands from other applications
		} m_Mode = Mode::Master;

		std::string m_MasterHostname;//Hostname of the master server
		uint16_t m_MasterPort = 0;//Port of the master server

		HttpServer m_Server;

		Database m_Database;

		Tournament m_TempTournament;
		std::vector<Tournament*> m_Tournaments;
		Tournament* m_CurrentTournament = nullptr;//Tournament that is currently open

		std::vector<IMat*> m_Mats;//List of all mats this application is aware of

		mutable std::mutex m_mutex;

		bool m_Running = true;
		const uint32_t m_StartupTimestamp;
	};
}