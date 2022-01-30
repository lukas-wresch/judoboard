#include "app.h"
#include "database.h"
#include "weightclass.h"
#include "pause.h"
#include "customtable.h"
#include "remote_mat.h"
#include "tournament.h"
#include "../ZED/include/log.h"
#include "../ZED/include/csv.h"
#include "../ZED/include/http_client.h"



using namespace Judoboard;



void Application::SetupHttpServer()
{
	m_Server.RegisterResource("/", [this](auto& Request) {
		if (!IsLoggedIn(Request))
			return HttpServer::LoadFile("html/login.html");
		return HttpServer::LoadFile("html/index.html");
	});

	m_Server.RegisterResource("/style.css", [](auto& Request) { return HttpServer::LoadFile("html/style.css"); }, HttpServer::ResourceType::CSS);
	m_Server.RegisterResource("/common.js", [](auto& Request) { return HttpServer::LoadFile("html/common.js"); }, HttpServer::ResourceType::JavaScript);

	m_Server.RegisterResource("/jquery-latest.min.js", [](auto& Request) { return HttpServer::LoadFile("html/jquery-latest.min.js"); }, HttpServer::ResourceType::JavaScript);
	m_Server.RegisterResource("/jquery-ui.min.js", [](auto& Request) { return HttpServer::LoadFile("html/jquery-ui.min.js"); }, HttpServer::ResourceType::JavaScript);
	m_Server.RegisterResource("/sha512.min.js", [](auto& Request) { return HttpServer::LoadFile("html/sha512.min.js"); }, HttpServer::ResourceType::JavaScript);
	m_Server.RegisterResource("/jquery-ui.css", [](auto& Request) { return HttpServer::LoadFile("html/jquery-ui.css"); }, HttpServer::ResourceType::CSS);

	m_Server.RegisterResource("/slideout.min.js", [](auto& Request) { return HttpServer::LoadFile("html/slideout.min.js"); }, HttpServer::ResourceType::JavaScript);
	m_Server.RegisterResource("/menu.png", [](auto& Request) { return HttpServer::LoadFile("html/menu.png"); }, HttpServer::ResourceType::Image_PNG);


	std::string urls[] = { "schedule", "mat", "mat_configure", "mat_edit", "participant_add", "judoka_add", "judoka_list", "judoka_edit",
		"club_list", "club_add", "add_match", "edit_match", "account_add", "account_edit", "account_change_password", "account_list",
		"matchtable_list", "matchtable_add", "rule_add", "rule_list", "tournament_list", "tournament_add",
		"server_config"
	};

	for (auto& filename : urls)
	{
		m_Server.RegisterResource("/" + filename + ".html", [this, filename](auto& Request) {
			if (!IsLoggedIn(Request))
				return HttpServer::LoadFile("html/login.html");
			return HttpServer::LoadFile("html/" + filename + ".html");
		});
	}


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
			DM4 dm4_file(Request.m_Body.Trim(pos + 4, boundary_end - pos - 4 + 1));

			if (!dm4_file)
				return Error(Error::Type::InvalidFormat);

			bool success;
			auto output = AddDM4File(dm4_file, true, &success);

			if (!success)
				return "Parsing FAILED<br/><br/>" + output;

			AddDM4File(dm4_file);//apply DM4 file

			return "Parsing OK<br/><br/>" + output;
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

			MD5 md5_file(Request.m_Body.Trim(pos + 4, boundary_end - pos - 4 + 1));

			if (!md5_file)
				return Error(Error::Type::InvalidFormat);

			AddTournament(new Tournament(md5_file, &GetDatabase()));//apply MD5 file

			return "Parsing OK";
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
			return Error(Error::Type::OperationFailed);

		Request.m_ResponseHeader = HttpServer::CookieHeader("session", response);

		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/get_status", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return "0";

		return std::to_string((int)account->GetAccessLevel());
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


	m_Server.RegisterResource("/ajax/config/fullscreen", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;
		return Ajax_SetFullscreen(true, Request);
	});


	m_Server.RegisterResource("/ajax/config/windowed", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;
		return Ajax_SetFullscreen(false, Request);
	});


	m_Server.RegisterResource("/ajax/config/uptime", [this](auto& Request) -> std::string {
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		return Ajax_Uptime();
	});


	m_Server.RegisterResource("/ajax/colors/get", [this](auto& Request) -> std::string {
		if (!IsLoggedIn(Request))
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
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		if (GetTournament() != &m_TempTournament)
			return Error();//OK
		return Error(Error::Type::ItemNotFound);
	});


	m_Server.RegisterResource("/ajax/get_schedule", [this](auto& Request) -> std::string {
		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);
		return GetTournament()->Schedule2String();
	});


	m_Server.RegisterResource("/ajax/participants/get", [this](auto& Request) -> std::string {
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);
		return GetTournament()->Participants2String();
	});


	m_Server.RegisterResource("/ajax/hansokumake/get", [this](auto& Request) -> std::string {
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);
		return Ajax_GetHansokumake();
	});


	m_Server.RegisterResource("/ajax/get_masterschedule", [this](auto& Request) -> std::string {
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);
		return GetTournament()->MasterSchedule2String();
	});


	m_Server.RegisterResource("/ajax/masterschedule/move_up", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return Error(Error::Type::InvalidID);

		GetTournament()->MoveScheduleEntryUp(id);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/masterschedule/move_down", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return Error(Error::Type::InvalidID);

		GetTournament()->MoveScheduleEntryDown(id);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/match/get", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

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
			return Error(Error::Type::InvalidID);
		if (mat <= 0)
			return std::string("Invalid mat id");

		if (GetTournament()->GetStatus() == Status::Concluded)
			return std::string("Tournament is already finalized");

		auto entry = GetTournament()->GetScheduleEntry(index);

		if (!entry)
			return Error(Error::Type::ItemNotFound);

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
			return Error(Error::Type::InvalidID);

		auto match = GetTournament()->FindMatch(id);
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
			return Error(Error::Type::InvalidID);

		LockTillScopeEnd();
		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

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
			return Error(Error::Type::MatNotFound);

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
			return Error(Error::Type::MatNotFound);

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
			return Error(Error::Type::InvalidID);

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

		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		if (!mat->EndMatch())
			return Error(Error::Type::OperationFailed);

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

		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		ZED::CSV ret;
		ret << mat->Scoreboard2String();
		ret << mat->GetTime2Display() << mat->IsHajime() << mat->Osaekomi2String(Fighter::White) << mat->Osaekomi2String(Fighter::Blue);
		ret << mat->CanNextMatchStart() << mat->HasConcluded() << mat->IsOutOfTime() << (mat->GetResult().m_Winner == Winner::Draw) << mat->IsGoldenScore() << mat->AreFightersOnMat();
		//Hansokumake with decision needed?
		ret << mat->GetScoreboard(Fighter::White).IsUnknownDisqualification();
		ret << mat->GetScoreboard(Fighter::Blue).IsUnknownDisqualification();
		return ret;
	});

	m_Server.RegisterResource("/ajax/mat/get_osaekomilist", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		auto& list = mat->GetOsaekomiList();
		ZED::CSV ret;
		for (auto& entry : list)
			ret << entry.m_Who << entry.m_Time;

		return ret;
	});

	m_Server.RegisterResource("/ajax/mat/screenshot", [this](auto& Request) -> ZED::Blob {
		if (!IsLoggedIn(Request))
			return (std::string)Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return (std::string)Error(Error::Type::InvalidID);

		auto mat = FindMat(id);

		if (!mat)
			return (std::string)Error(Error::Type::MatNotFound);

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

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+yuko", [this, fighter](auto& Request) -> std::string {
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->AddYuko(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-yuko", [this, fighter](auto& Request) -> std::string {
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveYuko(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+koka", [this, fighter](auto& Request) -> std::string {
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->AddKoka(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-koka", [this, fighter](auto& Request) -> std::string {
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveKoka(fighter);
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
				mat->AddHansokuMake(fighter);
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

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+disqualification", [this, fighter](auto& Request) -> std::string {
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);
			return Ajax_AddDisqualification(fighter, Request);
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-disqualification", [this, fighter](auto& Request) -> std::string {
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);
			return Ajax_RemoveDisqualification(fighter, Request);
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+nodisqualification", [this, fighter](auto& Request) -> std::string {
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);
			return Ajax_NoDisqualification(fighter, Request);
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-nodisqualification", [this, fighter](auto& Request) -> std::string {
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);
			return Ajax_RemoveNoDisqualification(fighter, Request);
		});


		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+medic", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

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

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+gachi", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->AddGachi(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-gachi", [this, fighter](auto& Request) -> std::string {
			auto account = IsLoggedIn(Request);
			if (!account)
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveGachi(fighter);
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

			if (!mat)
				return Error(Error::Type::MatNotFound);

			if (!mat->EnableGoldenScore())
				return Error(Error::Type::OperationFailed);

			return Error();//OK
		});
	}


	m_Server.RegisterResource("/ajax/mat/tokeda", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

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
			return Error(Error::Type::InvalidID);

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
		auto lastname = HttpServer::DecodeURLEncoded(Request.m_Body, "lastname");
		int  weight = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "weight"));
		Gender gender = (Gender)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));
		int  clubID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "club"));

		if (weight < 0)
			weight = 0;

		if (!firstname.size() || !lastname.size() || (gender != Gender::Male && gender != Gender::Female))
			return (std::string)(Error)Error::Type::InvalidInput;

		Judoka new_judoka(firstname, lastname, weight, gender);
		if (clubID >= 0)
			new_judoka.SetClub(GetDatabase().FindClub(clubID));

		m_Database.AddJudoka(std::move(new_judoka));
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
		return ret;
	});


	m_Server.RegisterResource("/ajax/judoka/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		if (id < 0)
			return Error(Error::Type::InvalidID);

		auto firstname = HttpServer::DecodeURLEncoded(Request.m_Body, "firstname");
		auto lastname = HttpServer::DecodeURLEncoded(Request.m_Body, "lastname");
		int  weight = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "weight"));
		Gender gender = (Gender)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));
		int  clubID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "club"));

		if (weight < 0)
			weight = 0;

		if (!firstname.size() || !lastname.size() || (gender != Gender::Male && gender != Gender::Female))
			return (std::string)(Error)Error::Type::InvalidInput;

		auto judoka = m_Database.FindJudoka(id);

		if (!judoka)
			return std::string("Judoka not found");

		judoka->SetFirstname(firstname);
		judoka->SetLastname(lastname);
		judoka->SetWeight(weight);
		judoka->SetGender(gender);
		if (clubID >= 0)
			judoka->SetClub(GetDatabase().FindClub(clubID));
		else
			judoka->SetClub(nullptr);

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


	//Clubs
	m_Server.RegisterResource("/ajax/club/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_AddClub(Request);
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

		auto judoka = m_Database.FindJudoka(id);

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

		return Error(Error::Type::OperationFailed);
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

		return Error(Error::Type::OperationFailed);
	});


	m_Server.RegisterResource("/ajax/account/add", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (account->GetAccessLevel() < Account::AccessLevel::Moderator)
			return Error(Error::Type::NotEnoughPermissions);

		auto username = HttpServer::DecodeURLEncoded(Request.m_Body, "username");
		auto password = HttpServer::DecodeURLEncoded(Request.m_Body, "password");
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

		auto account_to_change = m_Database.SetAccount(index);
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


	m_Server.RegisterResource("/ajax/account/update_password", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return std::string(Error(Error::Type::NotLoggedIn));

		return Ajax_UpdatePassword((Account*)account, Request);
	});


	m_Server.RegisterResource("/ajax/account/delete", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		auto account_to_change = m_Database.SetAccount(index);
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
		return ret;
	});


	m_Server.RegisterResource("/ajax/account/get", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		if (account->GetAccessLevel() < Account::AccessLevel::Moderator)
			return Error(Error::Type::NotEnoughPermissions);

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		auto account_to_change = m_Database.GetAccount(index);
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

		return ret;
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
			case MatchTable::Type::Pause:
				return Pause::GetHTMLForm();
			case MatchTable::Type::Custom:
				return CustomTable::GetHTMLForm();

			default:
				return std::string("Unknown form");
		}
	});


	m_Server.RegisterResource("/ajax/matchtable/get_participants", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;
		return Ajax_GetParticipantsFromMatchTable(Request);
	});


	m_Server.RegisterResource("/ajax/matchtable/get_matches", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;
		return Ajax_GetMatchesFromMatchTable(Request);
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

		case MatchTable::Type::Pause:
			return std::string("NOT IMPLEMENTED");
			break;

		case MatchTable::Type::Custom:
			new_table = new CustomTable(GetTournament());
			break;

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

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		auto name = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
		int color = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "color"));
		int mat = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat"));
		int rule = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "rule"));

		if (id < 0)
			return Error(Error::Type::InvalidID);

		auto& tables = GetTournament()->SetMatchTables();
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
			int gender = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));

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
		int blueID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "blue"));

		if (whiteID < 0 || blueID < 0)
			return Error(Error::Type::InvalidID);

		auto white = m_Database.FindJudoka(whiteID);
		auto blue = m_Database.FindJudoka(blueID);

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
		int matID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat"));

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

		std::string name = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
		int match_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "match_time"));
		int goldenscore_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "goldenscore_time"));
		int osaekomi_ippon = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_ippon"));
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
		int match_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "match_time"));
		int goldenscore_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "goldenscore_time"));
		int osaekomi_ippon = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_ippon"));
		int osaekomi_wazaari = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_wazaari"));
		bool yuko = HttpServer::DecodeURLEncoded(Request.m_Body, "yuko") == "true";
		bool koka = HttpServer::DecodeURLEncoded(Request.m_Body, "koka") == "true";
		bool draw = HttpServer::DecodeURLEncoded(Request.m_Body, "draw") == "true";
		int break_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "break_time"));

		auto rule = m_Database.FindRuleSet(id);

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

		auto rule = m_Database.FindRuleSet(id);
		if (!rule)
			return std::string("Could not find rule set in database");

		ZED::CSV ret;
		ret << rule->GetName() << rule->GetMatchTime() << rule->GetGoldenScoreTime();
		ret << rule->GetOsaeKomiTime(false) << rule->GetOsaeKomiTime(true);
		ret << rule->IsYukoEnabled() << rule->IsKokaEnabled() << rule->IsDrawAllowed() << rule->GetBreakTime();
		return ret;
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
		if (m_Tournaments[index]->GetDefaultRuleSet())
			ret << m_Tournaments[index]->GetDefaultRuleSet()->GetID();
		else
			ret << -1;
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
			return Error(Error::Type::InvalidID);

		m_Tournaments[index]->DeleteAllMatchResults();
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (index < 0 || (uint32_t)index >= m_Tournaments.size())
			return Error(Error::Type::InvalidID);

		if (GetTournament()->GetID() == m_Tournaments[index]->GetID())
			return Error(Error::Type::OperationFailed);

		ZED::Core::RemoveFile("tournaments/" + m_Tournaments[index]->GetName());

		delete m_Tournaments[index];
		m_Tournaments.erase(m_Tournaments.begin() + index);

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		ZED::CSV ret;
		for (auto tournament : m_Tournaments)
		{
			if (tournament)//Filter out temporary tournament
			{
				ret << tournament->GetName() << tournament->GetParticipants().size() << tournament->GetSchedule().size() << tournament->GetStatus();
				ret << (m_CurrentTournament && tournament->GetName() == m_CurrentTournament->GetName());
			}
		}

		return ret;
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
				return Error(Error::Type::OperationFailed);
			}
		}

		return Error(Error::Type::MatNotFound);
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
				return Error(Error::Type::OperationFailed);
			}
		}

		return Error(Error::Type::MatNotFound);
	});


	m_Server.RegisterResource("/ajax/slave/get_mat_status", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		for (auto mat : m_Mats)
			if (mat && mat->GetMatID() == id)
				return Error();//OK

		return Error(Error::Type::MatNotFound);
	});


	m_Server.RegisterResource("/ajax/slave/start_match", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		int matID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
		ZED::CSV matchCSV(Request.m_Body);

		for (auto mat : m_Mats)
		{
			if (mat && mat->GetMatID() == matID)
			{
				Match* match = new Match(matchCSV, GetTournament());
				GetTournament()->AddMatch(match);

				if (mat->StartMatch(match))
					return Error();//OK
				return Error(Error::Type::OperationFailed);
			}
		}

		return Error(Error::Type::MatNotFound);
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
			return Error(Error::Type::MatNotFound);

		if (mat->EndMatch())
			return Error();

		return Error(Error::Type::OperationFailed);
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
			if (mat && !mat->Close())
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

	m_Server.RegisterResource("/ajax/config/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		LockTillScopeEnd();

#ifdef _WIN32
		//system("Judoboard.exe --demo");
		//TODO: not supported
#else
		if (Application::NoWindow)
			system("git pull; cd ..; ./compile.sh; cd bin; ./judoboard --nowindow");
		else
			system("git pull; cd ..; ./compile.sh; cd bin; ./judoboard");
		Shutdown();
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

		auto ip = ZED::Core::IP2String(Request.m_RequestInfo.RemoteIP);
		int port = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "port"));

		ZED::Log::Info("Slave informed us about a new mat");

		uint32_t id = 1;
		while (FindMat(id))//Find first empty id
			id++;

		ZED::Log::Info("Starting remote mat with id " + std::to_string(id) + " at address " + ip);

		RemoteMat* new_mat = new RemoteMat(id, ip, port);
		new_mat->Open();

		LockTillScopeEnd();
		SetMats().emplace_back(new_mat);

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/master/find_judoka", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		auto uuid = HttpServer::DecodeURLEncoded(Request.m_Query, "uuid");

		//ZED::Log::Debug("Slave requested participant info");

		auto judoka = GetDatabase().FindJudoka(UUID(std::move(uuid)));

		if (!judoka)
			return "Not found";

		ZED::CSV csv;
		*judoka >> csv;
		csv << judoka->GetTimestampOfLastMatch();
		return csv;
	});

	m_Server.RegisterResource("/ajax/master/find_ruleset", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		auto uuid = HttpServer::DecodeURLEncoded(Request.m_Query, "uuid");

		ZED::Log::Info("Slave requested rule set info");

		auto rule_set = GetDatabase().FindRuleSet(UUID(std::move(uuid)));

		if (!rule_set)
			return "Not found";

		ZED::CSV csv;
		*rule_set >> csv;
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
		{
			ZED::Log::Error("Could not store match result that was sent by slave");
			return "Not found";
		}

		*match = posted_match;
		return "ok";
	});

	m_Server.RegisterResource("/ajax/master/get_next_matches", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		//TODO check security token

		int matID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		auto next_matches = GetNextMatches(matID);

		ZED::CSV match_data;
		match_data << next_matches.size();//TODO add number of matches to send

		for (auto match : next_matches)
			*match >> match_data;

		return match_data;
	});
}



Error Application::Ajax_UpdatePassword(Account* Account, const HttpServer::Request& Request)
{
	if (!Account)
		return Error::Type::InternalError;

	auto password = HttpServer::DecodeURLEncoded(Request.m_Body, "password");

	if (password.length() <= 0)//Password not changed
		return Error::Type::InvalidInput;

	Account->SetPassword(password);
	m_Database.Save();
	return Error::Type::NoError;//OK
}



ZED::CSV Application::Ajax_GetMats() const
{
	ZED::CSV ret;

	if (GetTournament())
	{//Show all mats that are available/used and an additional one
		auto max = std::max(GetHighestMatID(), GetTournament()->GetHighestMatIDUsed()) + 1;
		ret << max;

		for (uint32_t id = 1; id <= max; id++)
		{
			auto mat = FindMat(id);

			if (!mat)
			{
				std::string mat_name = Localizer::Translate("Mat") + " " + std::to_string(id);
				ret << id << IMat::Type::Unknown << false << mat_name << 0 << 0 << false;
			}
			else
				ret << mat->GetMatID() << mat->GetType() << mat->IsOpen() << mat->GetName() << mat->GetIpponStyle() << mat->GetTimerStyle() << mat->IsFullscreen();
		}
	}

	ret.AddNewline();
	return ret;
}



Error Application::Ajax_OpenMat(const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	if (id <= 0)
		return Error::Type::InvalidID;

	/*if (id == 0)//Create virtual mat
	{
		id = 1;
		while (FindMat(id))//Find first empty id
			id++;

		VirtualMat* new_mat = new VirtualMat(id);
		new_mat->Open();

		SetMats().emplace_back(new_mat);
		return Error();//OK
	}

	else
	{*/
	for (auto mat : SetMats())
	{
		if (mat && mat->GetMatID() == id)
		{
			mat->Open();
			return Error::Type::NoError;//OK
		}
	}
	//}

	return Error::Type::OperationFailed;
}



Error Application::Ajax_CloseMat(const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	if (id <= 0)
		return Error::Type::InvalidID;

	LockTillScopeEnd();
	if (CloseMat(id))
		return Error::Type::NoError;//OK

	return Error::Type::OperationFailed;
}



Error Application::Ajax_UpdateMat(const HttpServer::Request& Request)
{
	int id     = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	int new_id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body,  "id"));
	auto name  = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	int ipponStyle = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "ipponStyle"));
	int timerStyle = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "timerStyle"));

	if (id <= 0 || new_id <= 0)
		return Error::Type::InvalidID;
	if (ipponStyle <= -1)
		return Error::Type::InvalidID;
	if (timerStyle <= -1)
		return Error::Type::InvalidID;

	if (id != new_id)//Check if new_id is an unused id
	{
		for (const auto mat : GetMats())
		{
			if (mat && mat->GetMatID() == new_id)
				return Error::Type::InvalidID;
		}
	}

	LockTillScopeEnd();

	for (auto mat : SetMats())
	{
		if (mat && mat->GetMatID() == id)
		{
			if (id != new_id)
				mat->SetMatID(new_id);

			mat->SetName(name);
			mat->SetIpponStyle((Mat::IpponStyle)ipponStyle);
			mat->SetTimerStyle((Mat::TimerStyle)timerStyle);

			return Error();//OK
		}
	}

	return Error::Type::MatNotFound;
}



Error Application::Ajax_AddClub(const HttpServer::Request& Request)
{
	auto name = HttpServer::DecodeURLEncoded(Request.m_Body, "name");

	if (name.size() == 0)
		return Error::Type::InvalidInput;

	m_Database.AddClub(new Club(name));
	m_Database.Save();
	return Error();//OK
}



ZED::CSV Application::Ajax_ListClubs()
{
	ZED::CSV ret;
	for (auto club : m_Database.GetAllClubs())
	{
		if (club)
			ret << club->GetID() << club->GetName();
	}
	ret.AddNewline();
	return ret;
}



std::string Application::Ajax_GetParticipantsFromMatchTable(const HttpServer::Request& Request)
{
	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (id < 0)
		return Error(Error::Type::InvalidID);

	auto table = GetTournament()->FindMatchTable(id);

	if (!table)
		return Error(Error::Type::ItemNotFound);


	GetTournament()->Lock();

	ZED::CSV ret;
	for (auto judoka : table->GetParticipants())
		ret << judoka->GetID() << judoka->GetName();

	GetTournament()->Unlock();
	
	ret.AddNewline();
	return ret;
}



std::string Application::Ajax_GetMatchesFromMatchTable(const HttpServer::Request& Request)
{
	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (id < 0)
		return Error(Error::Type::InvalidID);

	auto table = GetTournament()->FindMatchTable(id);

	if (!table)
		return Error(Error::Type::ItemNotFound);


	GetTournament()->Lock();

	ZED::CSV ret;
	for (auto match : table->GetSchedule())
		ret << match->ToString();

	GetTournament()->Unlock();

	ret.AddNewline();
	return ret;
}



ZED::CSV Application::Ajax_Uptime()
{
	ZED::CSV ret;

	ret << (Timer::GetTimestamp() - m_StartupTimestamp);

	ret.AddNewline();
	return ret;
}



Error Application::Ajax_AddDisqualification(Fighter Whom, const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (id <= 0)
		return Error::Type::InvalidID;

	auto mat = FindMat(id);

	if (!mat)
		return Error::Type::MatNotFound;

	mat->AddDisqualification(Whom);
	return Error();//OK
}



Error Application::Ajax_RemoveDisqualification(Fighter Whom, const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (id <= 0)
		return Error::Type::InvalidID;

	auto mat = FindMat(id);

	if (!mat)
		return Error::Type::MatNotFound;

	mat->RemoveDisqualification(Whom);
	return Error();//OK
}



Error Application::Ajax_NoDisqualification(Fighter Whom, const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (id <= 0)
		return Error::Type::InvalidID;

	auto mat = FindMat(id);

	if (!mat)
		return Error::Type::MatNotFound;

	mat->AddNoDisqualification(Whom);
	return Error();//OK
}



Error Application::Ajax_RemoveNoDisqualification(Fighter Whom, const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (id <= 0)
		return Error::Type::InvalidID;

	auto mat = FindMat(id);

	if (!mat)
		return Error::Type::MatNotFound;

	mat->RemoveNoDisqualification(Whom);
	return Error();//OK
}



std::string Application::Ajax_GetHansokumake() const
{
	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	ZED::CSV ret;

	LockTillScopeEnd();
	for (auto mat : GetMats())
	{
		if (!mat) continue;
		if (!mat->GetMatch()) continue;

		for (Fighter fighter = Fighter::White; fighter <= Fighter::Blue; ++fighter)
		{
			if (mat->GetScoreboard(fighter).m_HansokuMake && mat->GetScoreboard(fighter).m_HansokuMake_Direct)
			{
				ret << mat->GetMatch()->ToString();
				ret << fighter;
				ret << mat->GetScoreboard(fighter).m_Disqualification;//Disqualification state
			}
		}
	}

	return ret;
}



Error Application::Ajax_SetFullscreen(bool Fullscreen, const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (id <= 0)
		return Error::Type::InvalidID;

	auto mat = FindMat(id);

	if (!mat)
		return Error::Type::MatNotFound;

	mat->SetFullscreen(Fullscreen);
	return Error();//OK
}