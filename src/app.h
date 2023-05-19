#pragma once
#include <string>
#include <mutex>
#include "../ZED/include/read_write_mutex.h"
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



	class Application : public ZED::RecursiveReadWriteMutex//TODO should be private/or private member
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
		Tournament* FindTournamentByName(const std::string& Name);
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

		std::vector<Match> GetNextMatches(uint32_t MatID, bool& Success) const;

		bool ConnectToMaster(const std::string& Hostname, uint16_t Port = 8080);

		void Run();
		void Shutdown() const { m_Running = false; }

		//AJAX requests

		//General
		Error Ajax_UpdatePassword(Account* Account, const HttpServer::Request& Request);

		Error Ajax_EditMatch(const HttpServer::Request& Request);

		//Tournaments
		Error Ajax_AddTournament(const HttpServer::Request& Request);
		Error Ajax_EditTournament(const HttpServer::Request& Request);
		std::string Ajax_GetTournament(const HttpServer::Request& Request);
		std::string Ajax_ListTournaments();
		Error Ajax_SwapMatchesOfTournament(const HttpServer::Request& Request);

		//Mat
		std::string Ajax_GetMats() const;
		Error Ajax_OpenMat(  const HttpServer::Request& Request);
		Error Ajax_CloseMat( const HttpServer::Request& Request);
		Error Ajax_PauseMat( const HttpServer::Request& Request);
		Error Ajax_UpdateMat(const HttpServer::Request& Request);
		Error Ajax_SetFullscreen(bool Fullscreen, const HttpServer::Request& Request);

		std::string Ajax_GetNamesOnMat(const HttpServer::Request& Request);

		//Commands
		Error Ajax_AddDisqualification(Fighter Whom, const HttpServer::Request& Request);
		Error Ajax_RemoveDisqualification(Fighter Whom, const HttpServer::Request& Request);
		Error Ajax_NoDisqualification(Fighter Whom, const HttpServer::Request& Request);
		Error Ajax_RemoveNoDisqualification(Fighter Whom, const HttpServer::Request& Request);

		//Schedule
		Error Ajax_MoveMatchUp(const HttpServer::Request& Request);
		Error Ajax_MoveMatchDown(const HttpServer::Request& Request);
		std::string Ajax_GetHansokumake() const;//Returns matches that are in progress and have a direct hansokumake

		//Judoka
		Error Ajax_AddJudoka(const HttpServer::Request& Request);
		std::string Ajax_GetJudoka(const HttpServer::Request& Request);
		Error Ajax_EditJudoka(const HttpServer::Request& Request);
		Error Ajax_ImportJudoka(const HttpServer::Request& Request);
		Error Ajax_DeleteJudoka(const HttpServer::Request& Request);

		//Clubs
		Error Ajax_AddClub(const HttpServer::Request& Request);
		std::string Ajax_GetClub(const HttpServer::Request& Request);
		Error Ajax_EditClub(const HttpServer::Request& Request);
		std::string Ajax_ListClubs(const HttpServer::Request& Request);
		Error Ajax_DeleteClub(const HttpServer::Request& Request);

		//Associations
		std::string Ajax_ListAssociations(const HttpServer::Request& Request);

		//Rule sets
		Error Ajax_AddRuleSet(const HttpServer::Request& Request);
		Error Ajax_EditRuleSet(const HttpServer::Request& Request);

		//Age groups
		Error Ajax_AddAgeGroup(const HttpServer::Request& Request);
		Error Ajax_EditAgeGroup(const HttpServer::Request& Request);
		Error Ajax_ImportAgeGroup(const HttpServer::Request& Request);
		std::string Ajax_GetAgeGroup(const HttpServer::Request& Request) const;
		std::string Ajax_ListAllAgeGroups() const;
		Error Ajax_DeleteAgeGroup(const HttpServer::Request& Request);

		//Match tables
		Error Ajax_AddMatchTable(HttpServer::Request Request);
		Error Ajax_EditMatchTable(const HttpServer::Request& Request);
		std::string Ajax_GetMatchTable(const HttpServer::Request& Request);
		std::string Ajax_ListAllMatchTables(const HttpServer::Request& Request);
		std::string Ajax_GetParticipantsFromMatchTable(const HttpServer::Request& Request);
		std::string Ajax_GetMatchesFromMatchTable(const HttpServer::Request& Request);
		Error Ajax_SetStartPosition(const HttpServer::Request& Request);

		//Lots
		Error Ajax_PerformLottery();
		std::string Ajax_GetLotteryTier();
		Error Ajax_SetLotteryTier(const HttpServer::Request& Request);
		std::string Ajax_ListLots();

		//Config
		std::string Ajax_GetSetup();
		Error Ajax_SetSetup(const HttpServer::Request& Request);
		std::string Ajax_Execute(const HttpServer::Request& Request);

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
			ScopedLock(std::recursive_mutex& Mutex) : m_Mutex(Mutex) { Mutex.lock(); }
			~ScopedLock() { m_Mutex.unlock(); }

		private:
			std::recursive_mutex& m_Mutex;
		};

		/*[[nodiscard]]
		ScopedLock LockTillScopeEnd() const { return ScopedLock(m_mutex); }
		void Lock()    const { m_mutex.lock(); }
		void Unlock()  const { m_mutex.unlock(); }
		bool TryLock() const { return m_mutex.try_lock(); }*/


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

		//mutable std::recursive_mutex m_mutex;
		//ZED::ReadWriteMutex m_mutex;

		mutable volatile bool m_Running = true;
		const uint32_t m_StartupTimestamp;
	};
}