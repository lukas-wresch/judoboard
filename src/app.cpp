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
const std::string Application::Version = "0.4.4";
bool Application::NoWindow = false;



Application::Application(uint16_t Port) : m_Server(Port), m_StartupTimestamp(Timer::GetTimestamp())
{
	m_TempTournament.EnableAutoSave(false);
	m_TempTournament.SetDefaultRuleSet(new RuleSet);//Take default rule set as rule set for temporary tournaments
	m_CurrentTournament = &m_TempTournament;

	std::string token = (std::string)ID::GenerateUUID();
	m_SecurityToken   = token.substr(0, 32);

	if (!m_Server.IsRunning())
	{
		ZED::Log::Error("Could not start http server!");
		Shutdown();
	}
	else
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

	auto guard = LockWriteForScope();

	for (auto club : File.GetClubs())
	{
		if (!club)
			continue;

		if (!GetTournament()->FindClubByName(club->Name))
			if (!GetDatabase().FindClubByName(club->Name))
		{
			ret += "Adding new club: " + club->Name + "<br/>";

			if (!ParseOnly)
				GetTournament()->GetDatabase().AddClub(new Club(club->Name));
		}
	}


	Gender gender_of_participants = File.GetGender();

	for (auto dm4_judoka : File.GetParticipants())
	{
		ret += "Judoka: " + dm4_judoka.Firstname + " " + dm4_judoka.Lastname + "<br/>";

		//Judoka is now added/updated

		if (!ParseOnly)
		{//Add to the current tournament
			Judoka* new_judoka = new Judoka(JudokaData(dm4_judoka), &GetTournament()->GetDatabase());

			if (!new_judoka->GetClub() && File.GetClubs().size() == 1)//If there is only one
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

	auto guard = LockWriteForScope();

	auto club = GetTournament()->FindClubByName(File.GetClub().Name);

	if (!club)
		club = GetDatabase().FindClubByName(File.GetClub().Name);

	if (!club)
	{
		ZED::Log::Warn("Could not find club");

		ret += "Adding new club: " + File.GetClub().Name + "<br/>";

		club = new Club(File.GetClub().Name);
	}

	for (auto dmf_judoka : File.GetParticipants())
	{
		ret += "Judoka: " + dmf_judoka.Firstname + " " + dmf_judoka.Lastname + "<br/>";

		//Judoka is now added/updated

		if (!ParseOnly)
		{//Add to the current tournament
			auto new_judoka = new Judoka(JudokaData(dmf_judoka), nullptr);
			new_judoka->SetClub(club);
			GetTournament()->AddParticipant(new_judoka);
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

	auto tournament = FindTournament(UUID);
	if (!tournament)
		return false;

	auto guard = LockWriteForScope();

	m_CurrentTournament = tournament;

	//Restart ongoing matches
	for (auto mat : m_Mats)
	{
		auto match = tournament->GetNextOngoingMatch(mat->GetMatID());
		mat->StartMatch(match, true);
	}

	m_Database.SetLastTournamentName(m_CurrentTournament->GetName());
	ZED::Log::Info("Opened tournament " + m_CurrentTournament->GetName());

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



bool Application::IsLoggedIn(const HttpServer::Request& Request) const
{
	if (!IsRunning())
		return false;

	if (IsSlave())
		if (CheckAccessToken(HttpServer::DecodeURLEncoded(Request.m_Query, "token")))
			return true;

	auto header = Request.m_RequestInfo.FindHeader("Cookie");
	if (header)
	{
		auto value = HttpServer::DecodeURLEncoded(header->Value, "session");
		return m_Database.IsLoggedIn(Request.m_RequestInfo.RemoteIP, value);
	}

	return false;
}



Error Application::CheckPermission(const HttpServer::Request& Request, Account::AccessLevel AccessLevel, const Account** pAccount) const
{
	if (!IsRunning())
		return Error::Type::ApplicationShuttingDown;

	auto header = Request.m_RequestInfo.FindHeader("Cookie");
	if (!header)
		return Error::Type::NotLoggedIn;

	auto value = HttpServer::DecodeURLEncoded(header->Value, "session");

	auto guard = LockReadForScope();

	auto account = m_Database.IsLoggedIn(Request.m_RequestInfo.RemoteIP, value);

	if (!account || account->GetAccessLevel() < AccessLevel)
		return Error::Type::NotEnoughPermissions;

	if (pAccount)
		*pAccount = account;

	return Error();
}



IMat* Application::GetLocalMat() const
{
	auto guard = LockReadForScope();

	for (auto mat : m_Mats)
		if (mat && mat->GetType() == IMat::Type::LocalMat)
			return mat;

	if (!m_Mats.empty())
		return m_Mats[0];
	
	return nullptr;
}



IMat* Application::FindMat(uint32_t ID)
{
	auto guard = LockReadForScope();

	for (auto mat : m_Mats)
		if (mat && mat->GetMatID() == ID)
			return mat;

	return nullptr;
}



const IMat* Application::FindMat(uint32_t ID) const
{
	auto guard = LockReadForScope();

	for (auto mat : m_Mats)
		if (mat && mat->GetMatID() == ID)
			return mat;

	return nullptr;
}



uint32_t Application::GetHighestMatID() const
{
	uint32_t ret = 0;

	auto guard = LockReadForScope();

	for (auto mat : m_Mats)
		if (mat && mat->GetMatID() > ret)
			ret = mat->GetMatID();

	return ret;
}



bool Application::CloseMat(uint32_t ID)
{
	LockRead();
	auto mats_copy = m_Mats;
	UnlockRead();

	for (auto it = mats_copy.begin(); it != mats_copy.end(); ++it)
	{
		if (*it && (*it)->GetMatID() == ID && (*it)->Close())//This also requires an application lock
		{
			if ((*it)->GetType() == IMat::Type::VirtualMat)
			{
				delete *it;
				it = m_Mats.erase(it);
			}

			LockWrite();
			m_Mats = std::move(mats_copy);
			UnlockWrite();

			return true;
		}
	}

	return false;
}



IMat* Application::StartLocalMat(uint32_t ID)
{
	ZED::Log::Info("Starting local mat");

	LockWrite();

	for (; true; ID++)
	{
		bool is_ok = true;

		for (auto mat : m_Mats)
		{
			/*if (mat && mat->GetType() == IMat::Type::LocalMat && mat->IsOpen())
			{
				ZED::Log::Warn("A local mat is already open, can not open another mat");
				return false;
			}*/

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

	new_mat->SetIpponStyle(GetDatabase().GetIpponStyle());
	new_mat->SetTimerStyle(GetDatabase().GetTimerStyle());
	new_mat->SetNameStyle(GetDatabase().GetNameStyle());

	//Is there a match already started on that mat?
	if (GetTournament())
	{
		auto ongoing_match = GetTournament()->GetNextOngoingMatch(ID);
		if (ongoing_match)
			new_mat->StartMatch(ongoing_match, true);
	}

	m_Mats.emplace_back(new_mat);
	UnlockWrite();

	ZED::Log::Info("New local mat has been created with ID=" + std::to_string(ID));

	if (IsSlave())
	{
		if (!RegisterMatWithMaster(new_mat))
		{
			ZED::Log::Error("Could not register mat with master");
			return nullptr;
		}
	}

	return new_mat;
}



bool Application::RegisterMatWithMaster(IMat* Mat)
{
	if (!Mat)
		return false;

	auto yaml = YAML::Load(SendCommandToMaster("/ajax/master/mat_available?id=" + std::to_string(Mat->GetMatID()) + "&port=" + std::to_string(m_Server.GetPort())));

	if (!yaml || !yaml.IsMap())
	{
		ZED::Log::Error("Could not register mat with master");
		return false;
	}

	auto guard = LockWriteForScope();

	if (yaml["id"])
		Mat->SetMatID(yaml["id"].as<uint32_t>());
	if (yaml["language"])
		Localizer::SetLanguage((Language)yaml["language"].as<int32_t>());
	if (yaml["ippon_style"])
		Mat->SetIpponStyle((IMat::IpponStyle)yaml["ippon_style"].as<int32_t>());
	if (yaml["timer_style"])
		Mat->SetTimerStyle((IMat::TimerStyle)yaml["timer_style"].as<int32_t>());
	if (yaml["name_style"])
		Mat->SetNameStyle((NameStyle)yaml["name_style"].as<int32_t>());

	if (yaml["token"])//Receive access token from master
		SetAccessToken(yaml["token"].as<std::string>());
	else
		return false;

	return true;
}



std::vector<Match> Application::GetNextMatches(uint32_t MatID, bool& Success) const
{
	if (!TryReadLock())//Can we get a lock?
	{
		std::vector<Match> empty;
		Success = false;
		return empty;
	}

	//Mutex is now locked

	Success = true;

	if (!GetTournament())
	{
		std::vector<Match> empty;
		UnlockRead();
		return empty;
	}

	auto ret = GetTournament()->GetNextMatches(MatID);
	UnlockRead();

	return ret;
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

	auto guard = LockWriteForScope();
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
			if ((*it)->IsReadonly())
				return false;

			const auto filename = "tournaments/" + (*it)->GetName() + ".yml";

			LockWrite();
			delete *it;
			m_Tournaments.erase(it);
			UnlockWrite();

			return ZED::Core::RemoveFile(filename);
		}
	}

	return false;
}



Tournament* Application::FindTournament(const UUID& UUID)
{
	auto guard = LockReadForScope();

	for (auto tournament : m_Tournaments)
		if (tournament && tournament->GetUUID() == UUID)
			return tournament;
	return nullptr;
}



const Tournament* Application::FindTournament(const UUID& UUID) const
{
	auto guard = LockReadForScope();

	for (auto tournament : m_Tournaments)
		if (tournament && tournament->GetUUID() == UUID)
			return tournament;
	return nullptr;
}



Tournament* Application::FindTournamentByName(const std::string& Name)
{
	auto guard = LockReadForScope();

	for (auto tournament : m_Tournaments)
		if (tournament && tournament->GetName() == Name)
			return tournament;
	return nullptr;
}



const Tournament* Application::FindTournamentByName(const std::string& Name) const
{
	auto guard = LockReadForScope();

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



std::string Application::SendCommandToMaster(const std::string& URL) const
{
	if (m_Mode != Mode::Slave)
	{
		ZED::Log::Warn("Command " + URL + " sent, but not in slave mode");
		return (Error)Error::Type::InternalError;
	}

	ZED::Log::Debug("Command " + URL + " sent to master");

	ZED::HttpClient client(m_MasterHostname, m_MasterPort);
	auto packet = client.GET(URL);
	return packet.body;
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
		LockRead();
		for (auto it = m_Mats.begin(); it != m_Mats.end();)
		{
			if (*it && !(*it)->IsConnected())
			{
				LockWrite();
				auto mat_to_be_deleted = *it;
				it = m_Mats.erase(it);
				UnlockWrite();

				ZED::Core::Pause(10 * 1000);
				runtime += 10;

				delete mat_to_be_deleted;
				break;
			}
			else
				++it;
		}

		if (GetTournament() && GetTournament()->IsLocal())
		{
			Tournament* tournament = (Tournament*)GetTournament();
			if (tournament->IsAutoSave() && tournament->TimeSinceLastSave() >= 10 * 60 * 1000)//10 minutes
			{
				ZED::Log::Info("Tournament auto-saved");
				if (!tournament->Save())
					ZED::Log::Error("Failed to save!");
			}
		}
		UnlockRead();

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