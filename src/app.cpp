#include <signal.h>
#include "app.h"
#include "database.h"
#include "weightclass.h"
#include "remote_mat.h"
#include "tournament.h"
#include "remote_tournament.h"
#include "dmf.h"
#include "../ZED/include/log.h"
#include "../ZED/include/csv.h"
#include "../ZED/include/http_client.h"



using namespace Judoboard;


const std::string Application::Name = "Judoboard";
const std::string Application::Version = "0.4.1";
bool Application::NoWindow = false;



Application::Application(uint16_t Port) : m_Server(Port), m_StartupTimestamp(Timer::GetTimestamp())
{
	m_TempTournament.EnableAutoSave(false);
	m_CurrentTournament = &m_TempTournament;

	if (!m_Server.IsRunning())
		ZED::Log::Error("Could not start http server!");


	SetupHttpServer();
}



Application::~Application()
{
	if (!m_Database.Save("database.yml"))
		ZED::Log::Error("Could not save database!");

	for (auto mat : m_Mats)
		delete mat;

	m_Running = false;
	m_CurrentTournament = nullptr;

	for (auto tournament : m_Tournaments)
		delete tournament;
}



bool Application::LoadDataFromDisk()
{
	if (!m_Database.Load("database.yml"))
	{
		ZED::Log::Warn("Could not load database!");
		//return false;
	}

	//Has to be done here, otherwise it will be overwritten when loading tournament files
	auto last_tournament_name = m_Database.GetLastTournamentName();
	ZED::Log::Info("Last opened tournament was " + last_tournament_name);


	ZED::Core::Indexer([this](auto Filename) {
		Filename = Filename.substr(Filename.find_last_of(ZED::Core::Separator) + 1);
		Filename = Filename.substr(0, Filename.length() - 4);//Remove .yml
		AddTournament(new Tournament(Filename));
		return true;
		}, "tournaments");


	if (last_tournament_name.length() > 0)
	{
		auto last_tournament = FindTournamentByName(last_tournament_name);
		if (last_tournament)
			OpenTournament(*last_tournament);
	}
	else
		CloseTournament();

	return true;
}



const Account* Application::GetDefaultAdminAccount() const
{
	for (auto account : GetDatabase().GetAccounts())
	{
		if (account && account->GetAccessLevel() == Account::AccessLevel::Admin)
			return account;
	}

	//No admin account found, create the default one
	return m_Database.AddAccount(Account("admin", "1234", Account::AccessLevel::Admin));
}



std::string Application::AddDM4File(const DM4& File, bool ParseOnly, bool* pSuccess)
{
	std::string ret;

	if (!File)
	{
		ZED::Log::Warn("Invalid DM4 file");
		if (pSuccess)
			*pSuccess = false;
		return ret;
	}

	ret += "Tournament name: " + File.GetTournamentName() + "<br/>";
	ret += "Tournament date: " + File.GetTournamentDate() + "<br/>";
	ret += "<br/>";

	LockTillScopeEnd();

	for (auto club : File.GetClubs())
	{
		if (!club)
			continue;

		if (!GetDatabase().FindClubByName(club->Name))
		{
			ret += "Adding new club: " + club->Name + "<br/>";

			if (!ParseOnly)
				GetDatabase().AddClub(new Club(club->Name));
		}
	}


	Gender gender_of_participants = File.GetGender();

	for (auto dm4_judoka : File.GetParticipants())
	{
		ret += "Judoka: " + dm4_judoka.Firstname + " " + dm4_judoka.Lastname + "<br/>";

		//auto new_judoka = GetDatabase().UpdateOrAdd(dm4_judoka, ParseOnly, ret);
		Judoka* new_judoka = new Judoka(JudokaData(dm4_judoka), &GetDatabase());

		//Judoka is now added/updated

		if (!ParseOnly)
		{//Add to the current tournament
			if (File.GetClubs().size() == 1)//If there is only one
				new_judoka->SetClub(GetDatabase().FindClubByName(File.GetClubs()[0]->Name));

			GetTournament()->AddParticipant(new_judoka);
		}
	}

	if (pSuccess)
		*pSuccess = true;

	return ret;
}



std::string Application::AddDMFFile(const DMF& File, bool ParseOnly, bool* pSuccess)
{
	std::string ret;

	if (!File)
	{
		ZED::Log::Warn("Invalid DMF file");
		if (pSuccess)
			*pSuccess = false;
		return ret;
	}

	ret += "Tournament name: " + File.GetTournamentName() + "<br/>";
	ret += "Tournament date: " + File.GetTournamentDate() + "<br/>";


	auto club = GetDatabase().FindClubByName(File.GetClub().Name);

	if (!club)
	{
		ZED::Log::Warn("Could not find club");

		ret += "Adding new club: " + File.GetClub().Name + "<br/>";

		if (!ParseOnly)
			GetDatabase().AddClub(new Club(File.GetClub().Name));

		club = GetDatabase().FindClubByName(File.GetClub().Name);
	}

	for (auto dmf_judoka : File.GetParticipants())
	{
		ret += "Judoka: " + dmf_judoka.Firstname + " " + dmf_judoka.Lastname + "<br/>";

		//auto new_judoka = GetDatabase().UpdateOrAdd(JudokaData(dmf_judoka), ParseOnly, ret);	

		//Judoka is now added/updated

		if (!ParseOnly)
		{//Add to the current tournament
			GetTournament()->AddParticipant(new Judoka(JudokaData(dmf_judoka), &GetDatabase()));
		}
	}

	if (pSuccess)
		*pSuccess = true;

	return ret;
}



bool Application::OpenTournament(const UUID& UUID)
{
	if (!CloseTournament())
		return false;

	m_CurrentTournament = FindTournament(UUID);

	if (m_CurrentTournament)
	{
		m_Database.SetLastTournamentName(m_CurrentTournament->GetName());
		ZED::Log::Info("Opened tournament " + m_CurrentTournament->GetName());
	}

	return true;
}



bool Application::CloseTournament()
{
	if (m_CurrentTournament)
		if (!m_CurrentTournament->CanCloseTournament())
			return false;

	//Re-open temporary tournament
	m_CurrentTournament = &m_TempTournament;
	m_Database.SetLastTournamentName("");
	return true;
}



const Account* Application::IsLoggedIn(const HttpServer::Request& Request) const
{
	if (!IsRunning())
		return nullptr;	

	auto header = Request.m_RequestInfo.FindHeader("Cookie");
	if (header)
	{
		auto token = HttpServer::DecodeURLEncoded(header->Value, "token");
		if (token == "test")//TODO remove this, i.e. replace this with a secure token (from a specific account maybe)
			return GetDefaultAdminAccount();

		auto value = HttpServer::DecodeURLEncoded(header->Value, "session");
		return m_Database.IsLoggedIn(Request.m_RequestInfo.RemoteIP, value);
	}

	return nullptr;
}



Error Application::CheckPermission(const HttpServer::Request& Request, Account::AccessLevel AccessLevel) const
{
	if (!IsRunning())
		return Error::Type::ApplicationShuttingDown;

	auto header = Request.m_RequestInfo.FindHeader("Cookie");
	if (!header)
		return Error::Type::NotLoggedIn;

	auto value = HttpServer::DecodeURLEncoded(header->Value, "session");

	LockTillScopeEnd();
	auto account = m_Database.IsLoggedIn(Request.m_RequestInfo.RemoteIP, value);

	if (!account || account->GetAccessLevel() < AccessLevel)
		return Error::Type::NotEnoughPermissions;

	return Error();
}



IMat* Application::GetDefaultMat() const
{
	for (auto mat : m_Mats)
		if (mat && mat->GetType() == IMat::Type::LocalMat)
			return mat;

	if (!m_Mats.empty())
		return m_Mats[0];
	
	return nullptr;
}



IMat* Application::FindMat(uint32_t ID)
{
	for (auto mat : m_Mats)
		if (mat && mat->GetMatID() == ID)
			return mat;

	return nullptr;
}



const IMat* Application::FindMat(uint32_t ID) const
{
	for (auto mat : m_Mats)
		if (mat && mat->GetMatID() == ID)
			return mat;

	return nullptr;
}



uint32_t Application::GetHighestMatID() const
{
	uint32_t ret = 0;

	for (auto mat : m_Mats)
		if (mat && mat->GetMatID() > ret)
			ret = mat->GetMatID();

	return ret;
}



bool Application::CloseMat(uint32_t ID)
{
	LockTillScopeEnd();

	for (auto it = m_Mats.begin(); it != m_Mats.end(); ++it)
	{
		if (*it && (*it)->GetMatID() == ID && (*it)->Close())
		{
			if ((*it)->GetType() == IMat::Type::VirtualMat)
			{
				delete* it;
				it = m_Mats.erase(it);
			}

			return true;
		}
	}

	return false;
}



bool Application::StartLocalMat(uint32_t ID)
{
	ZED::Log::Info("Starting local mat");

	LockTillScopeEnd();

	for (; true; ID++)
	{
		bool is_ok = true;

		for (auto mat : m_Mats)
		{
			if (mat && mat->GetType() == IMat::Type::LocalMat && mat->IsOpen())
			{
				ZED::Log::Warn("A local mat is already open, can not open another mat");
				return false;
			}

			if (mat && mat->GetMatID() == ID)
			{
				ZED::Log::Warn("There is already a mat open with the requested id");
				is_ok = false;
				break;
				//return false;
			}
		}

		if (is_ok)
			break;
	}

	Mat* new_mat = new Mat(ID, this);
	m_Mats.emplace_back(new_mat);

	ZED::Log::Info("New local mat has been created with ID=" + std::to_string(ID));

	new_mat->SetIpponStyle(GetDatabase().GetIpponStyle());
	new_mat->SetTimerStyle(GetDatabase().GetTimerStyle());
	new_mat->SetNameStyle(GetDatabase().GetNameStyle());

	if (IsSlave())
		SendCommandToMaster("/ajax/master/mat_available?port=" + std::to_string(m_Server.GetPort()));

	return true;
}



std::vector<Match> Application::GetNextMatches(uint32_t MatID) const
{
	LockTillScopeEnd();

	if (!GetTournament())
	{
		std::vector<Match> empty;
		return empty;
	}
	return GetTournament()->GetNextMatches(MatID);
}



bool Application::AddTournament(Tournament* NewTournament)
{
	if (!NewTournament)
	{
		ZED::Log::Warn("Invalid tournament");
		return false;
	}

	if (FindTournamentByName(NewTournament->GetName()))
	{
		ZED::Log::Warn("Tournament with this name already exists");
		return false;
	}

	m_Tournaments.emplace_back(NewTournament);

	if (NewTournament->GetStatus() == Status::Scheduled)//Fresh new tournament
	{//Try to open right away
		if (!m_CurrentTournament || m_CurrentTournament->CanCloseTournament())
			OpenTournament(*NewTournament);
	}

	return true;
}



bool Application::DeleteTournament(const UUID& UUID)
{
	for (auto it = m_Tournaments.begin(); it != m_Tournaments.end(); ++it)
	{
		if (*it && (*it)->GetUUID() == UUID)
		{
			const auto filename = "tournaments/" + (*it)->GetName() + ".yml";

			delete *it;
			m_Tournaments.erase(it);

			return ZED::Core::RemoveFile(filename);
		}
	}

	return false;
}



Tournament* Application::FindTournament(const UUID& UUID)
{
	for (auto tournament : m_Tournaments)
		if (tournament && tournament->GetUUID() == UUID)
			return tournament;
	return nullptr;
}



const Tournament* Application::FindTournament(const UUID& UUID) const
{
	for (auto tournament : m_Tournaments)
		if (tournament && tournament->GetUUID() == UUID)
			return tournament;
	return nullptr;
}



Tournament* Application::FindTournamentByName(const std::string& Name)
{
	for (auto tournament : m_Tournaments)
		if (tournament && tournament->GetName() == Name)
			return tournament;
	return nullptr;
}



const Tournament* Application::FindTournamentByName(const std::string& Name) const
{
	for (auto tournament : m_Tournaments)
		if (tournament && tournament->GetName() == Name)
			return tournament;
	return nullptr;
}



bool Application::ConnectToMaster(const std::string& Hostname, uint16_t Port)
{
	ZED::HttpClient client(Hostname, Port);
	auto response = (std::string)client.GET("/ajax/master/connection_test");
	
	if (response != "ok")
	{
		ZED::Log::Info("Could not connect to master server: " + Hostname + ":" + std::to_string(Port));
		return false;
	}

	SetTournament(new RemoteTournament(Hostname, Port));

	m_Mode = Mode::Slave;
	m_MasterHostname = Hostname;
	m_MasterPort = Port;
	return true;
}



bool Application::SendCommandToMaster(const std::string& URL) const
{
	if (m_Mode != Mode::Slave)
	{
		ZED::Log::Warn("Command " + URL + " sent, but not in slave mode");
		return false;
	}

	ZED::Log::Debug("Command " + URL + " sent to master");

	ZED::HttpClient client(m_MasterHostname, m_MasterPort);
	auto packet = client.GET(URL);
	return packet.body == "ok";
}



void Application::Run()
{
	if (!m_Server.IsRunning())
	{
		ZED::Log::Warn("Server is not running");
		return;
	}

	uint32_t runtime = 0;
	while (IsRunning())
	{
		for (auto it = m_Mats.begin(); it != m_Mats.end();)
		{
			if (*it && !(*it)->IsConnected())
			{
				delete *it;
				it = m_Mats.erase(it);
			}
			else
				++it;
		}

		ZED::Core::Pause(10 * 1000);
		runtime += 10;

		if (runtime % (5 * 60) == 0)//Every 5 minutes
		{
			if (!m_Database.Save())
				ZED::Log::Warn("Auto saving of database failed!");
		}
	}

	ZED::Log::Info("Closing application");
}