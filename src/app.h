#pragma once
#include <string>
#include <mutex>
#include "database.h"
#include "tournament.h"
#include "account.h"
#include "dmf.h"
#include "imat.h"
#include "error.h"
#include "HttpServer/HttpServer.h"



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

		[[nodiscard]]
		bool IsRunning() const { return m_Running; }
		[[nodiscard]]
		bool IsMaster()  const { return m_Mode == Mode::Master; }
		[[nodiscard]]
		bool IsSlave()   const { return m_Mode == Mode::Slave; }
		[[nodiscard]]
		auto GetPort()   const { return m_Server.GetPort(); }

		[[nodiscard]]
		const Database& GetDatabase() const { return m_Database; }
		[[nodiscard]]
		Database& GetDatabase() { return m_Database; }

		bool LoadDataFromDisk();
		const Account* GetDefaultAdminAccount() const;

		//DM4
		std::string AddDM4File(const DM4& File, bool ParseOnly = false, bool* pSuccess = nullptr);

		//DMF
		std::string AddDMFFile(const DMF& File, bool ParseOnly = false, bool* pSuccess = nullptr);

		//Tournaments
		[[nodiscard]]
		const ITournament* GetTournament() const { return m_CurrentTournament; }//Returns the tournament that is currently open, return null pointer if no tournament is open
		[[nodiscard]]
		ITournament* GetTournament() { return m_CurrentTournament; }//Returns the tournament that is currently open, return null pointer if no tournament is open
		void SetTournament(ITournament* Tournament) { m_CurrentTournament = Tournament; }
		[[nodiscard]]
		const auto& GetTournamentList() const { return m_Tournaments; }
		[[nodiscard]]
		auto& SetTournamentList() { return m_Tournaments; }
		bool OpenTournament(const UUID& UUID);
		bool CloseTournament();
		bool AddTournament(Tournament* NewTournament);
		bool DeleteTournament(const UUID& UUID);
		Tournament* FindTournament(const UUID& UUID);
		const Tournament* FindTournament(const UUID& UUID) const;
		const Tournament* FindTournamentByName(const std::string& Name) const;

		//Mats
		const std::vector<IMat*>& GetMats() const { return m_Mats; }
		std::vector<IMat*>& SetMats() { return m_Mats; }
		uint32_t FindDefaultMatID() const { if (GetDefaultMat()) return GetDefaultMat()->GetMatID(); return 0; }//If only one mat is available the ID of that mat will be returned. 0 otherwise

		IMat* GetDefaultMat() const;
		IMat* FindMat(uint32_t ID);
		const IMat* FindMat(uint32_t ID) const;
		uint32_t GetHighestMatID() const;

		bool StartLocalMat(uint32_t ID = 1);
		bool CloseMat(uint32_t ID);

		std::vector<Match> GetNextMatches(uint32_t MatID) const;

		bool ConnectToMaster(const std::string& Hostname, uint16_t Port = 8080);

		void Run();
		void Shutdown() const { m_Running = false; }

		//AJAX requests

		//General
		Error Ajax_UpdatePassword(Account* Account, const HttpServer::Request& Request);

		//Mat
		std::string Ajax_GetMats() const;
		Error Ajax_OpenMat(  const HttpServer::Request& Request);
		Error Ajax_CloseMat( const HttpServer::Request& Request);
		Error Ajax_UpdateMat(const HttpServer::Request& Request);
		Error Ajax_SetFullscreen(bool Fullscreen, const HttpServer::Request& Request);

		//Commands
		Error Ajax_AddDisqualification(Fighter Whom, const HttpServer::Request& Request);
		Error Ajax_RemoveDisqualification(Fighter Whom, const HttpServer::Request& Request);
		Error Ajax_NoDisqualification(Fighter Whom, const HttpServer::Request& Request);
		Error Ajax_RemoveNoDisqualification(Fighter Whom, const HttpServer::Request& Request);

		//Schedule
		std::string Ajax_GetHansokumake() const;//Returns matches that are in progress and have a direct hansokumake

		//Clubs
		Error Ajax_AddClub(const HttpServer::Request& Request);
		std::string Ajax_ListClubs();

		//Rule sets

		//Age groups
		std::string Ajax_ListAllAgeGroups() const;

		//Match tables
		std::string Ajax_ListAllMatchTables(const HttpServer::Request& Request);
		std::string Ajax_GetParticipantsFromMatchTable(const HttpServer::Request& Request);
		std::string Ajax_GetMatchesFromMatchTable(const HttpServer::Request& Request);

		ZED::CSV Ajax_Uptime();

		//Serialization
		[[deprecated]]
		ZED::CSV Mats2String() const;

		static const std::string Name;
		static const std::string Version;
		static constexpr uint16_t DefaultPort = 8080;
		static bool NoWindow;

	private:
		void SetupHttpServer();

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

		mutable Database m_Database;

		Tournament m_TempTournament;
		std::vector<Tournament*> m_Tournaments;
		ITournament* m_CurrentTournament = nullptr;//Tournament that is currently open

		std::vector<IMat*> m_Mats;//List of all mats this application is aware of

		mutable std::mutex m_mutex;

		mutable bool m_Running = true;
		const uint32_t m_StartupTimestamp;
	};
}