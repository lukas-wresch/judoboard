#include "app.h"
#include "database.h"
#include "weightclass.h"
//#include "virtual_mat.h"
#include "remote_mat.h"
#include "tournament.h"
#include "remote_tournament.h"
#include "../ZED/include/log.h"
#include "../ZED/include/csv.h"
#include "../ZED/include/http_client.h"



using namespace Judoboard;


const std::string Application::Name = "Judoboard";
const std::string Application::Version = "0.2.8";
bool Application::NoWindow = false;



Application::Application(uint16_t Port) : m_Server(Port), m_StartupTimestamp(Timer::GetTimestamp())
{
	m_TempTournament.EnableAutoSave(false);
	m_CurrentTournament = &m_TempTournament;

	if (!m_Server.IsRunning())
		ZED::Log::Error("Could not start http server!");


	m_Server.RegisterResource("/", [this](auto& Request) {
		if (!IsLoggedIn(Request))
			return HttpServer::LoadFile("html/login.html");
		return HttpServer::LoadFile("html/index.html");
	});

	m_Server.RegisterResource("/style.css",  [](auto& Request) { return HttpServer::LoadFile("html/style.css"); }, HttpServer::ResourceType::CSS);
	m_Server.RegisterResource("/common.js",  [](auto& Request) { return HttpServer::LoadFile("html/common.js"); }, HttpServer::ResourceType::JavaScript);

	m_Server.RegisterResource("/jquery-latest.min.js", [](auto& Request) { return HttpServer::LoadFile("html/jquery-latest.min.js"); }, HttpServer::ResourceType::JavaScript);
	m_Server.RegisterResource("/jquery-ui.min.js",     [](auto& Request) { return HttpServer::LoadFile("html/jquery-ui.min.js"); }, HttpServer::ResourceType::JavaScript);
	m_Server.RegisterResource("/sha512.min.js",        [](auto& Request) { return HttpServer::LoadFile("html/sha512.min.js"); }, HttpServer::ResourceType::JavaScript);
	m_Server.RegisterResource("/jquery-ui.css",        [](auto& Request) { return HttpServer::LoadFile("html/jquery-ui.css"); }, HttpServer::ResourceType::CSS);

	m_Server.RegisterResource("/slideout.min.js", [](auto& Request) { return HttpServer::LoadFile("html/slideout.min.js"); }, HttpServer::ResourceType::JavaScript);
	m_Server.RegisterResource("/menu.png",        [](auto& Request) { return HttpServer::LoadFile("html/menu.png"); }, HttpServer::ResourceType::Image_PNG);

	m_Server.RegisterResource("/schedule.html",      [](auto& Request) { return HttpServer::LoadFile("html/schedule.html"); });
	m_Server.RegisterResource("/mat.html",           [](auto& Request) { return HttpServer::LoadFile("html/mat.html"); });
	m_Server.RegisterResource("/mat_configure.html", [](auto& Request) { return HttpServer::LoadFile("html/mat_configure.html"); });
	m_Server.RegisterResource("/mat_edit.html",      [](auto& Request) { return HttpServer::LoadFile("html/mat_edit.html"); });

	m_Server.RegisterResource("/participant_add.html", [](auto& Request) { return HttpServer::LoadFile("html/participant_add.html"); });

	m_Server.RegisterResource("/add_judoka.html",  [](auto& Request) { return HttpServer::LoadFile("html/add_judoka.html"); });
	m_Server.RegisterResource("/list_judoka.html", [](auto& Request) { return HttpServer::LoadFile("html/list_judoka.html"); });
	m_Server.RegisterResource("/judoka_edit.html", [](auto& Request) { return HttpServer::LoadFile("html/judoka_edit.html"); });

	m_Server.RegisterResource("/club_list.html", [](auto& Request) { return HttpServer::LoadFile("html/club_list.html"); });
	m_Server.RegisterResource("/club_add.html",  [](auto& Request) { return HttpServer::LoadFile("html/club_add.html"); });

	m_Server.RegisterResource("/add_match.html",   [](auto& Request) { return HttpServer::LoadFile("html/add_match.html"); });
	m_Server.RegisterResource("/edit_match.html",  [](auto& Request) { return HttpServer::LoadFile("html/edit_match.html"); });

	m_Server.RegisterResource("/account_add.html",  [](auto& Request) { return HttpServer::LoadFile("html/account_add.html"); });
	m_Server.RegisterResource("/account_edit.html", [](auto& Request) { return HttpServer::LoadFile("html/account_edit.html"); });
	m_Server.RegisterResource("/account_list.html", [](auto& Request) { return HttpServer::LoadFile("html/account_list.html"); });

	m_Server.RegisterResource("/class_list.html", [](auto& Request) { return HttpServer::LoadFile("html/class_list.html"); });
	m_Server.RegisterResource("/weightclass_add.html",  [](auto& Request) { return HttpServer::LoadFile("html/weightclass_add.html"); });
	m_Server.RegisterResource("/weightclass_edit.html", [](auto& Request) { return HttpServer::LoadFile("html/weightclass_edit.html"); });

	m_Server.RegisterResource("/matchtable_add.html", [](auto& Request) { return HttpServer::LoadFile("html/matchtable_add.html"); });

	m_Server.RegisterResource("/rule_add.html",  [](auto& Request) { return HttpServer::LoadFile("html/rule_add.html"); });
	m_Server.RegisterResource("/rule_list.html", [](auto& Request) { return HttpServer::LoadFile("html/rule_list.html"); });

	m_Server.RegisterResource("/tournament_list.html", [](auto& Request) { return HttpServer::LoadFile("html/tournament_list.html"); });
	m_Server.RegisterResource("/tournament_add.html",  [](auto& Request) { return HttpServer::LoadFile("html/tournament_add.html"); });

	m_Server.RegisterResource("/server_config.html", [](auto& Request) { return HttpServer::LoadFile("html/server_config.html"); });

	m_Server.RegisterResource("/dm4.html", [](auto& Request) { return HttpServer::LoadFile("html/dm4.html"); });


	//File uploads

	m_Server.RegisterResource("/upload/dm4", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		//ZED::Log::Debug(Request.m_Body);

		auto pos = Request.m_Body.Find("\r\n\r\n");
		if (pos != 0)
		{
			//std::string body_after_boundary = Request.m_Body.substr(pos + 4)
			auto boundary_end = Request.m_Body.FindLast("\r\n------WebKitFormBoundary");

			if (boundary_end == 0)
				return Error(Error::Type::InvalidFormat);

			//auto upload_content = ZED::Blob(Request.m_Body.substr(pos + 4, boundary_end - pos - 4));

			//ZED::Log::Debug(Request.m_Body.substr(pos + 4, boundary_end - pos - 4));

			//upload_content.Trim(content_length);
			DM4 dm4_file(Request.m_Body.Trim(pos + 4, boundary_end - pos - 4));

			if (!dm4_file)
				return Error(Error::Type::InvalidFormat);

			bool success;
			auto output = AddDM4File(dm4_file, true, &success);

			if (!success)
				return "Parsing FAILED<br/><br/>" + output;

			AddDM4File(dm4_file);

			return "Parsing OK<br/><br/>" + output;

			//TODO apply DM4 file
		}
		
		return Error(Error::Type::InvalidFormat);
	});

	m_Server.RegisterResource("/upload/md5", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		//ZED::Log::Debug(Request.m_Body);

		auto pos = Request.m_Body.Find("\r\n\r\n");
		if (pos != 0)
		{
			auto boundary_end = Request.m_Body.FindLast("\r\n------WebKitFormBoundary");

			if (boundary_end == 0)
				return Error(Error::Type::InvalidFormat);

			MD5 md5_file(Request.m_Body.Trim(pos + 4, boundary_end - pos - 4));

			if (!md5_file)
				return Error(Error::Type::InvalidFormat);

			//TODO apply MD5 file
			AddTournament(new Tournament(md5_file));
		}

		return Error(Error::Type::InvalidFormat);
	});

	//Ajax requests

	m_Server.RegisterResource("/ajax/get_nonce", [this](auto& Request) {
		auto& nonce = m_Database.CreateNonce(Request.m_RequestInfo.RemoteIP, Request.m_RequestInfo.RemotePort);
		return nonce.GetChallenge();
	});


	m_Server.RegisterResource("/ajax/login", [this](auto& Request) -> std::string {
		auto username  = HttpServer::DecodeURLEncoded(Request.m_Body, "username");
		auto challenge = HttpServer::DecodeURLEncoded(Request.m_Body, "challenge");
		auto response  = HttpServer::DecodeURLEncoded(Request.m_Body, "response");

		if (!m_Database.DoLogin(username, Request.m_RequestInfo.RemoteIP, response))
			return "Login failed!";

		Request.m_ResponseHeader = HttpServer::CookieHeader("session", response);

		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/config/get_mats", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_GetMats();
	});


	m_Server.RegisterResource("/ajax/config/open_mat", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_OpenMat(Request);
	});


	m_Server.RegisterResource("/ajax/config/close_mat", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_CloseMat(Request);
	});


	m_Server.RegisterResource("/ajax/config/set_mat", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_UpdateMat(Request);
	});


	m_Server.RegisterResource("/ajax/config/uptime", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		return Ajax_Uptime();
	});


	m_Server.RegisterResource("/ajax/colors/get", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		ZED::CSV ret;

		Color base_color;
		Color i;
		do
		{
			ret << i << i.ToString() << i.ToHexString();
			++i;
		} while (i != base_color);

		return ret;
	});


	m_Server.RegisterResource("/ajax/tournament/is_open", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (GetTournament() != &m_TempTournament)
			return Error();//OK
		return Error(Error::Type::ItemNotFound);
	});


	m_Server.RegisterResource("/ajax/get_schedule", [this](auto& Request) -> std::string {
		if (!GetTournament())
			return std::string("No tournament is open");
		return GetTournament()->Schedule2String();
	});


	m_Server.RegisterResource("/ajax/participants/get", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (!GetTournament())
			return std::string("No tournament is open");
		return GetTournament()->Participants2String();
	});


	m_Server.RegisterResource("/ajax/get_masterschedule", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (!GetTournament())
			return std::string("No tournament is open");
		return GetTournament()->MasterSchedule2String();
	});


	m_Server.RegisterResource("/ajax/masterschedule/move_up", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament is open");

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return std::string();

		GetTournament()->MoveScheduleEntryUp(id);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/masterschedule/move_down", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament is open");

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return std::string("Invalid id");

		GetTournament()->MoveScheduleEntryDown(id);
		return std::string();
	});


	m_Server.RegisterResource("/ajax/match/get", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (!GetTournament())
			return std::string();

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return Error(Error::Type::InvalidID);

		LockTillScopeEnd();
		auto match = GetTournament()->FindMatch(id);

		if (!match)
			return Error(Error::Type::ItemNotFound);

		return match->AllToString();
	});


	m_Server.RegisterResource("/ajax/masterschedule/set_mat", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament open");

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "index"));
		int mat   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "mat"));

		if (index < 0)
			return std::string("Invalid match id");
		if (mat <= 0)
			return std::string("Invalid mat id");

		if (GetTournament()->GetStatus() == Status::Concluded)
			return std::string("Tournament is already finalized");

		auto* entry = GetTournament()->GetScheduleEntry(index);

		if (!entry)
			return std::string("Could not find entry");

		//if (!GetTournament()->IsMatUsed(mat))
			//return std::string("Can't change to a mat that is not used in this tournament");

		entry->SetMatID(mat);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/match/set_rule", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament open");

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		auto rule = HttpServer::DecodeURLEncoded(Request.m_Body, "rule");

		if (id < 0)
			return std::string("Invalid match id");

		auto* match = GetTournament()->FindMatch(id);
		auto ruleSet = m_Database.FindRuleSetByName(rule);

		if (!match)
			return std::string("Could not find match");
		if (!ruleSet)
			return std::string("Could not find rule set");

		match->SetRuleSet(ruleSet);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/match/move_up", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament open");

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return std::string();

		GetTournament()->MoveMatchUp(id);

		return std::string();
	});

	m_Server.RegisterResource("/ajax/match/move_down", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament open");

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return std::string();

		GetTournament()->MoveMatchDown(id);

		return std::string();
	});

	m_Server.RegisterResource("/ajax/match/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament open");

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return std::string();

		bool success = GetTournament()->DeleteMatch(id);

		return std::string();
	});

	m_Server.RegisterResource("/ajax/match/get_log", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (!GetTournament())
			return std::string("No tournament open");

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return std::string();

		auto match = GetTournament()->FindMatch(id);

		if (!match)
			return std::string();

		return match->GetLog().ToString();
	});


	m_Server.RegisterResource("/ajax/mat/hajime", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return "Invalid id";

		LockTillScopeEnd();
		auto mat = FindMat(id);

		if (!mat)
			return "Could not find mat";

		mat->Hajime();
		return Error();//OK
	});

	//Generic pause. Will call Mate() or Sonomama() if it's more appropriate
	m_Server.RegisterResource("/ajax/mat/pause", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		LockTillScopeEnd();
		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		if (mat->IsOsaekomiRunning())
			mat->Sonomama();
		else
			mat->Mate();

		return Error();
	});

	m_Server.RegisterResource("/ajax/mat/mate", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		LockTillScopeEnd();
		auto mat = FindMat(id);

		if (!mat)
			return "Could not find mat";

		mat->Mate();
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/mat/sonomama", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		LockTillScopeEnd();
		auto mat = FindMat(id);

		if (!mat)
			return "Could not find mat";

		mat->Sonomama();
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/mat/start_match", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (!GetTournament())
			return "No Tournament is currently open";

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return "Invalid mat";

		auto mat = FindMat(id);

		if (!mat)
			return "Mat not found";

		auto nextMatch = GetTournament()->GetNextMatch(mat->GetMatID());
		if (nextMatch)
			if (!mat->StartMatch(nextMatch))
				return "Could not start next match";
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/mat/end_match", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto* mat = FindMat(id);

		if (!mat)
			return "Could not find mat";

		mat->EndMatch();

		if (GetTournament())
			GetTournament()->Save();

		return Error();//OK
	});

	//Serialization
	m_Server.RegisterResource("/ajax/mat/current_time", [this](auto& Request) -> std::string {
		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return std::string();

		auto mat = FindMat(id);

		if (!mat)
			return std::string();

		return std::to_string(mat->GetTimeElapsed()) + "," + (mat->IsHajime() ? "1" : "0");
	});

	m_Server.RegisterResource("/ajax/mat/get_score", [this](auto& Request) -> std::string {
		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto* mat = FindMat(id);
		 
		if (!mat)
			return std::string("Could not find mat");

		ZED::CSV ret;
		ret << mat->Scoreboard2String();
		ret << mat->GetTime2Display() << mat->IsHajime() << mat->Osaekomi2String(Fighter::White) << mat->Osaekomi2String(Fighter::Blue) << mat->CanNextMatchStart() << mat->HasConcluded() << (mat->IsOutOfTime() && mat->GetResult().m_Winner == Winner::Draw) << mat->IsGoldenScore() << mat->AreFightersOnMat();
		return ret;
	});

	m_Server.RegisterResource("/ajax/mat/get_osaekomilist", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return std::string();

		auto* mat = FindMat(id);

		if (!mat)
			return std::string();

		auto& list = mat->GetOsaekomiList();
		ZED::CSV ret;
		for (auto& entry : list)
			ret << entry.m_Who << entry.m_Time;

		return (std::string)ret;
	});

	m_Server.RegisterResource("/ajax/mat/screenshot", [this](auto& Request) -> ZED::Blob {
		auto account = IsLoggedIn(Request);
		if (!account)
			return (std::string)Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return (std::string)Error(Error::Type::InvalidID);

		auto mat = FindMat(id);

		if (!mat)
			return (std::string)Error(Error::Type::MatNotFound);

		//if (!mat->RequestScreenshot())
			//return std::string("Could not create screenshot");

		//const auto ret = HttpServer::LoadFile("screenshot.png");
		//return ret;
		ZED::Blob data = mat->RequestScreenshot();
		return data;
	}, HttpServer::ResourceType::Image_PNG);



	//Commands for fighters
	for (int i = 0; i < 2; i++)
	{
		Fighter fighter = (Fighter)i;

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+ippon", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->AddIppon(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-ippon", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveIppon(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+wazaari", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->AddWazaAri(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-wazaari", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveWazaAri(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/osaekomi", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->Osaekomi(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+shido", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return "Invalid id";

			auto mat = FindMat(id);

			if (mat)
				mat->AddShido(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-shido", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveShido(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+hansokumake", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return "Invalid id";

			auto mat = FindMat(id);

			if (mat)
			{
				mat->AddHansokuMake(fighter);
				mat->AddNotDisqualification(fighter);//TODO REMOVE THIS!!
			}
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-hansokumake", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveHansokuMake(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+medic", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return "Invalid id";

			auto mat = FindMat(id);

			if (mat)
				mat->AddMedicalExamination(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-medic", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveMedicalExamination(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/hantei", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return "Invalid id";

			auto mat = FindMat(id);

			if (mat)
			{
				mat->Hantei(fighter);
				mat->EndMatch();
			}

			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/+draw", [this](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
			{
				mat->SetAsDraw();
				mat->EndMatch();
			}

			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/+golden_score", [this](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->EnableGoldenScore();
			return Error();//OK
		});
	}


	m_Server.RegisterResource("/ajax/mat/tokeda", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return "Invalid id";

		auto mat = FindMat(id);

		if (mat)
			mat->Tokeda();
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/mat/names_on_mat", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return std::string();

		auto mat = FindMat(id);

		if (!mat)
			return std::string();

		ZED::CSV ret;
		if (!mat->AreFightersOnMat())
			ret << "- - -,- - -";
		else if (mat->GetMatch())
			ret << mat->GetMatch()->GetFighter(Fighter::White)->GetName() << mat->GetMatch()->GetFighter(Fighter::Blue)->GetName();
		else//Not supported by mat
			ret << "???,???";

		auto nextMatches = mat->GetNextMatches();
		for (auto match : nextMatches)
		{
			if (match && match->GetFighter(Fighter::White) && match->GetFighter(Fighter::Blue))
				ret << match->GetFighter(Fighter::White)->GetName() << match->GetFighter(Fighter::Blue)->GetName();
		}

		return ret;
	});



	m_Server.RegisterResource("/ajax/judoka/autocomplete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		bool filter = HttpServer::DecodeURLEncoded(Request.m_Query, "filter") == "true";

		if (filter && GetTournament())
			return GetTournament()->GetDatabase().JudokaToJSON();

		return m_Database.JudokaToJSON();
	});


	m_Server.RegisterResource("/ajax/judoka/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		auto firstname = HttpServer::DecodeURLEncoded(Request.m_Body, "firstname");
		auto lastname  = HttpServer::DecodeURLEncoded(Request.m_Body, "lastname");
		int  weight = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "weight"));
		Gender gender = (Gender)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));

		if (weight < 0)
			weight = 0;

		if (!firstname.size() || !lastname.size() || (gender != Gender::Male && gender != Gender::Female))
			return std::string("Invalid input");

		m_Database.AddJudoka(Judoka(firstname, lastname, weight, gender));
		m_Database.Save();
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/judoka/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return Error(Error::Type::InvalidID);

		auto judoka = m_Database.FindJudoka(id);

		if (judoka)
			return judoka->ToString();

		return std::string();
	});


	m_Server.RegisterResource("/ajax/judoka/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		ZED::CSV ret;
		for (auto [id, judoka] : m_Database.GetAllJudokas())
		{
			if (judoka)
				ret << judoka->ToString();
		}
		return (std::string)ret;
	});


	m_Server.RegisterResource("/ajax/judoka/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return Error(Error::Type::InvalidID);

		auto firstname = HttpServer::DecodeURLEncoded(Request.m_Body, "firstname");
		auto lastname  = HttpServer::DecodeURLEncoded(Request.m_Body, "lastname");
		int  weight = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "weight"));
		Gender gender = (Gender)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));

		if (weight < 0)
			weight = 0;

		if (!firstname.size() || !lastname.size() || (gender != Gender::Male && gender != Gender::Female))
			return std::string("Invalid input");

		auto judoka = m_Database.FindJudoka(id);

		if (!judoka)
			return std::string("Judoka not found");

		judoka->SetFirstname(firstname);
		judoka->SetLastname(lastname);
		judoka->SetWeight(weight);
		judoka->SetGender(gender);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/judoka/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return Error(Error::Type::InvalidID);

		auto judoka = m_Database.FindJudoka(id);

		if (!judoka)
			return std::string("Judoka not found");

		if (!m_Database.DeleteJudoka(judoka->GetID()))
			return std::string("Failed to delete");
		
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/club/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ListClubs();
	});


	m_Server.RegisterResource("/ajax/participant/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "id"));

		if (id < 0)
			return std::string("Invalid input");

		auto* judoka = m_Database.FindJudoka(id);

		if (!judoka)
			return std::string("Could not find judoka in database");

		if (GetTournament())
		{
			if (GetTournament()->IsParticipant(*judoka))
				return Error();//OK, Judoka is already a participant

			if (GetTournament()->AddParticipant(judoka))
			{
				GetTournament()->GenerateSchedule();//Recalculate schedule
				return Error();//OK
			}
		}

		return std::string("failed");
	});


	m_Server.RegisterResource("/ajax/participant/remove", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id < 0)
			return Error(Error::Type::InvalidID);

		if (GetTournament() && GetTournament()->RemoveParticipant(id))
			return Error();//OK

		return std::string("failed");
	});


	m_Server.RegisterResource("/ajax/account/add", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (account->GetAccessLevel() < Account::AccessLevel::Moderator)
			return Error(Error::Type::NotEnoughPermissions);

		auto username    = HttpServer::DecodeURLEncoded(Request.m_Body, "username");
		auto password    = HttpServer::DecodeURLEncoded(Request.m_Body, "password");
		int  permissions = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "permissions"));

		if (username.length() <= 0 || password.length() <= 0)
			return std::string("Invalid username or password");

		if (permissions <= 1)
			m_Database.AddAccount(Account(username, password));
		else if (permissions == 2 && account->GetAccessLevel() == Account::AccessLevel::Admin)
			m_Database.AddAccount(Account(username, password, Account::AccessLevel::Moderator));
		else if (permissions == 3 && account->GetAccessLevel() == Account::AccessLevel::Admin)
			m_Database.AddAccount(Account(username, password, Account::AccessLevel::Admin));

		m_Database.Save();
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/account/update", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		auto* account_to_change = m_Database.SetAccount(index);
		if (!account_to_change)
			return std::string("Account not found");

		if (account->GetAccessLevel() != Account::AccessLevel::Admin)//If we are not an admin
		{
			//If this not our account, we need to have more permissions than the account we want to change
			if (account_to_change->GetUsername() != account->GetUsername() && account_to_change->GetAccessLevel() >= account->GetAccessLevel())
				return Error(Error::Type::NotEnoughPermissions);
		}

		auto password = HttpServer::DecodeURLEncoded(Request.m_Body, "password");
		int  permissions = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "permissions"));

		if (password.length() <= 0)//Password not changed
			password = account_to_change->GetPassword();//Keep old one

		if (permissions <= 1)
			*account_to_change = Account(account_to_change->GetUsername(), password);
		else if (permissions == 2)
			*account_to_change = Account(account_to_change->GetUsername(), password, Account::AccessLevel::Moderator);
		else if (permissions == 3)
			*account_to_change = Account(account_to_change->GetUsername(), password, Account::AccessLevel::Admin);
		else
			return std::string("Unknown access level");

		m_Database.Save();
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/account/delete", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		auto* account_to_change = m_Database.SetAccount(index);
		if (!account_to_change)
			return std::string("Account not found");

		//If we are not an admin
		if (account->GetAccessLevel() != Account::AccessLevel::Admin)
			return Error(Error::Type::NotEnoughPermissions);

		if (!m_Database.DeleteAccount(account_to_change->GetUsername()))
			return std::string("Failed");

		m_Database.Save();
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/account/list", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (account->GetAccessLevel() < Account::AccessLevel::Moderator)
			return Error(Error::Type::NotEnoughPermissions);

		ZED::CSV ret;
		for (auto acc : m_Database.GetAccounts())
		{
			if (acc)
				ret << acc->ToString(account->GetAccessLevel());
		}
		return (std::string)ret;
	});


	m_Server.RegisterResource("/ajax/account/get", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (account->GetAccessLevel() < Account::AccessLevel::Moderator)
			return Error(Error::Type::NotEnoughPermissions);

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		auto* account_to_change = m_Database.GetAccount(index);
		if (!account_to_change)
			return std::string("Account not found");

		if (account_to_change->GetAccessLevel() >= account->GetAccessLevel() && account->GetAccessLevel() != Account::AccessLevel::Admin)
			return std::string("Not enough permissions to change an account of this status");

		return account_to_change->ToString(account->GetAccessLevel());
	});


	m_Server.RegisterResource("/ajax/nonces/list", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (account->GetAccessLevel() < Account::AccessLevel::Moderator)
			return Error(Error::Type::NotEnoughPermissions);

		ZED::CSV ret;
		for (const auto& [nonce, account] : m_Database.GetNonces())
		{
			if (!account)
				ret << nonce.GetChallenge() << nonce.GetIP2String() << "- - -" << (nonce.GetExpirationTimestamp() - Timer::GetTimestamp());
			else
				ret << nonce.GetChallenge() << nonce.GetIP2String() << account->GetUsername() << (nonce.GetExpirationTimestamp() - Timer::GetTimestamp());
		}

		return (std::string)ret;
	});


	m_Server.RegisterResource("/ajax/matchtable/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament open");

		std::string ret;
		for (auto table : GetTournament()->GetMatchTables())
		{
			if (!table)
				continue;

			ret += table->ToHTML();
			ret += "<br/><br/><br/>";
		}

		return ret;
	});


	m_Server.RegisterResource("/ajax/matchtable/get_form", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament open");
		
		int type = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "type"));

		switch ((MatchTable::Type)type)
		{
			case MatchTable::Type::Weightclass:
				return Weightclass::GetHTMLForm();
				break;

			default:
				return std::string("Unknown form");
		}
	});


	m_Server.RegisterResource("/ajax/matchtable/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament is open");

		MatchTable::Type type = (MatchTable::Type)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "type"));
		MatchTable* new_table = nullptr;

		switch (type)
		{
			case MatchTable::Type::Weightclass:
			{
				int minWeight = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "minWeight"));
				int maxWeight = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "maxWeight"));
				if (minWeight < 0 || maxWeight < 0)
					return std::string("Invalid value");

				int gender = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));

				if (gender < 0 || gender >= 2)
					new_table = new Weightclass(GetTournament(), minWeight, maxWeight);
				else
					new_table = new Weightclass(GetTournament(), minWeight, maxWeight, (Gender)gender);
				break;
			}

			default:
				return std::string("Unknown type");
		}
		
		int mat = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat"));
		auto ruleName = HttpServer::DecodeURLEncoded(Request.m_Body, "rule");//TODO Use ID instead

		new_table->SetMatID(FindDefaultMatID());
		if (mat >= 0)
			new_table->SetMatID(mat);
		if (m_Database.FindRuleSetByName(ruleName))
			new_table->SetRuleSet(m_Database.FindRuleSetByName(ruleName));


		GetTournament()->Lock();
		GetTournament()->AddMatchTable(new_table);
		GetTournament()->GenerateSchedule();
		GetTournament()->Unlock();
		return Error();//OK
	});



	m_Server.RegisterResource("/ajax/matchtable/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament is open");

		int id    = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		auto name = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
		int color = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "color"));
		int mat   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat"));
		int rule  = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "rule"));

		if (id < 0)
			return Error(Error::Type::InvalidID);

		auto& tables      = GetTournament()->SetMatchTables();
		auto  table_index = GetTournament()->FindMatchTableIndex(id);

		if (table_index < 0 || !tables[table_index])
			return std::string("Could not find class");


		GetTournament()->Lock();

		if (color >= 0)
			tables[table_index]->SetColor(color);

		tables[table_index]->SetName(name);

		if (mat >= 0)
			tables[table_index]->SetMatID(mat);
		if (m_Database.FindRuleSet(rule))
			tables[table_index]->SetRuleSet(m_Database.FindRuleSet(rule));

		GetTournament()->Unlock();


		switch (tables[table_index]->GetType())
		{
			case MatchTable::Type::Weightclass:
			{
				int minWeight = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "minWeight"));
				int maxWeight = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "maxWeight"));
				int gender    = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));

				if (minWeight < 0 || maxWeight < 0)
					return std::string("Invalid value");

				Weightclass* table = (Weightclass*)tables[table_index];

				GetTournament()->Lock();

				table->SetMinWeight(minWeight);
				table->SetMaxWeight(maxWeight);

				if (gender < 0 || gender >= 2)
					table->EnforceGender(false);
				else
					table->SetGender((Gender)gender);

				GetTournament()->Unlock();
				break;
			}

			default:
				return std::string("Class is not a weightclass");
		}

		GetTournament()->UpdateMatchTable(id);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/matchtable/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament is open");

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id < 0)
			return Error(Error::Type::InvalidID);


		GetTournament()->Lock();
		bool ret = GetTournament()->DeleteMatchTable(id);
		GetTournament()->Unlock();

		if (!ret)
			return std::string("Failed to delete match table");

		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/matchtable/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return std::string("No tournament is open");

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id < 0)
			return std::string("Invalid id");

		if (!GetTournament()->FindMatchTable(id))
			return std::string("Could not find class");

		return GetTournament()->FindMatchTable(id)->ToString();
	});



	m_Server.RegisterResource("/ajax/match/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		int whiteID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "white"));
		int blueID  = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "blue"));

		if (whiteID < 0 || blueID < 0)
			return Error(Error::Type::InvalidID);

		auto white = m_Database.FindJudoka(whiteID);
		auto blue  = m_Database.FindJudoka(blueID);

		if (!white || !blue)//Judokas exist?
			return std::string("Judoka not found in database");

		GetTournament()->AddMatch(Match(GetTournament(), white, blue, FindDefaultMatID()));
		
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/match/set_mat", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		int matchID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		int matID   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat"));

		if (matchID < 0 || matID < 0)
			return Error(Error::Type::InvalidID);

		auto match = GetTournament()->FindMatch(matchID);

		if (!match)
			return Error(Error::Type::ItemNotFound);

		match->SetMatID(matID);
		return Error();//OK
	});


	//Rule Sets

	m_Server.RegisterResource("/ajax/rule/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		std::string name     = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
		int match_time       = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "match_time"));
		int goldenscore_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "goldenscore_time"));
		int osaekomi_ippon   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_ippon"));
		int osaekomi_wazaari = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_wazaari"));
		bool yuko = HttpServer::DecodeURLEncoded(Request.m_Body, "yuko") == "true";
		bool koka = HttpServer::DecodeURLEncoded(Request.m_Body, "koka") == "true";
		bool draw = HttpServer::DecodeURLEncoded(Request.m_Body, "draw") == "true";
		int break_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "break_time"));

		RuleSet* new_rule_set = new RuleSet(name, match_time, goldenscore_time, osaekomi_ippon, osaekomi_wazaari, yuko, koka, draw, break_time);
		if (!m_Database.AddRuleSet(new_rule_set))
			return std::string("Could not add rule set to database");
		m_Database.Save();
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/rule/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		std::string name = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
		int match_time       = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "match_time"));
		int goldenscore_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "goldenscore_time"));
		int osaekomi_ippon   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_ippon"));
		int osaekomi_wazaari = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_wazaari"));
		bool yuko = HttpServer::DecodeURLEncoded(Request.m_Body, "yuko") == "true";
		bool koka = HttpServer::DecodeURLEncoded(Request.m_Body, "koka") == "true";
		bool draw = HttpServer::DecodeURLEncoded(Request.m_Body, "draw") == "true";
		int break_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "break_time"));

		auto* rule = m_Database.FindRuleSet(id);

		if (!rule)
			return std::string("Could not find rule set in database");

		*rule = RuleSet(name, match_time, goldenscore_time, osaekomi_ippon, osaekomi_wazaari, yuko, koka, draw, break_time);
		m_Database.Save();
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/rule/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		auto* rule = m_Database.FindRuleSet(id);
		if (!rule)
			return std::string("Could not find rule set in database");

		ZED::CSV ret;
		ret << rule->GetName() << rule->GetMatchTime() << rule->GetGoldenScoreTime();
		ret << rule->GetOsaeKomiTime(false) << rule->GetOsaeKomiTime(true);
		ret << rule->IsYukoEnabled() << rule->IsKokaEnabled() << rule->IsDrawAllowed() << rule->GetBreakTime();
		return (std::string)ret;
	});

	m_Server.RegisterResource("/ajax/rule/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		auto& rules = m_Database.GetRuleSets();
		ZED::CSV ret;

		if (GetTournament() && GetTournament()->GetDefaultRuleSet())
			ret << GetTournament()->GetDefaultRuleSet()->GetID();
		else if (!rules.empty())
			ret << rules[0]->GetID();
		else
			ret << 0;

		for (auto rule : rules)
		{
			if (rule)
				ret << rule->GetID() << rule->GetName() << rule->GetDescription();
		}

		return (std::string)ret;
	});


	//Tournaments

	m_Server.RegisterResource("/ajax/tournament/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		auto name = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "rules"));

		if (FindTournament(name))
			return std::string("There is already a tournament with that name");
		
		auto& rules = m_Database.GetRuleSets();
		if (index < 0 || (uint32_t)index >= rules.size())
			return std::string("Could not find rule set in database");

		auto rule = rules[index];
		if (!rule)
			return std::string("Internal error");

		Tournament* new_tournament = new Tournament(name);
		new_tournament->SetDefaultRuleSet(rule);

		if (!AddTournament(new_tournament))
			return std::string("Could not add tournament");

		new_tournament->Save();		
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;
		
		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (index < 0 || (uint32_t)index >= m_Tournaments.size())
			return std::string("Invalid tournament id");

		ZED::CSV ret;
		ret << m_Tournaments[index]->GetName() << m_Tournaments[index]->GetParticipants().size();
		ret << m_Tournaments[index]->GetSchedule().size() << m_Tournaments[index]->GetStatus();
		ret << m_Tournaments[index]->GetDefaultRuleSet()->GetID();
		return ret;
	});

	m_Server.RegisterResource("/ajax/tournament/open", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (index < 0 || (uint32_t)index >= m_Tournaments.size())
			return std::string("Invalid tournament id");

		if (!OpenTournament(index))
			return std::string("Could not open tournament");

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/close", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (GetTournament())
			GetTournament()->Save();

		if (!CloseTournament())
			return std::string("Could not close tournament");

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/empty", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (index < 0 || (uint32_t)index >= m_Tournaments.size())
			return std::string("Invalid tournament id");

		m_Tournaments[index]->DeleteAllMatchResults();
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		ZED::CSV ret;
		for (auto tournament : m_Tournaments)
		{
			if (tournament)
			{
				ret << tournament->GetName() << tournament->GetParticipants().size() << tournament->GetSchedule().size() << tournament->GetStatus();
				ret << (m_CurrentTournament && tournament->GetName() == m_CurrentTournament->GetName());
			}
		}

		return (std::string)ret;
	});


	//Slave

	m_Server.RegisterResource("/ajax/slave/connection_test", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		return Error();//OK
	});

	//Slave mat commands

	m_Server.RegisterResource("/ajax/slave/set_mat_id", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";
		//TODO verify request came from master

		int new_id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (!GetDefaultMat())
			return "Could not find mat";

		GetDefaultMat()->SetMatID(new_id);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/slave/is_mat_open", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		for (auto mat : m_Mats)
		{
			if (mat && mat->GetMatID() == id)
			{
				if (mat->IsOpen())
					return Error();//OK
				return "Mat not open";
			}
		}

		return "Could not find mat";
	});


	m_Server.RegisterResource("/ajax/slave/open_mat", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		for (auto mat : m_Mats)
		{
			if (mat && mat->GetMatID() == id)
			{
				if (mat->Open())
					return Error();//OK
				return "Could not open";
			}
		}

		return "Could not find mat";
	});


	m_Server.RegisterResource("/ajax/slave/close_mat", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		for (auto mat : m_Mats)
		{
			if (mat && mat->GetMatID() == id)
			{
				if (mat->Close())
					return Error();//OK
				return "Could not close mat";
			}
		}

		return "Could not find mat";
	});


	m_Server.RegisterResource("/ajax/slave/get_mat_status", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		for (auto mat : m_Mats)
			if (mat && mat->GetMatID() == id)
				return Error();//OK

		return "Could not find mat";
	});


	m_Server.RegisterResource("/ajax/slave/start_match", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		int matID         = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		ZED::CSV matchCSV(Request.m_Body);

		for (auto mat : m_Mats)
		{
			if (mat && mat->GetMatID() == matID)
			{
				Match* match = new Match(matchCSV, GetTournament());
				GetTournament()->AddMatch(match);

				if (mat->StartMatch(match))
					return "ok";//OK
				return "Could not start match";
			}
		}

		return "Mat not found";
	});

	m_Server.RegisterResource("/ajax/slave/end_match", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		//TODO check security token

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto mat = FindMat(id);

		if (!mat)
			return "Could not find mat";

		if (mat->EndMatch())
			return Error();

		return "Operation failed";
	});

	m_Server.RegisterResource("/ajax/config/shutdown", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		LockTillScopeEnd();

		if (GetTournament())
		{
			if (!CloseTournament())
				return std::string("Could not close currently opened tournament");
		}

		for (auto mat : SetMats())
		{
			if (mat &&	!mat->Close())
				return std::string("Could not close mat");
		}

		Shutdown();
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/config/testscreen", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		LockTillScopeEnd();

#ifdef _WIN32
		system("Judoboard.exe --testscreen");
#else
		system("./Judoboard --testscreen");
#endif

		return Error();//OK
		});


	m_Server.RegisterResource("/ajax/config/demo", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		LockTillScopeEnd();

#ifdef _WIN32
		system("Judoboard.exe --demo");
#else
		system("./Judoboard --demo");
#endif

		return Error();//OK
	});

	//Commands slave -> master

	m_Server.RegisterResource("/ajax/master/connection_test", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsMaster())
			return "You are not allowed to connect";

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/config/connect_to_master", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		auto host = HttpServer::DecodeURLEncoded(Request.m_Body, "host");

		ZED::Log::Info("Received command to connect to master" + host);

		if (!ConnectToMaster(host))
			return std::string("Could not connect");

		if (!SendCommandToMaster("/ajax/master/mat_available"))
			ZED::Log::Warn("Could not make mat available to master");

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/master/mat_available", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		auto ip  = ZED::Core::IP2String(Request.m_RequestInfo.RemoteIP);
		int port = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "port"));

		ZED::Log::Info("Slave informed us about a new mat");

		uint32_t id = 1;
		while (FindMat(id))//Find first empty id
			id++;

		ZED::Log::Info("Starting remote mat with id " + std::to_string(id) + " at address " + ip);

		RemoteMat* new_mat = new RemoteMat(id, ip, port);
		new_mat->Open();

		SetMats().emplace_back(new_mat);
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/master/find_participant", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		auto uuid = HttpServer::DecodeURLEncoded(Request.m_Query, "uuid");

		ZED::Log::Info("Slave requested participant info");

		auto judoka = GetTournament()->FindParticipant(UUID(std::move(uuid)));

		if (!judoka)
			return "Not found";

		ZED::CSV csv;
		*judoka >> csv;
		return csv;
	});

	m_Server.RegisterResource("/ajax/master/post_match_result", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		//TODO check security token

		ZED::Log::Info("Slave posted match results to us");

		ZED::CSV match_data = Request.m_Body;
		Match posted_match(match_data, GetTournament());

		auto match = GetTournament()->FindMatch(posted_match);

		if (!match)
			return "Not found";

		*match = posted_match;
		return "ok";
	});
}



Application::~Application()
{
	if (!m_Database.Save("database.csv"))
		ZED::Log::Error("Could not save database!");

	for (auto mat : m_Mats)
		delete mat;

	m_Mats.clear();

	m_Running = false;
	m_CurrentTournament = nullptr;

	for (auto tournament : m_Tournaments)
		delete tournament;
}



bool Application::LoadDataFromDisk()
{
	if (!m_Database.Load("database.csv"))
	{
		ZED::Log::Error("Could not load database!");
		return false;
	}

	ZED::Core::Indexer([this](auto Filename) {
		Filename = Filename.substr(Filename.find_last_of(ZED::Core::Separator) + 1);
		AddTournament(new Tournament(Filename));
		return true;
		}, "tournaments");

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

	for (auto new_judoka : File.GetParticipants())
	{
		auto old_judoka = GetDatabase().FindJudoka_DM4_ExactMatch(new_judoka);

		if (!old_judoka)//No exact match
		{
			old_judoka = GetDatabase().FindJudoka_DM4_SameName(new_judoka);

			if (old_judoka)//Found someone with the right name but incorrect club/birthyear
			{
				ret += "Updating information of judoka: " + old_judoka->GetName() + "<br/>";

				if (!ParseOnly)
				{
					if (new_judoka.Club)
						old_judoka->SetClub(GetDatabase().FindClubByName(new_judoka.Club->Name));
					if (new_judoka.Birthyear > 0)
						old_judoka->SetBirthyear(new_judoka.Birthyear);
					if (new_judoka.Weight > 0)
						old_judoka->SetWeight(new_judoka.Weight);
				}
			}

			else//We don't have a judoka with this name
			{
				//Add to database
				ret += "Adding judoka: " + new_judoka.Firstname + " " + new_judoka.Lastname + "<br/>";

				if (!ParseOnly)
				{
					old_judoka = new Judoka(new_judoka);
					GetDatabase().AddJudoka(old_judoka);
				}
			}
		}


		//Judoka is now added/updated

		if (!ParseOnly)
		{//Add to the current tournament
			GetTournament()->AddParticipant(old_judoka);
		}
	}

	return ret;
}



bool Application::OpenTournament(uint32_t Index)
{
	if (Index >= m_Tournaments.size())
		return false;

	if (!CloseTournament())
		return false;

	m_CurrentTournament = m_Tournaments[Index];
	return true;
}



bool Application::CloseTournament()
{
	if (m_CurrentTournament)
		if (!m_CurrentTournament->CanCloseTournament())
			return false;

	m_CurrentTournament = nullptr;
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
	auto* account = m_Database.IsLoggedIn(Request.m_RequestInfo.RemoteIP, value);

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
				delete *it;
				it = m_Mats.erase(it);
			}
			return true;
		}
	}

	return false;
}



bool Application::StartLocalMat(uint32_t ID)
{
	ZED::Log::Debug("Starting local mat");

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
			return false;
		}
	}

	LockTillScopeEnd();
	Mat* new_mat = new Mat(ID, this);
	m_Mats.emplace_back(new_mat);

	ZED::Log::Info("New local mat has been created with ID=" + std::to_string(ID));

	if (IsSlave())
		SendCommandToMaster("/ajax/master/mat_available?port=" + std::to_string(m_Server.GetPort()));

	return true;
}



std::vector<const Match*> Application::GetNextMatches(uint32_t MatID) const
{
	if (!GetTournament())
	{
		std::vector<const Match*> empty;
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

	if (FindTournament(NewTournament->GetName()))
	{
		ZED::Log::Warn("Tournament with this name already exists");
		return false;
	}

	NewTournament->ConnectToDatabase(m_Database);
	m_Tournaments.emplace_back(NewTournament);

	if (NewTournament->GetStatus() == Status::Scheduled)//Fresh new tournament
	{//Try to open right away
		if (!m_CurrentTournament || m_CurrentTournament->CanCloseTournament())
			m_CurrentTournament = m_Tournaments.back();
	}

	return true;
}



const Tournament* Application::FindTournament(const std::string& Name) const
{
	for (auto* tournament : m_Tournaments)
		if (tournament && tournament->GetName() == Name)
			return tournament;
	return nullptr;
}



int Application::FindTournamentIndex(const std::string& Name) const
{
	for (uint32_t i = 0;i < m_Tournaments.size();i++)
		if (m_Tournaments[i] && m_Tournaments[i]->GetName() == Name)
			return i;
	return -1;
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
				delete* it;
				it = m_Mats.erase(it);
			}
			else
				++it;
		}

		ZED::Core::Pause(10 * 1000);
		runtime += 10;

		if (runtime % (10 * 60) == 0)//Every 10 minutes
		{
			if (!m_Database.Save())
				ZED::Log::Warn("Auto saving of database failed!");
		}
	}

	ZED::Log::Info("Closing application");
}



ZED::CSV Application::Mats2String() const
{
	ZED::CSV ret;
	ret << (uint32_t)m_Mats.size();

	for (auto mat : m_Mats)
		if (mat)
			ret << mat->GetMatID();

	return ret;
}