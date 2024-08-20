#include <stdio.h>
#include "app.h"
#include "database.h"
#include "weightclass.h"
#include "customtable.h"
#include "single_elimination.h"
#include "double_elimination.h"
#include "pool.h"
#include "remote_mat.h"
#include "tournament.h"
#include "../ZED/include/log.h"
#include "../ZED/include/http_client.h"



using namespace Judoboard;



void Application::SetupHttpServer()
{
	m_Server.RegisterResource("/", [this](auto& Request) {
		if (!IsLoggedIn(Request))
			return HttpServer::LoadFile("html/login.html");
		return HttpServer::LoadFile("html/index.html");
	});

	m_Server.RegisterResource("/style.css", [](auto& Request) { return HttpServer::LoadFile("html/style.css"); }, HttpServer::ResourceType::CSS, 24*60*60);
	m_Server.RegisterResource("/common.js", [](auto& Request) { return HttpServer::LoadFile("html/common.js"); }, HttpServer::ResourceType::JavaScript, 24*60*60);

	m_Server.RegisterResource("/jquery-latest.min.js", [](auto& Request) { return HttpServer::LoadFile("html/jquery-latest.min.js"); }, HttpServer::ResourceType::JavaScript, 24*60*60);
	m_Server.RegisterResource("/jquery-ui.min.js", [](auto& Request) { return HttpServer::LoadFile("html/jquery-ui.min.js"); }, HttpServer::ResourceType::JavaScript, 24*60*60);
	m_Server.RegisterResource("/sha512.min.js", [](auto& Request) { return HttpServer::LoadFile("html/sha512.min.js"); }, HttpServer::ResourceType::JavaScript, 24*60*60);
	m_Server.RegisterResource("/jquery-ui.css", [](auto& Request) { return HttpServer::LoadFile("html/jquery-ui.css"); }, HttpServer::ResourceType::CSS, 24*60*60);

	m_Server.RegisterResource("/yaml.min.js", [](auto& Request) { return HttpServer::LoadFile("html/yaml.min.js"); }, HttpServer::ResourceType::JavaScript, 24*60*60);

	m_Server.RegisterResource("/slideout.min.js", [](auto& Request) { return HttpServer::LoadFile("html/slideout.min.js"); }, HttpServer::ResourceType::JavaScript, 24*60*60);

	m_Server.RegisterResource("/menu.png",   [](auto& Request) { return HttpServer::LoadFile("html/menu.png");   }, HttpServer::ResourceType::Image_PNG, 24*60*60);
	m_Server.RegisterResource("/winner.png", [](auto& Request) { return HttpServer::LoadFile("html/winner.png"); }, HttpServer::ResourceType::Image_PNG, 24*60*60);
	m_Server.RegisterResource("/en.png", [](auto& Request) { return HttpServer::LoadFile("html/en.png"); }, HttpServer::ResourceType::Image_PNG, 24*60*60);
	m_Server.RegisterResource("/de.png", [](auto& Request) { return HttpServer::LoadFile("html/de.png"); }, HttpServer::ResourceType::Image_PNG, 24*60*60);


	std::string urls[] = { "schedule", "mat", "mat_configure", "mat_edit", "participant_add", "participant_weight", "judoka_add", "judoka_list", "judoka_edit", "lots",
		"club_list", "club_add", "association_list", "association_add", "add_match", "edit_match", "account_add", "account_edit", "account_change_password", "account_list",
		"matchtable_list", "matchtable_add", "matchtable_creator", "rule_add", "rule_list", "age_groups_add", "age_groups_list", "age_groups_select", "tournament_list", "tournament_add", "tournament_settings",
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
			auto boundary_end = Request.m_Body.FindLast("\r\n------");

			if (boundary_end == 0)
				return Error(Error::Type::InvalidFormat);

			DM4 dm4_file(Request.m_Body.Trim(pos + 5, boundary_end - pos - 5 + 1));

			if (!dm4_file)
				return Error(Error::Type::InvalidFormat);

			bool success;
			auto output = AddDM4File(dm4_file, true, &success);

			if (!success)
				return "Parsing FAILED<br/><br/>" + output;

			AddDM4File(dm4_file);//apply DM4 file

			output += R"(
<html>
	<head>
		<meta http-equiv = "refresh" content = "5; url=/#schedule.html"/>
	</head>
</html>
)";

			return "Parsing OK<br/><br/>" + output;
		}

		return Error(Error::Type::InvalidFormat);
	});

	m_Server.RegisterResource("/upload/dmf", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		//ZED::Log::Debug(Request.m_Body);

		auto pos = Request.m_Body.Find("\r\n\r\n");
		if (pos != 0)
		{
			auto boundary_end = Request.m_Body.FindLast("\r\n------");

			if (boundary_end == 0)
				return Error(Error::Type::InvalidFormat);

			DMF dmf_file(Request.m_Body.Trim(pos + 5, boundary_end - pos - 5 + 1));

			if (!dmf_file)
				return Error(Error::Type::InvalidFormat);

			bool success;
			auto output = AddDMFFile(dmf_file, true, &success);

			if (!success)
				return "Parsing FAILED<br/><br/>" + output;

			AddDMFFile(dmf_file);//apply DMF file

			output += R"(
<html>
	<head>
		<meta http-equiv = "refresh" content = "5; url=/#schedule.html"/>
	</head>
</html>
)";

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
			auto boundary_end = Request.m_Body.FindLast("\r\n------");

			if (boundary_end == 0)
				return Error(Error::Type::InvalidFormat);

			MD5 md5_file(Request.m_Body.Trim(pos + 5, boundary_end - pos - 5 + 1));

			if (!md5_file)
				return Error(Error::Type::InvalidFormat);

			AddTournament(new Tournament(md5_file, &GetDatabase()));//apply MD5 file

			std::string output = R"(
<html>
	<head>
		<meta http-equiv = "refresh" content = "5; url=/#tournament_list.html"/>
	</head>
</html>
)";

			return "Parsing OK<br/><br/>" + output;
		}

		return Error(Error::Type::InvalidFormat);
	});

	m_Server.RegisterResource("/upload/yml", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		//ZED::Log::Debug(Request.m_Body);

		auto pos = Request.m_Body.Find("\r\n\r\n");
		if (pos != 0)
		{
			auto boundary_end = Request.m_Body.FindLast("\r\n------");

			if (boundary_end == 0)
				return Error(Error::Type::InvalidFormat);

			Tournament* tournament_file = new Tournament("");

			auto yaml = YAML::Load((char*)Request.m_Body.Trim(pos + 5, boundary_end - pos - 5 + 1));

			if (!tournament_file->Load(yaml))
			{
				delete tournament_file;
				return Error(Error::Type::InvalidFormat);
			}

			tournament_file->Save();
			AddTournament(tournament_file);//Add

			std::string output = R"(
<html>
	<head>
		<meta http-equiv = "refresh" content = "5; url=/#tournament_list.html"/>
	</head>
</html>
)";

			return "Parsing OK<br/><br/>" + output;
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
		const Account* account;
		auto error = CheckPermission(Request, Account::AccessLevel::User, &account);
		if (!error)
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


	m_Server.RegisterResource("/ajax/config/pause", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;
		return Ajax_PauseMat(Request);
	});


	m_Server.RegisterResource("/ajax/config/pause_all_mats", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_PauseAllMats(Request);
	});


	m_Server.RegisterResource("/ajax/config/set_mat", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;
		return Ajax_UpdateMat(Request);
	});

	m_Server.RegisterResource("/ajax/config/list_sound_files", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;
		return Ajax_ListSoundFiles();
	});

	m_Server.RegisterResource("/ajax/config/play_sound_file", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;
		return Ajax_PlaySoundFile(Request);
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

		auto guard = LockReadForScope();

		if (GetTournament() != &m_TempTournament)
			return Error();//OK
		return Error(Error::Type::ItemNotFound);
	});


	m_Server.RegisterResource("/ajax/get_schedule", [this](auto& Request) -> std::string {
		bool important_only = HttpServer::DecodeURLEncoded(Request.m_Query, "filter") == "2";
		int mat = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "mat"));

		auto guard = LockReadForScope();
		
		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		return GetTournament()->Schedule2String(important_only, mat);
	});


	m_Server.RegisterResource("/ajax/judoka/search", [this](auto& Request) -> std::string {
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		return Ajax_SearchJudoka(Request);
	});


	m_Server.RegisterResource("/ajax/hansokumake/get", [this](auto& Request) -> std::string {
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		return Ajax_GetHansokumake();
	});


	m_Server.RegisterResource("/ajax/get_masterschedule", [this](auto& Request) -> std::string {
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		auto guard = LockReadForScope();

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		return GetTournament()->MasterSchedule2String();
	});


	m_Server.RegisterResource("/ajax/masterschedule/move_up", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockReadForScope();

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		if (!GetTournament()->MoveScheduleEntryUp(id))
			return Error(Error::Type::OperationFailed);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/masterschedule/move_down", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockReadForScope();

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		if (!GetTournament()->MoveScheduleEntryDown(id))
			return Error(Error::Type::OperationFailed);
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/masterschedule/move", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_MoveMatchTable(Request);
	});


	m_Server.RegisterResource("/ajax/match/get", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto app_guard = LockReadForScope();

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		auto tournament_guard = GetTournament()->LockReadForScope();

		auto match = GetTournament()->FindMatch(id);

		if (!match)
			return Error(Error::Type::ItemNotFound);

		YAML::Emitter ret;
		match->ToString(ret);
		return ret.c_str();
	});


	m_Server.RegisterResource("/ajax/match/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		UUID whiteID = HttpServer::DecodeURLEncoded(Request.m_Body, "white");
		UUID blueID  = HttpServer::DecodeURLEncoded(Request.m_Body, "blue");
		int  matID   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat"));

		auto guard = LockReadForScope();

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		auto white = GetTournament()->GetDatabase().FindJudoka(whiteID);
		if (!white)
			white = m_Database.FindJudoka(whiteID);

		auto blue = GetTournament()->GetDatabase().FindJudoka(blueID);
		if (!blue)
			blue = m_Database.FindJudoka(blueID);

		if (!white || !blue)//Judokas exist?
			return std::string("Judoka not found in database");

		if (matID <= 0)//Illegal mat?
			matID = FindDefaultMatID();//Use the default

		if (!GetTournament()->AddMatch(Match(white, blue, GetTournament(), matID)))
			return Error(Error::Type::OperationFailed);

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/match/set_mat", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_SetMatOfMatch(Request);
	});

	m_Server.RegisterResource("/ajax/match/edit", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_EditMatch(Request);
	});

	m_Server.RegisterResource("/ajax/match/skip_break", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		return Ajax_SkipBreak(Request);
	});

	m_Server.RegisterResource("/ajax/match/revise", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ReviseMatch(Request);
	});

	m_Server.RegisterResource("/ajax/match/move_up", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::User);
		if (!error)
			return error;

		return Ajax_MoveMatchUp(Request);
	});

	m_Server.RegisterResource("/ajax/match/move_down", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::User);
		if (!error)
			return error;

		return Ajax_MoveMatchDown(Request);
	});

	m_Server.RegisterResource("/ajax/match/move_to", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::User);
		if (!error)
			return error;

		return Ajax_MoveMatchTo(Request);
	});

	m_Server.RegisterResource("/ajax/match/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockReadForScope();

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		if (!GetTournament()->RemoveMatch(id))
			return Error(Error::Type::OperationFailed);
		return Error(Error::Type::NoError);
	});

	m_Server.RegisterResource("/ajax/match/get_log", [this](auto& Request) -> std::string {
		auto account = IsLoggedIn(Request);
		if (!account)
			return Error(Error::Type::NotLoggedIn);

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto app_guard = LockReadForScope();

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		auto tournament_guard = GetTournament()->LockReadForScope();

		auto match = GetTournament()->FindMatch(id);

		if (!match)
			return Error(Error::Type::ItemNotFound);

		YAML::Emitter yaml;
		match->GetLog() >> yaml;
		return yaml.c_str();
	});


	//Mat commands
	m_Server.RegisterResource("/ajax/mat/stop_match", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		return Ajax_StopMatch(Request);
	});

	m_Server.RegisterResource("/ajax/mat/hajime", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		mat->Hajime();
		return Error();//OK
	});

	//Generic pause. Will call Mate() or Sonomama() if it's more appropriate
	m_Server.RegisterResource("/ajax/mat/pause", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

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
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		mat->Mate();
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/mat/sonomama", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int matID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (matID <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(matID);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		mat->Sonomama();
		return Error();//OK
	});

	//Serialization

	m_Server.RegisterResource("/ajax/mat/get_score", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		YAML::Emitter yaml;
		mat->ToString(yaml);
		return yaml.c_str();
	});

	m_Server.RegisterResource("/ajax/mat/get_osaekomilist", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		auto list = mat->GetOsaekomiList();
		ZED::CSV ret;
		for (const auto& entry : list)
			ret << entry.m_Who << entry.m_Time;

		return ret;
	});

	m_Server.RegisterResource("/ajax/mat/screenshot", [this](auto& Request) -> ZED::Blob {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return (std::string)Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return (std::string)Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

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
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->AddIppon(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-ippon", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveIppon(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+wazaari", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->AddWazaAri(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-wazaari", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveWazaAri(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+yuko", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->AddYuko(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-yuko", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveYuko(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+koka", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->AddKoka(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-koka", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveKoka(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/osaekomi", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->Osaekomi(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+shido", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->AddShido(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-shido", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveShido(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+hansokumake", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->AddHansokuMake(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-hansokumake", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveHansokuMake(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+disqualification", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);
			return Ajax_AddDisqualification(fighter, Request);
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-disqualification", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);
			return Ajax_RemoveDisqualification(fighter, Request);
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+nodisqualification", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);
			return Ajax_NoDisqualification(fighter, Request);
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-nodisqualification", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);
			return Ajax_RemoveNoDisqualification(fighter, Request);
		});


		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+medic", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->AddMedicalExamination(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-medic", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveMedicalExamination(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/+gachi", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->AddGachi(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/-gachi", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->RemoveGachi(fighter);
			return Error();//OK
		});

		m_Server.RegisterResource("/ajax/mat/" + Fighter2String(fighter) + "/hantei", [this, fighter](auto& Request) -> std::string {
			Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
			if (!IsLoggedIn(Request))
				return Error(Error::Type::NotLoggedIn);

			int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

			if (id <= 0)
				return Error(Error::Type::InvalidID);

			auto guard = LockReadForScope();

			auto mat = FindMat(id);

			if (mat)
				mat->Hantei(fighter);

			return Error();//OK
		});
	}


	m_Server.RegisterResource("/ajax/mat/-hantei", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (mat)
			mat->RevokeHantei();

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/mat/+draw", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (mat)
			mat->SetAsDraw();

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/mat/-draw", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (mat)
			mat->SetAsDraw(false);

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/mat/+golden_score", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		if (!mat->EnableGoldenScore())
			return Error(Error::Type::OperationFailed);

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/mat/-golden_score", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (!mat)
			return Error(Error::Type::MatNotFound);

		if (!mat->EnableGoldenScore(false))
			return Error(Error::Type::OperationFailed);

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/mat/tokeda", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		if (id <= 0)
			return Error(Error::Type::InvalidID);

		auto guard = LockReadForScope();

		auto mat = FindMat(id);

		if (mat)
			mat->Tokeda();
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/mat/names_on_mat", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response
		if (!IsLoggedIn(Request))
			return Error(Error::Type::NotLoggedIn);

		return Ajax_GetNamesOnMat(Request);
	});


	//Judoka

	m_Server.RegisterResource("/ajax/judoka/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_AddJudoka(Request);
	});


	m_Server.RegisterResource("/ajax/judoka/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_GetJudoka(Request);
	});


	m_Server.RegisterResource("/ajax/judoka/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_EditJudoka(Request);
	});


	m_Server.RegisterResource("/ajax/judoka/import", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ImportJudoka(Request);
	});


	m_Server.RegisterResource("/ajax/judoka/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		return Ajax_DeleteJudoka(Request);
	});


	//Clubs
	m_Server.RegisterResource("/ajax/club/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_AddClub(Request);
	});

	m_Server.RegisterResource("/ajax/club/edit", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_EditClub(Request);
	});

	m_Server.RegisterResource("/ajax/club/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		return Ajax_DeleteClub(Request);
	});

	m_Server.RegisterResource("/ajax/club/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_GetClub(Request);
	});

	m_Server.RegisterResource("/ajax/club/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ListClubs(Request);
	});


	m_Server.RegisterResource("/ajax/association/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ListAssociations(Request);
	});


	m_Server.RegisterResource("/ajax/lots/perform_lottery", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_PerformLottery();
	});


	m_Server.RegisterResource("/ajax/lots/get_tier", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_GetLotteryTier();
	});


	m_Server.RegisterResource("/ajax/lots/set_tier", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_SetLotteryTier(Request);
	});


	m_Server.RegisterResource("/ajax/lots/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ListLots();
	});


	m_Server.RegisterResource("/ajax/participant/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

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

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		if (GetTournament() && GetTournament()->RemoveParticipant(id))
			return Error();//OK

		return Error(Error::Type::OperationFailed);
	});


	m_Server.RegisterResource("/ajax/participant/weight/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::User);
		if (!error)
			return error;

		return Ajax_UpdateParticipantWeight(Request);
	});


	m_Server.RegisterResource("/ajax/account/add", [this](auto& Request) -> std::string {
		const Account* account;
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator, &account);
		if (!error)
			return error;

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
		const Account* account;
		auto error = CheckPermission(Request, Account::AccessLevel::User, &account);
		if (!error)
			return error;

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
		const Account* account;
		auto error = CheckPermission(Request, Account::AccessLevel::User, &account);
		if (!error)
			return error;

		return Ajax_UpdatePassword((Account*)account, Request);
	});


	m_Server.RegisterResource("/ajax/account/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		auto guard = LockWriteForScope();

		auto account_to_change = m_Database.SetAccount(index);
		if (!account_to_change)
			return std::string("Account not found");

		if (!m_Database.DeleteAccount(account_to_change->GetUsername()))
			return std::string("Failed");

		m_Database.Save();
		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/account/list", [this](auto& Request) -> std::string {
		const Account* account;
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator, &account);
		if (!error)
			return error;

		ZED::CSV ret;
		for (auto acc : m_Database.GetAccounts())
		{
			if (acc)
				ret << acc->ToString(account->GetAccessLevel());
		}
		return ret;
	});


	m_Server.RegisterResource("/ajax/account/get", [this](auto& Request) -> std::string {
		const Account* account;
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator, &account);
		if (!error)
			return error;

		int index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		auto account_to_change = m_Database.GetAccount(index);
		if (!account_to_change)
			return std::string("Account not found");

		if (account_to_change->GetAccessLevel() >= account->GetAccessLevel() && account->GetAccessLevel() != Account::AccessLevel::Admin)
			return std::string("Not enough permissions to change an account of this status");

		return account_to_change->ToString(account->GetAccessLevel());
	});


	m_Server.RegisterResource("/ajax/nonces/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

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

		auto guard = LockReadForScope();

		if (!GetTournament())
			return std::string("No tournament open");

		GetTournament()->LockRead();
		std::string ret;
		for (auto table : GetTournament()->GetMatchTables())
		{
			if (!table)
				continue;

			ret += table->ToHTML();
			ret += "<br/><br/><br/>";
		}
		GetTournament()->UnlockRead();

		return ret;
	});


	m_Server.RegisterResource("/ajax/filter/get_form", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int type = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "type"));

		switch ((IFilter::Type)type)
		{
		case IFilter::Type::Weightclass:
			return Weightclass::GetHTMLForm();

		default:
			return std::string("Unknown form");
		}
	});


	m_Server.RegisterResource("/ajax/fight_system/get_form", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		int type = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "type"));

		switch ((MatchTable::Type)type)
		{
			case MatchTable::Type::RoundRobin:
				return RoundRobin::GetHTMLForm();			
			case MatchTable::Type::Custom:
				return CustomTable::GetHTMLForm();
			case MatchTable::Type::SingleElimination:
				return SingleElimination::GetHTMLForm();
			case MatchTable::Type::Pool:
				return Pool::GetHTMLForm();
			case MatchTable::Type::DoubleElimination:
				return DoubleElimination::GetHTMLForm();

			default:
				return std::string("Unknown form");
		}
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

		return Ajax_AddMatchTable(Request);
	});



	m_Server.RegisterResource("/ajax/matchtable/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_EditMatchTable(Request);
	});


	m_Server.RegisterResource("/ajax/matchtable/set_start_pos", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_SetStartPosition(Request);
	});


	m_Server.RegisterResource("/ajax/matchtable/generate", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		auto guard = LockWriteForScope();

		if (!GetTournament())
			return std::string("No tournament is open");

		auto min   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "min"));
		auto max   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "max"));
		auto diff  = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "diff"));
		auto split = HttpServer::DecodeURLEncoded(Request.m_Body, "split_genders") == "true";
		auto apply = HttpServer::DecodeURLEncoded(Request.m_Body, "apply") == "true";

		if (min <= 0 || max <= 0 || diff <= 0)
			return Error(Error::Type::InvalidInput);

		auto age_groups = GetTournament()->GetAgeGroups();
		for (auto it = age_groups.begin(); it != age_groups.end();)
		{
			if (ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, (std::string)(*it)->GetUUID())) != 1)
				it = age_groups.erase(it);
			else
				++it;
		}

		auto descriptors = GetTournament()->GenerateWeightclasses(min, max, diff, age_groups, split);
		if (!apply)
		{
			YAML::Emitter yaml;
			yaml << YAML::BeginSeq;

			for (const auto& desc : descriptors)
				desc.ToString(yaml);

			yaml << YAML::EndSeq;
			return yaml.c_str();
		}

		if (!GetTournament()->ApplyWeightclasses(descriptors))
			return Error(Error::Type::OperationFailed);

		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/matchtable/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		auto guard = LockWriteForScope();

		if (!GetTournament())
			return std::string("No tournament is open");

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		if (!GetTournament()->RemoveMatchTable(id))
			return std::string("Failed to delete match table");

		return Error();//OK
	});


	m_Server.RegisterResource("/ajax/matchtable/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_GetMatchTable(Request);
	});


	m_Server.RegisterResource("/ajax/matchtable/get_all", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_GetMatchTables(Request);
	});


	m_Server.RegisterResource("/ajax/matchtable/move_all", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_MoveAllMatchTables(Request);
	});



	m_Server.RegisterResource("/ajax/match/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_AddMatch(Request);
	});


	m_Server.RegisterResource("/ajax/match/edit", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_EditMatch(Request);
	});

	m_Server.RegisterResource("/ajax/match/revise", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ReviseMatch(Request);
	});


	//Rule Sets

	m_Server.RegisterResource("/ajax/rule/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_AddRuleSet(Request);
	});

	m_Server.RegisterResource("/ajax/rule/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_EditRuleSet(Request);
	});

	m_Server.RegisterResource("/ajax/rule/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_GetRuleSet(Request);
	});

	m_Server.RegisterResource("/ajax/rule/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ListRuleSets();
	});

	m_Server.RegisterResource("/ajax/rule/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockWriteForScope();

		if (!m_Database.DeleteRuleSet(id))
			return Error(Error::Type::OperationFailed);

		m_Database.Save();
		return Error(Error::Type::NoError);//OK
	});


	//Age groups

	m_Server.RegisterResource("/ajax/age_groups/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_AddAgeGroup(Request);
	});

	m_Server.RegisterResource("/ajax/age_groups/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_EditAgeGroup(Request);
	});

	m_Server.RegisterResource("/ajax/age_groups/import", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ImportAgeGroup(Request);
	});

	m_Server.RegisterResource("/ajax/age_groups/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ListAllAgeGroups();
	});

	m_Server.RegisterResource("/ajax/age_groups/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_GetAgeGroup(Request);
	});

	m_Server.RegisterResource("/ajax/age_groups/select", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		UUID age_group_id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockWriteForScope();//In case the tournament gets closed at the same time

		auto age_group = m_Database.FindAgeGroup(age_group_id);

		if (!age_group)
			return Error(Error::Type::ItemNotFound);

		//Insert a copy
		if (!GetTournament()->AddAgeGroup(new AgeGroup(*age_group)))
			return Error(Error::Type::OperationFailed);

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/age_groups/remove", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		UUID age_group_id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockWriteForScope();//In case the tournament gets closed at the same time

		if (!GetTournament()->RemoveAgeGroup(age_group_id))
			return Error(Error::Type::OperationFailed);

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/age_groups/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_DeleteAgeGroup(Request);
	});


	//Tournaments

	m_Server.RegisterResource("/ajax/tournament/add", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_AddTournament(Request);
	});

	m_Server.RegisterResource("/ajax/tournament/update", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_EditTournament(Request);
	});

	m_Server.RegisterResource("/ajax/tournament/get", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_GetTournament(Request);
	});

	m_Server.RegisterResource("/ajax/tournament/assign_age_group", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_AssignAgeGroup(Request);
	});

	m_Server.RegisterResource("/ajax/tournament/open", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockWriteForScope();

		if (!OpenTournament(id))
			return std::string("Could not open tournament");

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/close", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		auto guard = LockWriteForScope();

		if (GetTournament())
		{
			GetTournament()->Save();
			ZED::Log::Info("Tournament saved");
		}

		if (!CloseTournament())
			return std::string("Could not close tournament");

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/swap_fighters", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		return Ajax_SwapMatchesOfTournament(Request);
	});

	m_Server.RegisterResource("/ajax/tournament/empty", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockWriteForScope();

		auto tournament = FindTournament(id);
		if (!tournament)
			return std::string("Could not find tournament");

		tournament->DeleteAllMatchResults();
		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/delete_matchless_tables", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;
		return Ajax_DeleteMatchlessMatchTables();
	});

	m_Server.RegisterResource("/ajax/tournament/delete_completed_tables", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;
		return Ajax_DeleteCompletedMatchTables();
	});

	m_Server.RegisterResource("/ajax/tournament/distribute_evenly", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;
		return Ajax_DistributeMatchTablesEvenly(Request);
	});

	m_Server.RegisterResource("/ajax/tournament/delete", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockWriteForScope();

		if (!DeleteTournament(id))
			return Error(Error::Type::OperationFailed);

		return Error();//OK
	});

	m_Server.RegisterResource("/ajax/tournament/download", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto tournament = FindTournament(id);
		if (!tournament)
			return std::string("Could not find tournament");

		std::string filename = "tournaments/" + tournament->GetName() + ".yml";

		Request.m_ResponseHeader = std::string("Content-Disposition: attachment; filename=") + tournament->GetName() + ".yml";

		return HttpServer::LoadFile(filename);
	}, HttpServer::ResourceType::Binary);

	m_Server.RegisterResource("/ajax/tournament/export-md5", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockReadForScope();

		auto tournament = FindTournament(id);
		if (!tournament)
			return std::string("Could not find tournament");

		//Convert to MD5 and save
		MD5 md5_tournament(*tournament);

		std::string filename = tournament->GetName() + ".md5";
		md5_tournament.Save(filename);

#ifdef _DEBUG
		md5_tournament.Dump();
#endif
		
		Request.m_ResponseHeader = std::string("Content-Disposition: attachment; filename=") + filename;

		return HttpServer::LoadFile(filename);
	}, HttpServer::ResourceType::Binary);


	m_Server.RegisterResource("/ajax/tournament/list", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Moderator);
		if (!error)
			return error;

		return Ajax_ListTournaments();
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

		if (!GetLocalMat())
			return "Could not find mat";

		GetLocalMat()->SetMatID(new_id);
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
		YAML::Node match_data = YAML::Load((const char*)Request.m_Body);

		for (auto mat : m_Mats)
		{
			if (mat && mat->GetMatID() == matID)
			{
				Match* match = new Match(match_data, nullptr, GetTournament());
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

	/*m_Server.RegisterResource("/ajax/slave/get_log", [this](auto& Request) -> std::string {
		Request.m_ResponseHeader = "Access-Control-Allow-Origin: *";//CORS response

		if (!IsSlave())
			return "You are not allowed to connect";

		//TODO check security token

		UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

		auto guard = LockReadForScope();

		if (!GetTournament())
			return Error(Error::Type::TournamentNotOpen);

		auto match = GetTournament()->FindMatch(id);

		if (!match)
			return Error(Error::Type::ItemNotFound);

		YAML::Emitter yaml;
		match->GetLog() >> yaml;
		return yaml.c_str();
	});*/

	m_Server.RegisterResource("/ajax/config/get_setup", [this](auto& Request) -> std::string {
		const bool is_admin = CheckPermission(Request, Account::AccessLevel::Admin);
		return Ajax_GetSetup(is_admin);
	});

	m_Server.RegisterResource("/ajax/config/set_setup", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		return Ajax_SetSetup(Request);
	});

	m_Server.RegisterResource("/ajax/config/execute", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		return Ajax_Execute(Request);
	});

	m_Server.RegisterResource("/ajax/config/shutdown", [this](auto& Request) -> std::string {
		auto error = CheckPermission(Request, Account::AccessLevel::Admin);
		if (!error)
			return error;

		auto guard = LockWriteForScope();

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

		auto guard = LockWriteForScope();

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

		auto mat = GetLocalMat();
		if (mat)
			if (!RegisterMatWithMaster(mat))
				ZED::Log::Error("Could not register mat with master");

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

		RemoteMat* new_mat = new RemoteMat(id, ip, port, GetAccessToken());
		new_mat->Open();

		auto guard = LockWriteForScope();

		SetMats().emplace_back(new_mat);

		YAML::Emitter yaml;
		yaml << YAML::BeginMap;
		yaml << YAML::Key << "id"          << YAML::Value << id;		
		yaml << YAML::Key << "language"    << YAML::Value << (int)Localizer::GetLanguage();
		yaml << YAML::Key << "ippon_style" << YAML::Value << (int)GetDatabase().GetIpponStyle();
		yaml << YAML::Key << "timer_style" << YAML::Value << (int)GetDatabase().GetTimerStyle();
		yaml << YAML::Key << "name_style"  << YAML::Value << (int)GetDatabase().GetNameStyle();
		yaml << YAML::Key << "token"       << YAML::Value << GetAccessToken();
		yaml << YAML::EndMap;
		return yaml.c_str();
	});

	m_Server.RegisterResource("/ajax/master/find_judoka", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		UUID uuid = HttpServer::DecodeURLEncoded(Request.m_Query, "uuid");

		//ZED::Log::Debug("Slave requested participant info");

		auto judoka = GetTournament()->FindParticipant(uuid);

		if (!judoka)
			return "Not found";

		YAML::Emitter yaml;
		*judoka >> yaml;
		return yaml.c_str();
	});

	m_Server.RegisterResource("/ajax/master/find_ruleset", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		UUID uuid = HttpServer::DecodeURLEncoded(Request.m_Query, "uuid");

		ZED::Log::Info("Slave requested rule set info");

		auto rule_set = GetTournament()->GetDatabase().FindRuleSet(uuid);

		if (!rule_set)
			return "Not found";

		YAML::Emitter yaml;
		*rule_set >> yaml;
		return yaml.c_str();
	});

	m_Server.RegisterResource("/ajax/master/find_match_table", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		UUID uuid = HttpServer::DecodeURLEncoded(Request.m_Query, "uuid");

		ZED::Log::Info("Slave requested match table info");

		auto match_table = GetTournament()->FindMatchTable(uuid);

		if (!match_table)
			return "Not found";

		YAML::Emitter yaml;
		*match_table >> yaml;
		return yaml.c_str();
	});

	m_Server.RegisterResource("/ajax/master/post_match_result", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		//TODO check security token

		ZED::Log::Info("Slave posted match results to us");

		//Extract UUID and result data
		YAML::Node match_data = YAML::Load((const char*)Request.m_Body);
		Match posted_match(match_data, nullptr, GetTournament());

		auto match = GetTournament()->FindMatch(posted_match.GetUUID());

		if (!match)
		{
			ZED::Log::Error("Could not store match result that was sent by slave");
			return "Not found";
		}

		match->SetResult(posted_match.GetResult());
		match->SetLog(posted_match.GetLog());
		match->EndMatch();
		return "ok";
	});

	m_Server.RegisterResource("/ajax/master/get_next_matches", [this](auto& Request) -> std::string {
		if (!IsMaster())
			return "You are not allowed to connect";

		//TODO check security token

		int matID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

		bool success = false;
		std::vector<const Match*> next_matches;

		while (!success)
		{
			next_matches = GetNextMatches(matID, success);
			if (!success)
				ZED::Core::Pause(100);
		}

		YAML::Emitter ret;
		ret << YAML::BeginSeq;

		for (auto match : next_matches)
			*match >> ret;

		ret << YAML::EndSeq;
		return ret.c_str();
	});
}



Error Application::Ajax_UpdatePassword(Account* Account, const HttpServer::Request& Request)
{
	if (!Account)
		return Error::Type::InternalError;

	auto password = HttpServer::DecodeURLEncoded(Request.m_Body, "password");

	if (password.length() <= 0)//Password not changed
		return Error::Type::InvalidInput;

	auto guard = LockWriteForScope();

	Account->SetPassword(password);
	m_Database.Save();
	return Error::Type::NoError;//OK
}



Error Application::Ajax_AddMatch(const HttpServer::Request& Request)
{
	UUID whiteID = HttpServer::DecodeURLEncoded(Request.m_Body, "white");
	UUID blueID  = HttpServer::DecodeURLEncoded(Request.m_Body, "blue");
	UUID match_tableID = HttpServer::DecodeURLEncoded(Request.m_Body, "match_table");
	UUID ruleID  = HttpServer::DecodeURLEncoded(Request.m_Body, "rule");
	int  matID   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat"));
  
	auto guard = LockReadForScope();

	auto tournament = GetTournament();
	if (!tournament)
		return Error(Error::Type::TournamentNotOpen);

	auto white = tournament->GetDatabase().FindJudoka(whiteID);
	if (!white)
		white = m_Database.FindJudoka(whiteID);

	auto blue = tournament->GetDatabase().FindJudoka(blueID);
	if (!blue)
		blue = m_Database.FindJudoka(blueID);

	if (!white || !blue)//Judokas exist?
		return Error(Error::Type::ItemNotFound);

	auto rule = tournament->FindRuleSet(ruleID);
	if (!rule)
		rule = m_Database.FindRuleSet(ruleID);

	auto match_table = tournament->FindMatchTable(match_tableID);

	auto match = new Match(white, blue, GetTournament());
	match->SetRuleSet(rule);

	if (match_table)
	{
		if (matID >= 1 && match_table->GetMatID() != matID)
			match->SetMatID(matID);
		if (rule && match_table->GetRuleSet() != *rule)
			match->SetRuleSet(rule);

		if (!match_table->AddMatch(match))
			return Error(Error::Type::OperationFailed);

		GetTournament()->OnUpdateMatchTable(*match_table);
	}

	else
	{
		if (matID <= 0)//Illegal mat?
			matID = FindDefaultMatID();//Use the default
		else
			match->SetMatID(matID);

		match->SetRuleSet(rule);

		if (!GetTournament()->AddMatch(match))
			return Error(Error::Type::OperationFailed);
	}
  	return Error();//OK
}


  
Error Application::Ajax_SetMatOfMatch(const HttpServer::Request& Request)
{
	UUID matchID = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	int matID    = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "mat"));

	if (matID < 0)
		return Error::Type::InvalidID;

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error::Type::TournamentNotOpen;

	auto tournament_guard = GetTournament()->LockWriteForScope();

	auto match = GetTournament()->FindMatch(matchID);

	if (!match)
		return Error::Type::ItemNotFound;
	if (match->HasConcluded() || match->IsRunning())
		return Error::Type::OperationFailed;

	match->SetMatID(matID);

	return Error();//OK
}



Error Application::Ajax_EditMatch(const HttpServer::Request& Request)
{
	UUID matchID = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	int matID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat"));
	UUID rule = HttpServer::DecodeURLEncoded(Request.m_Body, "rule");

	if (matID < 0)
		return Error::Type::InvalidID;

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error::Type::TournamentNotOpen;

	auto tournament_guard = GetTournament()->LockWriteForScope();

	auto match = GetTournament()->FindMatch(matchID);

	if (!match)
		return Error::Type::ItemNotFound;
	if (match->HasConcluded() || match->IsRunning())
		return Error::Type::OperationFailed;

	auto ruleSet = GetTournament()->FindRuleSet(rule);
	if (!ruleSet)
		ruleSet = m_Database.FindRuleSet(rule);

	match->SetMatID(matID);
	match->SetRuleSet(ruleSet);

	return Error();//OK
}



Error Application::Ajax_SkipBreak(const HttpServer::Request& Request)
{
	UUID matchID = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockReadForScope();

	auto tournament = GetTournament();
	if (!tournament)
		return Error::Type::TournamentNotOpen;

	auto match = tournament->FindMatch(matchID);
	if (!match)
		return Error::Type::ItemNotFound;

	auto white = match->GetFighter(Fighter::White);
	auto blue  = match->GetFighter(Fighter::Blue);

	if (white)
		((Judoka*)white)->SkipBreak();
	if (blue)
		((Judoka*)blue)->SkipBreak();

	return Error();//OK
}



Error Application::Ajax_ReviseMatch(const HttpServer::Request& Request)
{
	UUID matchID = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockReadForScope();

	auto tournament = GetTournament();
	if (!tournament)
		return Error::Type::TournamentNotOpen;
	if (!tournament->IsLocal())
		return Error::Type::InternalError;

	auto tournament_guard = GetTournament()->LockWriteForScope();

	auto match = tournament->FindMatch(matchID);
	if (!match)
		return Error::Type::ItemNotFound;

	auto mat = FindMat(match->GetMatID());
	if (!mat)
		return Error::Type::MatNotFound;

	if (! ((Tournament*)tournament)->ReviseMatch(matchID, *mat) )
		return Error::Type::OperationFailed;

	return Error();//OK
}



Error Application::Ajax_AddTournament(const HttpServer::Request& Request)
{
	auto name         = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	auto year         = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "year"));
	UUID rule_id      = HttpServer::DecodeURLEncoded(Request.m_Body, "rules");
	UUID organizer_id = HttpServer::DecodeURLEncoded(Request.m_Body, "organizer");
	auto description  = HttpServer::DecodeURLEncoded(Request.m_Body, "description");

	if (name.empty())
		return Error::Type::InvalidInput;

	if (FindTournamentByName(name))
		return Error::Type::OperationFailed;

	auto rules = m_Database.FindRuleSet(rule_id);
	if (!rules)
		ZED::Log::Warn("Adding tournament: Could not find rule set in database");

	auto organizer = m_Database.FindAssociation(organizer_id);

	Tournament* new_tournament = new Tournament(name);
	if (year > 0)
		new_tournament->SetYear(year);
	new_tournament->SetDefaultRuleSet(rules);
	new_tournament->SetOrganizer(organizer);
	new_tournament->SetDescription(description);

	if (!AddTournament(new_tournament))
		return Error::Type::OperationFailed;

	return Error();//OK
}



Error Application::Ajax_EditTournament(const HttpServer::Request& Request)
{
	UUID id           = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	auto name         = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	auto year         = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "year"));
	UUID rule_id      = HttpServer::DecodeURLEncoded(Request.m_Body, "rules");
	UUID organizer_id = HttpServer::DecodeURLEncoded(Request.m_Body, "organizer");
	auto description  = HttpServer::DecodeURLEncoded(Request.m_Body, "description");
	bool readonly     = HttpServer::DecodeURLEncoded(Request.m_Body, "readonly") == "true";

	if (name.empty())
		return Error::Type::InvalidInput;

	auto tournament = FindTournament(id);
	if (!tournament)
		return Error(Error::Type::ItemNotFound);

	//Check if the tournament is closed
	if (GetTournament() && tournament->GetName() == GetTournament()->GetName())
		return Error(Error::Type::OperationFailed);

	auto tournament_guard = tournament->LockWriteForScope();

	auto rules = m_Database.FindRuleSet(rule_id);
	if (!rules)
		ZED::Log::Warn("Adding tournament: Could not find rule set in database");

	if (!readonly)
		tournament->IsReadonly(readonly);

	auto organizer = m_Database.FindAssociation(organizer_id);

	auto old_name = tournament->GetName();
	tournament->SetName(name);
	if (year >= 0)
		tournament->SetYear(year);
	tournament->SetDefaultRuleSet(rules);
	tournament->SetOrganizer(organizer);
	tournament->SetDescription(description);

	if (readonly)
		tournament->IsReadonly(readonly);

	if (!readonly && !tournament->Save())
		return Error(Error::Type::OperationFailed);

	if (old_name != name)//Did the name change?
		ZED::Core::RemoveFile("tournaments/" + old_name + ".yml");//Remove the old file

	return Error();//OK
}



std::string Application::Ajax_GetTournament(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockReadForScope();

	auto tournament = FindTournament(id);
	if (!tournament)
		return (Error)Error::Type::TournamentNotOpen;

	YAML::Emitter yaml;
	yaml << YAML::BeginMap;

	yaml << YAML::Key << "name" << YAML::Value << tournament->GetName();
	yaml << YAML::Key << "year" << YAML::Value << tournament->GetDatabase().GetYear();
	yaml << YAML::Key << "num_participants" << YAML::Value << tournament->GetParticipants().size();
	yaml << YAML::Key << "schedule_size" << YAML::Value    << tournament->GetSchedule().size();
	yaml << YAML::Key << "status" << YAML::Value << (int)tournament->GetStatus();
	yaml << YAML::Key << "description" << YAML::Value << tournament->GetDescription();
	yaml << YAML::Key << "is_locked" << YAML::Value << tournament->IsReadonly();

	if (tournament->GetDefaultRuleSet())
		yaml << YAML::Key << "rule_set_uuid" << YAML::Value << (std::string)tournament->GetDefaultRuleSet()->GetUUID();

	if (tournament->GetOrganizer())
		yaml << YAML::Key << "organizer_uuid" << YAML::Value << (std::string)tournament->GetOrganizer()->GetUUID();

	yaml << YAML::EndMap;

	return yaml.c_str();
}



Error Application::Ajax_AssignAgeGroup(const HttpServer::Request& Request)
{
	UUID id           = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	UUID age_group_id = HttpServer::DecodeURLEncoded(Request.m_Query, "age");

	auto judoka    = GetTournament()->FindParticipant(id);
	auto age_group = GetTournament()->FindAgeGroup(age_group_id);

	if (!judoka || !age_group)
		return Error::Type::ItemNotFound;

	if (!GetTournament()->AssignJudokaToAgeGroup(judoka, age_group))
		return Error(Error::Type::OperationFailed);

	return Error();//OK
}



std::string Application::Ajax_ListTournaments()
{
	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	auto guard = LockReadForScope();

	for (auto tournament : m_Tournaments)
	{
		if (tournament)//Filter out temporary tournament
		{
			ret << YAML::BeginMap;
			ret << YAML::Key << "uuid" << YAML::Value << (std::string)tournament->GetUUID();
			ret << YAML::Key << "name" << YAML::Value << tournament->GetName();
			ret << YAML::Key << "num_participants" << YAML::Value << tournament->GetParticipants().size();
			ret << YAML::Key << "num_matches" << YAML::Value << tournament->GetSchedule().size();
			ret << YAML::Key << "status"  << YAML::Value << (int)tournament->GetStatus();
			ret << YAML::Key << "is_open" << YAML::Value << (m_CurrentTournament && tournament->GetUUID() == m_CurrentTournament->GetUUID());
			ret << YAML::Key << "is_locked" << YAML::Value << tournament->IsReadonly();
			ret << YAML::EndMap;
		}
	}
	
	ret << YAML::EndSeq;
	return ret.c_str();
}



Error Application::Ajax_SwapMatchesOfTournament(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto tournament = FindTournament(id);
	if (!tournament)
		return Error::Type::ItemNotFound;

	tournament->SwapAllFighters();
	return Error();//OK
}



Error Application::Ajax_DeleteMatchlessMatchTables()
{
	auto guard = LockWriteForScope();

	auto tournament = GetTournament();
	if (!tournament)
		return Error::Type::OperationFailed;

	auto tables = tournament->GetMatchTables();//Make copy of list
	for (auto table : tables)
	{
		if (table && table->GetNumberOfMatches() == 0)
			tournament->RemoveMatchTable(*table);//This modifies the original list
	}

	return Error::Type::NoError;
}



Error Application::Ajax_DeleteCompletedMatchTables()
{
	auto guard = LockReadForScope();

	auto tournament = GetTournament();
	if (!tournament)
		return Error::Type::OperationFailed;

	auto tables = tournament->GetMatchTables();//Make copy of list
	for (auto table : tables)
	{
		if (table && (table->GetStatus() == Status::Concluded || table->GetSchedule().empty()))
		{
			if (!tournament->RemoveMatchTable(*table))//This modifies the original list
				return Error::Type::OperationFailed;
		}
	}

	return Error::Type::NoError;
}



std::string Application::Ajax_GetMats() const
{
	YAML::Emitter ret;

	auto guard = LockReadForScope();

	if (GetTournament())
	{//Show all mats that are available/used and an additional one
		auto max = std::max(GetHighestMatID(), GetTournament()->GetHighestMatIDUsed()) + 1;

		ret << YAML::BeginMap;
		ret << YAML::Key << "highest_mat_id" << YAML::Value << max;

		ret << YAML::Key << "monitors" << YAML::Value;
		ret << YAML::BeginSeq;

		auto monitors = Window::EnumerateMonitors();
		for (auto& monitor : monitors)
		{
			ret << YAML::BeginMap;
			ret << YAML::Key << "index"  << YAML::Value << monitor.index;
			ret << YAML::Key << "width"  << YAML::Value << monitor.right;
			ret << YAML::Key << "height" << YAML::Value << monitor.bottom;
			ret << YAML::EndMap;
		}

		ret << YAML::EndSeq;

		ret << YAML::Key << "audio_devices" << YAML::Value;
		ret << YAML::BeginSeq;

		for (int i = 0; i < ZED::SoundDevice::GetNumberOfDevices(); i++)
		{
			ret << YAML::BeginMap;
			ret << YAML::Key << "index" << YAML::Value << i;
			ret << YAML::Key << "name"  << YAML::Value << ZED::SoundDevice::GetDeviceName(i);
			ret << YAML::EndMap;
		}

		ret << YAML::EndSeq;

		ret << YAML::Key << "mats" << YAML::Value;
		ret << YAML::BeginSeq;

		for (uint32_t id = 1; id <= max; id++)
		{
			auto mat = FindMat(id);

			if (!mat)
			{
				std::string mat_name = Localizer::Translate("Mat") + " " + std::to_string(id);

				ret << YAML::BeginMap;
				ret << YAML::Key << "id"   << YAML::Value << id;
				ret << YAML::Key << "name" << YAML::Value << mat_name;
				ret << YAML::EndMap;
			}
			else
			{
				ret << YAML::BeginMap;
				ret << YAML::Key << "id"      << YAML::Value << id;
				ret << YAML::Key << "name"    << YAML::Value << mat->GetName();
				ret << YAML::Key << "type"    << YAML::Value << (int)mat->GetType();
				ret << YAML::Key << "is_open" << YAML::Value << mat->IsOpen();
				ret << YAML::Key << "is_paused" << YAML::Value << mat->IsPaused();
				ret << YAML::Key << "ippon_style"    << YAML::Value << (int)mat->GetIpponStyle();
				ret << YAML::Key << "osaekomi_style" << YAML::Value << (int)mat->GetOsaekomiStyle();
				ret << YAML::Key << "timer_style"    << YAML::Value << (int)mat->GetTimerStyle();
				ret << YAML::Key << "name_style"     << YAML::Value << (int)mat->GetNameStyle();
				ret << YAML::Key << "is_fullscreen"  << YAML::Value << mat->IsFullscreen();
				ret << YAML::Key << "monitor"        << YAML::Value << mat->GetMonitor();
				ret << YAML::Key << "sound_enabled"  << YAML::Value << mat->IsSoundEnabled();
				ret << YAML::Key << "sound_filename" << YAML::Value << mat->GetSoundFilename();
				ret << YAML::Key << "sound_device"   << YAML::Value << mat->GetAudioDeviceID();

				if (mat->GetType() == IMat::Type::RemoteMat)
				{
					RemoteMat* rm = (RemoteMat*)mat;
					ret << YAML::Key << "hostname" << YAML::Value << rm->GetHostname();
					ret << YAML::Key << "port"     << YAML::Value << rm->GetPort();
					ret << YAML::Key << "token"    << YAML::Value << GetAccessToken();
				}

				ret << YAML::EndMap;
			}
		}

		ret << YAML::EndSeq;
		ret << YAML::EndMap;
	}

	return ret.c_str();
}



Error Application::Ajax_OpenMat(const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	if (id < 0)
		return Error::Type::InvalidID;

	auto guard = LockWriteForScope();

	if (id == 0)//Create new mat
	{
		id = 1;
		while (FindMat(id))//Find first empty id
			id++;

		if (!StartLocalMat(id))
			return Error::Type::OperationFailed;
		return Error();//OK
	}

	else
	{
		for (auto mat : SetMats())
		{
			if (mat && mat->GetMatID() == id)
			{
				mat->Open();
				return Error::Type::NoError;//OK
			}
		}
	}

	return Error::Type::OperationFailed;
}



Error Application::Ajax_CloseMat(const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	if (id <= 0)
		return Error::Type::InvalidID;

	//CloseMat() is thread-safe, so no need to lock

	if (CloseMat(id))
		return Error::Type::NoError;//OK

	return Error::Type::OperationFailed;
}



Error Application::Ajax_PauseMat(const HttpServer::Request& Request)
{
	int id      = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	bool enable = HttpServer::DecodeURLEncoded(Request.m_Query, "enable") == "true";

	if (id <= 0)
		return Error(Error::Type::InvalidID);

	auto guard = LockWriteForScope();

	auto mat = FindMat(id);

	if (!mat)
		return Error(Error::Type::MatNotFound);

	if (!mat->Pause(enable))
		return Error(Error::Type::OperationFailed);

	return Error();//OK
}



Error Application::Ajax_PauseAllMats(const HttpServer::Request& Request)
{
	bool enable = HttpServer::DecodeURLEncoded(Request.m_Query, "enable") == "true";

	auto guard = LockWriteForScope();

	bool success = true;
	for (auto mat : GetMats())
	{
		if (!mat || !mat->Pause(enable))
			success = false;	
	}

	if (!success)
		return Error(Error::Type::OperationFailed);

	return Error();//OK
}



Error Application::Ajax_UpdateMat(const HttpServer::Request& Request)
{
	int id     = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	int new_id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body,  "id"));

	bool fullscreen = HttpServer::DecodeURLEncoded(Request.m_Body, "fullscreen") == "true";
	int monitor = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "monitor"));

	auto name  = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	int ipponStyle = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "ipponStyle"));
	int osaekomiStyle = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomiStyle"));
	int timerStyle = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "timerStyle"));
	bool sound     = HttpServer::DecodeURLEncoded(Request.m_Body, "sound") == "true";
	std::string soundFilename = HttpServer::DecodeURLEncoded(Request.m_Body, "sound_filename");
	int audio_device = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "sound_device"));

	int nameStyle  = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "nameStyle"));

	if (id <= 0 || new_id <= 0)
		return Error::Type::InvalidID;
	if (ipponStyle <= -1)
		return Error::Type::InvalidInput;
	if (timerStyle <= -1)
		return Error::Type::InvalidInput;

	if (id != new_id)//Check if new_id is an unused id
	{
		auto guard = LockReadForScope();

		for (const auto mat : GetMats())
		{
			if (mat && mat->GetMatID() == new_id)
				return Error::Type::InvalidID;
		}
	}

	auto guard = LockWriteForScope();

	for (auto mat : SetMats())
	{
		if (mat && mat->GetMatID() == id)
		{
			if (id != new_id)
				mat->SetMatID(new_id);

			mat->SetFullscreen(fullscreen, monitor);
			mat->SetName(name);
			mat->SetIpponStyle((Mat::IpponStyle)ipponStyle);
			mat->SetOsaekomiStyle((Mat::OsaekomiStyle)osaekomiStyle);
			mat->SetTimerStyle((Mat::TimerStyle)timerStyle);
			mat->SetAudio(sound, soundFilename, audio_device);
			mat->SetNameStyle((NameStyle)nameStyle);

			return Error();//OK
		}
	}

	return Error::Type::MatNotFound;
}



Error Application::Ajax_AddJudoka(const HttpServer::Request& Request)
{
	auto firstname = HttpServer::DecodeURLEncoded(Request.m_Body, "firstname");
	auto lastname  = HttpServer::DecodeURLEncoded(Request.m_Body, "lastname");
	auto weight    = HttpServer::DecodeURLEncoded(Request.m_Body, "weight");
	Gender gender  = (Gender)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));
	int  birthyear = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "birthyear"));
	auto number = HttpServer::DecodeURLEncoded(Request.m_Body, "number");
	UUID clubID = HttpServer::DecodeURLEncoded(Request.m_Body, "club");

	if (!firstname.size() || !lastname.size() || (gender != Gender::Male && gender != Gender::Female))
		return Error::Type::InvalidInput;

	Judoka new_judoka(firstname, lastname, Weight(weight), gender);
	if (birthyear > 1900 && birthyear < 2100)
		new_judoka.SetBirthyear(birthyear);

	if (!number.empty())
		new_judoka.SetNumber(number);

	new_judoka.SetClub(GetDatabase().FindClub(clubID));

	auto guard = LockWriteForScope();

	m_Database.AddJudoka(std::move(new_judoka));
	m_Database.Save();
	
	return Error();//OK
}



std::string Application::Ajax_GetJudoka(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockReadForScope();

	auto judoka = m_Database.FindJudoka(id);

	if (judoka)
	{
		YAML::Emitter ret;
		judoka->ToString(ret);
		return ret.c_str();
	}

	//Search for participant
	if (!GetTournament())
		return Error(Error::Type::ItemNotFound);
	
	judoka = GetTournament()->FindParticipant(id);

	if (!judoka)
		return Error(Error::Type::ItemNotFound);

	YAML::Emitter ret;
	judoka->ToString(ret);
	return ret.c_str() + std::string("\nis_participant: true");
}



Error Application::Ajax_EditJudoka(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	auto firstname = HttpServer::DecodeURLEncoded(Request.m_Body, "firstname");
	auto lastname  = HttpServer::DecodeURLEncoded(Request.m_Body, "lastname");
	auto weight    = HttpServer::DecodeURLEncoded(Request.m_Body, "weight");
	Gender gender  = (Gender)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));
	int  birthyear = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "birthyear"));
	auto number = HttpServer::DecodeURLEncoded(Request.m_Body, "number");
	UUID clubID = HttpServer::DecodeURLEncoded(Request.m_Body, "club");

	if (!firstname.size() || !lastname.size() || (gender != Gender::Male && gender != Gender::Female))
		return Error::Type::InvalidInput;

	auto change_judoka = [&](Judoka* judoka) {
		if (!judoka)
			return;

		judoka->SetFirstname(firstname);
		judoka->SetLastname(lastname);
		judoka->SetWeight(Weight(weight));
		judoka->SetGender(gender);

		if (birthyear > 1800 && birthyear < 2200)
			judoka->SetBirthyear(birthyear);

		if (!number.empty())
			judoka->SetNumber(number);

		if (GetTournament()->GetDatabase().FindClub(clubID))
			judoka->SetClub(GetTournament()->GetDatabase().FindClub(clubID));
		else if (GetDatabase().FindClub(clubID))
			judoka->SetClub(GetDatabase().FindClub(clubID));
	};


	auto guard = LockWriteForScope();

	auto judoka = m_Database.FindJudoka(id);

	bool success = false;

	if (judoka)
	{
		change_judoka(judoka);
		success = true;
	}

	//Search for participant
	if (GetTournament())
	{
		judoka = GetTournament()->FindParticipant(id);

		if (judoka)
		{
			bool weight_changed = judoka->GetWeight() != Weight(weight);
			bool gender_changed = judoka->GetGender() != gender;
			bool birthyear_changed = judoka->GetBirthyear() != birthyear;

			change_judoka(judoka);
			success = true;

			if (weight_changed || gender_changed || birthyear_changed)
			{
				GetTournament()->MarkedAsWeighted(*judoka);
				GetTournament()->OnUpdateParticipant(*judoka);
			}
		}
	}

	if (!success)
		return Error(Error::Type::ItemNotFound);	

	return Error();//OK
}



std::string Application::Ajax_SearchJudoka(const HttpServer::Request& Request)
{
	auto search_string = HttpServer::DecodeURLEncoded(Request.m_Query, "name");
	auto tournament_search = HttpServer::DecodeURLEncoded(Request.m_Query, "participants") == "true";
	auto all_search = HttpServer::DecodeURLEncoded(Request.m_Query, "all") == "true";

	//Transform to lower case
	std::transform(search_string.begin(), search_string.end(), search_string.begin(),
		[](unsigned char c) { return std::tolower(c); });

	//Split by ' ' character
	std::vector<std::string> search_string_split;
	std::istringstream iss(search_string);
	std::string word;

	while (iss >> word)
		search_string_split.push_back(word);

	LockRead();

	std::vector<Judoka*> judokas;
	if (all_search)
	{
		judokas = GetTournament()->GetDatabase().GetAllJudokas();
		for (auto judoka : m_Database.GetAllJudokas())
		{
			if (std::find(judokas.begin(), judokas.end(), judoka) == judokas.end())
				judokas.push_back(judoka);
		}

		std::sort(judokas.begin(), judokas.end(), [](const Judoka* a, const Judoka* b)
		{
			if (a->GetLastname() != b->GetLastname())
				return a->GetLastname() < b->GetLastname();
			if (a->GetFirstname() != b->GetFirstname())
				return a->GetFirstname() < b->GetFirstname();
			return a->GetUUID() < b->GetUUID() ;
		});
	}
	else if (tournament_search)
		judokas = GetTournament()->GetDatabase().GetAllJudokas();
	else
		judokas = m_Database.GetAllJudokas();

	auto schedule = GetTournament()->GetSchedule();

	UnlockRead();


	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	for (auto judoka : judokas)
	{
		auto to_search = judoka->GetName(NameStyle::GivenName);

		auto club = judoka->GetClub();
		if (club)
			to_search += club->GetName();

		auto judoka_age_group = GetTournament()->GetAgeGroupOfJudoka(judoka);
		if (judoka_age_group)
			to_search += judoka_age_group->GetName();

		to_search += std::to_string(judoka->GetBirthyear());

		//Convert to lower case
		std::transform(to_search.begin(), to_search.end(), to_search.begin(),
			[](unsigned char c) { return std::tolower(c); });

		//Does the judoka match all the search criteria?
		bool matches = true;
		for (const auto& word : search_string_split)
		{
			if (to_search.find(word) == std::string::npos)
			{
				matches = false;
				break;
			}
		}

		if (!matches)
			continue;//Does not match search criteria			


		ret << YAML::BeginMap;

		ret << YAML::Key << "uuid" << YAML::Value << (std::string)judoka->GetUUID();
		ret << YAML::Key << "firstname" << YAML::Value << judoka->GetFirstname();
		ret << YAML::Key << "lastname" << YAML::Value << judoka->GetLastname();
		ret << YAML::Key << "gender" << YAML::Value << (int)judoka->GetGender();
		ret << YAML::Key << "weight" << YAML::Value << judoka->GetWeight().ToString();
		ret << YAML::Key << "birthyear" << YAML::Value << judoka->GetBirthyear();

		if (club)
		{
			ret << YAML::Key << "club_uuid" << YAML::Value << (std::string)judoka->GetClub()->GetUUID();
			ret << YAML::Key << "club_name" << YAML::Value << judoka->GetClub()->GetName();
		}

		uint32_t num_matches = 0;
		for (auto match : schedule)
		{
			if (match && match->HasValidFighters() && match->Contains(*judoka))
				num_matches++;
		}

		ret << YAML::Key << "num_matches" << YAML::Value << num_matches;

		if (judoka_age_group)
		{
			ret << YAML::Key << "age_group_uuid" << YAML::Value << (std::string)judoka_age_group->GetUUID();
			ret << YAML::Key << "age_group_name" << YAML::Value << (std::string)judoka_age_group->GetName();
		}


		LockRead();
		ret << YAML::Key << "is_participant" << YAML::Value << GetTournament()->IsParticipant(*judoka);
		ret << YAML::Key << "is_weighted"    << YAML::Value << GetTournament()->IsMarkedAsWeighted(*judoka);

		auto age_groups = GetTournament()->GetEligableAgeGroupsOfJudoka(judoka);
		UnlockRead();

		//Calculate eligable age groups
		ret << YAML::Key << "age_groups" << YAML::Value;
		ret << YAML::BeginSeq;

		for (auto age_group : age_groups)
		{
			ret << YAML::BeginMap;
			ret << YAML::Key << "uuid" << YAML::Value << (std::string)age_group->GetUUID();
			ret << YAML::Key << "name" << YAML::Value << age_group->GetName();
			ret << YAML::EndMap;
		}

		ret << YAML::EndSeq;

		ret << YAML::EndMap;
	}

	ret << YAML::EndSeq;
	return ret.c_str();
}



Error Application::Ajax_ImportJudoka(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockWriteForScope();

	auto judoka = GetTournament()->FindParticipant(id);

	if (!judoka)
		return Error::Type::ItemNotFound;

	Club* club = (Club*)judoka->GetClub();

	//Already in database?
	if (m_Database.FindJudoka(id))
		return Error::Type::OperationFailed;

	if (!m_Database.AddJudoka(judoka))
		return Error::Type::OperationFailed;

	if (club && !m_Database.FindClubByName(club->GetName()))
		if (!m_Database.AddClub(club))
			return Error::Type::OperationFailed;

	return Error();//OK
}



Error Application::Ajax_DeleteJudoka(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockWriteForScope();

	auto judoka = m_Database.FindJudoka(id);

	if (!judoka)
		return Error::Type::ItemNotFound;

	if (!m_Database.DeleteJudoka(judoka->GetUUID()))
		return Error::Type::OperationFailed;

	return Error();//OK
}



Error Application::Ajax_UpdateParticipantWeight(const HttpServer::Request& Request)
{
	UUID id       = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	Weight weight = Weight(HttpServer::DecodeURLEncoded(Request.m_Query, "weight"));

	auto guard = LockWriteForScope();

	if (!GetTournament())
		return Error::Type::TournamentNotOpen;

	auto judoka = GetTournament()->FindParticipant(id);
	if (!judoka)
		return Error::Type::ItemNotFound;

	judoka->SetWeight(weight);
	GetTournament()->MarkedAsWeighted(*judoka);
	GetTournament()->OnUpdateParticipant(*judoka);

	return Error::Type::NoError;
}



Error Application::Ajax_AddClub(const HttpServer::Request& Request)
{
	auto name      = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	auto shortname = HttpServer::DecodeURLEncoded(Request.m_Body, "shortname");
	bool is_assoc  = HttpServer::DecodeURLEncoded(Request.m_Query, "is_association") == "true";
	UUID parent_id = HttpServer::DecodeURLEncoded(Request.m_Body, "parent");

	if (name.length() == 0)
		return Error::Type::InvalidInput;

	auto guard = LockWriteForScope();

	Association* parent = nullptr;

	if (parent_id)
	{
		parent = m_Database.FindAssociation(parent_id);

		if (!parent)
			return Error::Type::OperationFailed;
	}

	if (!is_assoc)
	{
		auto new_club = new Club(name, parent);
		if (!shortname.empty())
			new_club->SetShortName(shortname);
		m_Database.AddClub(new_club);
	}
	else
	{
		auto new_assoc = new Association(name, parent);
		if (!shortname.empty())
			new_assoc->SetShortName(shortname);
		m_Database.AddAssociation(new_assoc);
	}

	m_Database.Save();
	return Error();//OK
}



std::string Application::Ajax_GetClub(const HttpServer::Request& Request)
{
	UUID id  = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockReadForScope();

	const Association* club = m_Database.FindAssociation(id);

	if (!club)
	{
		club = m_Database.FindClub(id);

		if (!club)
		{
			assert(GetTournament()->IsLocal());
			auto guard = GetTournament()->LockReadForScope();

			auto tour = (Tournament*)GetTournament();//TODO: could be remote tournament
			club = tour->GetDatabase().FindAssociation(id);

			if (!club)
				club = tour->GetDatabase().FindClub(id);

			if (!club)
				return Error(Error::Type::ItemNotFound);
		}
	}

	YAML::Emitter ret;
	club->ToString(ret);
	return ret.c_str();
}



Error Application::Ajax_EditClub(const HttpServer::Request& Request)
{
	UUID id        = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	auto name      = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	auto shortname = HttpServer::DecodeURLEncoded(Request.m_Body, "shortname");
	UUID parent_id = HttpServer::DecodeURLEncoded(Request.m_Body, "parent");

	auto guard = LockWriteForScope();

	auto club   = m_Database.FindAssociation(id);
	auto parent = m_Database.FindAssociation(parent_id);

	if (!club)
	{
		club = m_Database.FindClub(id);

		if (!club)
			return Error(Error::Type::ItemNotFound);
	}

	if (!name.empty())
		club->SetName(name);
	if (!shortname.empty())
		club->SetShortName(shortname);
	if (parent)
		club->SetParent(parent);

	return Error::Type::NoError;
}



Error Application::Ajax_DeleteClub(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockWriteForScope();

	auto assoc = m_Database.FindAssociation(id);

	if (assoc)
	{
		if (!m_Database.DeleteAssociation(id))
			return Error(Error::Type::OperationFailed);

		if (GetTournament())
			GetTournament()->RemoveAssociation(id);

		return Error::Type::NoError;
	}

	else
	{
		auto club = m_Database.FindClub(id);

		if (!club)
			return Error(Error::Type::ItemNotFound);

		if (!m_Database.DeleteClub(id))
			return Error(Error::Type::OperationFailed);

		if (GetTournament())
			GetTournament()->RemoveClub(id);

		return Error::Type::NoError;
	}

	return Error(Error::Type::ItemNotFound);
}



std::string Application::Ajax_ListClubs(const HttpServer::Request& Request)
{
	bool all = HttpServer::DecodeURLEncoded(Request.m_Query, "all") == "true";

	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	auto guard = LockReadForScope();

	if (all && GetTournament())
	{
		Tournament* tour = (Tournament*)GetTournament();//TODO: could be remote tournament

		tour->LockRead();
		for (auto club : tour->GetDatabase().GetAllClubs())
		{
			if (club)
				*club >> ret;
		}
		tour->UnlockRead();
	}

	for (auto club : m_Database.GetAllClubs())
	{
		if (club)
			*club >> ret;
	}

	ret << YAML::EndSeq;
	return ret.c_str();
}



std::string Application::Ajax_ListAssociations(const HttpServer::Request& Request)
{
	bool only_children = HttpServer::DecodeURLEncoded(Request.m_Query, "only_children") == "true";
	bool also_clubs    = HttpServer::DecodeURLEncoded(Request.m_Query, "also_clubs")    == "true";

	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	auto guard = LockReadForScope();

	for (auto assoc : m_Database.GetAllAssociations())
	{
		if (assoc)
		{
			if (only_children && m_Database.AssociationHasChildren(assoc))
				continue;

			assoc->ToString(ret);
		}
	}

	if (also_clubs)
		for (auto club : m_Database.GetAllClubs())
			if (club)
				club->ToString(ret);

	ret << YAML::EndSeq;
	return ret.c_str();
}



Error Application::Ajax_AddRuleSet(const HttpServer::Request& Request)
{
	std::string name = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	int match_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "match_time"));
	int goldenscore_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "goldenscore_time"));
	int osaekomi_ippon   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_ippon"));
	int osaekomi_wazaari = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_wazaari"));
	bool yuko = HttpServer::DecodeURLEncoded(Request.m_Body, "yuko") == "true";
	bool koka = HttpServer::DecodeURLEncoded(Request.m_Body, "koka") == "true";
	bool draw = HttpServer::DecodeURLEncoded(Request.m_Body, "draw") == "true";
	int break_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "break_time"));
	bool extend_break_time = HttpServer::DecodeURLEncoded(Request.m_Body, "extend_break_time") == "true";

	RuleSet* new_rule_set = new RuleSet(name, match_time, goldenscore_time, osaekomi_ippon, osaekomi_wazaari, yuko, koka, draw, break_time, extend_break_time);

	auto guard = LockWriteForScope();

	if (!m_Database.AddRuleSet(new_rule_set))
		return Error(Error::Type::OperationFailed);

	m_Database.Save();
	return Error::Type::NoError;//OK
}



Error Application::Ajax_EditRuleSet(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	std::string name = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	int match_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "match_time"));
	int goldenscore_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "goldenscore_time"));
	int osaekomi_ippon   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_ippon"));
	int osaekomi_wazaari = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomi_wazaari"));
	bool yuko = HttpServer::DecodeURLEncoded(Request.m_Body, "yuko") == "true";
	bool koka = HttpServer::DecodeURLEncoded(Request.m_Body, "koka") == "true";
	bool draw = HttpServer::DecodeURLEncoded(Request.m_Body, "draw") == "true";
	int break_time = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "break_time"));
	bool extend_break_time = HttpServer::DecodeURLEncoded(Request.m_Body, "extend_break_time") == "true";

	auto guard = LockWriteForScope();

	auto rule = m_Database.FindRuleSet(id);

	if (!rule)
	{
		if (GetTournament())
			rule = GetTournament()->FindRuleSet(id);
		if (!rule)
			return Error::Type::ItemNotFound;
	}

	*rule = RuleSet(name, match_time, goldenscore_time, osaekomi_ippon, osaekomi_wazaari, yuko, koka, draw, break_time, extend_break_time);
	m_Database.Save();
	return Error::Type::NoError;//OK
}



std::string Application::Ajax_GetRuleSet(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockReadForScope();

	auto rule = m_Database.FindRuleSet(id);
	if (!rule)
	{
		rule = GetTournament()->GetDatabase().FindRuleSet(id);
		if (!rule)
			return Error(Error::Type::ItemNotFound);
	}

	YAML::Emitter ret;
	*rule >> ret;
	return ret.c_str();
}



std::string Application::Ajax_ListRuleSets()
{
	auto guard = LockReadForScope();

	auto& rules = m_Database.GetRuleSets();
	YAML::Emitter ret;

	ret << YAML::BeginMap;

	if (GetTournament() && GetTournament()->GetDefaultRuleSet())
		ret << YAML::Key << "default" << YAML::Value << (std::string)GetTournament()->GetDefaultRuleSet()->GetUUID();

	ret << YAML::Key << "rules" << YAML::Value;
	ret << YAML::BeginSeq;

	for (auto rule : rules)
	{
		if (rule)
		{
			ret << YAML::BeginMap;
			ret << YAML::Key << "uuid" << YAML::Value << (std::string)rule->GetUUID();
			ret << YAML::Key << "name" << YAML::Value << (std::string)rule->GetName();
			ret << YAML::Key << "desc" << YAML::Value << (std::string)rule->GetDescription();
			ret << YAML::EndMap;
		}
	}

	ret << YAML::EndSeq;
	ret << YAML::EndMap;

	return ret.c_str();
}



Error Application::Ajax_AddAgeGroup(const HttpServer::Request& Request)
{
	auto name     = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	int min_age   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "min_age"));
	int max_age   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "max_age"));
	Gender gender = (Gender)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));
	UUID rule_id  = HttpServer::DecodeURLEncoded(Request.m_Body, "rule");

	auto guard = LockWriteForScope();

	auto rule  = GetDatabase().FindRuleSet(rule_id);

	if (!rule)
		ZED::Log::Warn("Could not find rule set.");

	auto new_age_group = new AgeGroup(name, min_age, max_age, rule, gender);

	if (!GetDatabase().AddAgeGroup(new_age_group))
	{
		delete new_age_group;
		return Error::Type::OperationFailed;
	}

	return Error::Type::NoError;
}



Error Application::Ajax_EditAgeGroup(const HttpServer::Request& Request)
{
	UUID id       = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	auto name     = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	int min_age   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "min_age"));
	int max_age   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "max_age"));
	Gender gender = (Gender)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));
	UUID rule_id  = HttpServer::DecodeURLEncoded(Request.m_Body, "rule");

	auto guard = LockWriteForScope();

	auto age_group = GetDatabase().FindAgeGroup(id);

	if (!age_group)
	{
		age_group = GetTournament()->FindAgeGroup(id);

		if (!age_group)
			return Error::Type::InvalidID;
	}

	auto rule = GetDatabase().FindRuleSet(rule_id);

	if (!rule)
		ZED::Log::Warn("Could not find rule set.");

	age_group->SetName(name);
	age_group->SetMinAge(min_age);
	age_group->SetMaxAge(max_age);
	age_group->SetGender(gender);
	age_group->SetRuleSet(rule);

	GetTournament()->AddRuleSet(rule);

	return Error::Type::NoError;
}



Error Application::Ajax_ImportAgeGroup(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockWriteForScope();

	auto age_group = GetTournament()->FindAgeGroup(id);

	if (!age_group)
		return Error::Type::ItemNotFound;

	//Already in database?
	if (m_Database.FindAgeGroup(id))
		return Error::Type::OperationFailed;

	if (!m_Database.AddAgeGroup(age_group))
		return Error::Type::OperationFailed;

	return Error();//OK
}



std::string Application::Ajax_GetAgeGroup(const HttpServer::Request& Request) const
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	YAML::Emitter ret;

	auto guard = LockReadForScope();

	auto age_group = GetDatabase().FindAgeGroup(id);
	bool in_database = true;

	if (!age_group)
	{
		in_database = false;
		age_group = GetTournament()->FindAgeGroup(id);
	}

	if (age_group)
	{
		ret << YAML::BeginMap;
		age_group->ToString(ret);
		ret << YAML::Key << "in_db" << YAML::Value << in_database;
		ret << YAML::EndMap;
	}

	return ret.c_str();
}



std::string Application::Ajax_ListAllAgeGroups() const
{
	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	auto guard = LockReadForScope();

	for (const auto age_group : GetDatabase().GetAgeGroups())
	{
		if (age_group)
		{
			ret << YAML::BeginMap;

			age_group->ToString(ret);

			bool is_used = false;
			if (GetTournament())
				is_used = GetTournament()->FindAgeGroup(age_group->GetUUID());

			if (is_used)
				GetTournament()->GetAgeGroupInfo(ret, age_group);

			ret << YAML::Key << "is_used" << YAML::Value << is_used;
			ret << YAML::Key << "in_db"   << YAML::Value << true;

			ret << YAML::EndMap;
		}
	}

	//Get all age groups that are exclusive to the tournament
	for (const auto age_group : GetTournament()->GetDatabase().GetAgeGroups())
	{
		if (age_group && !GetDatabase().FindAgeGroup(*age_group))
		{
			ret << YAML::BeginMap;

			age_group->ToString(ret);

			GetTournament()->GetAgeGroupInfo(ret, age_group);

			ret << YAML::Key << "is_used" << YAML::Value << true;
			ret << YAML::Key << "in_db"   << YAML::Value << false;

			ret << YAML::EndMap;
		}
	}

	ret << YAML::EndSeq;
	return ret.c_str();
}



Error Application::Ajax_DeleteAgeGroup(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockWriteForScope();

	if (!GetDatabase().RemoveAgeGroup(id))
		return Error::Type::OperationFailed;

	return Error::Type::NoError;
}



Error Application::Ajax_AddMatchTable(HttpServer::Request Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	IFilter::Type type = (IFilter::Type)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "type"));
	MatchTable::Type fight_system = (MatchTable::Type)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "fight_system"));

	MatchTable* new_table = nullptr;

	switch (fight_system)
	{
	case MatchTable::Type::RoundRobin:
	{
		new_table = new RoundRobin(new Weightclass(0, 0));
		break;
	}

	case MatchTable::Type::SingleElimination:
	{
		new_table = new SingleElimination(new Weightclass(0, 0));
		break;
	}

	case MatchTable::Type::Pool:
		new_table = new Pool(new Weightclass(0, 0));
		break;

	case MatchTable::Type::DoubleElimination:
		new_table = new DoubleElimination(new Weightclass(0, 0));
		break;

	case MatchTable::Type::Custom:
		new_table = new CustomTable();
		break;

	default:
		return Error::Type::InvalidInput;
	}

	if (id)
		new_table->SetUUID(std::move(id));

	auto guard = LockWriteForScope();

	if (!GetTournament())
	{
		delete new_table;
		return Error::Type::TournamentNotOpen;
	}

	GetTournament()->AddMatchTable(new_table);

	Request.m_Query = "id=" + (std::string)new_table->GetUUID();
	if (!Ajax_EditMatchTable(Request))
		return Error::Type::OperationFailed;

	return Error();//OK
}



Error Application::Ajax_EditMatchTable(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	IFilter::Type type = (IFilter::Type)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "type"));
	MatchTable::Type fight_system = (MatchTable::Type)ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "fight_system"));
	auto name = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	int color = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "color"));
	int mat   = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat"));
	UUID age_group_id = HttpServer::DecodeURLEncoded(Request.m_Body, "age_group");
	UUID rule_set_id  = HttpServer::DecodeURLEncoded(Request.m_Body, "rule");


	auto guard = LockWriteForScope();

	if (!GetTournament())
		return Error::Type::TournamentNotOpen;

	auto table = GetTournament()->FindMatchTable(id);

	if (!table)
		return Error::Type::ItemNotFound;

	//Change fight system?
	if (!table->IsSubMatchTable() && table->GetType() != fight_system)//Don't support sub match tables
	{
		//Re-create match table

		auto color = table->GetColor();
		auto schedule_index = table->GetScheduleIndex();

		if (!GetTournament()->RemoveMatchTable(*table))
			return Error::Type::OperationFailed;

		auto error = Ajax_AddMatchTable(Request);
		if (!error)
		{
			GetTournament()->AddMatchTable(table);//Add match table again since we just deleted it
			return error;
		}

		table = GetTournament()->FindMatchTable(id);

		if (!table)
			return Error(Error::Type::ItemNotFound);

		table->SetColor(color);
		table->SetScheduleIndex(schedule_index);
	}

	auto age_group = m_Database.FindAgeGroup(age_group_id);
	if (!age_group)
		age_group = GetTournament()->FindAgeGroup(age_group_id);
	auto rule_set = m_Database.FindRuleSet(rule_set_id);
	if (!rule_set)
		rule_set = GetTournament()->FindRuleSet(rule_set_id);

	GetTournament()->LockWrite();

	if (color >= 0)
		table->SetColor(color);

	table->SetName(name);

	if (mat >= 0)
		table->SetMatID(mat);

	table->SetAgeGroup(age_group);
	GetTournament()->AddAgeGroup(age_group);

	table->SetRuleSet(rule_set);
	GetTournament()->AddRuleSet(rule_set);

	GetTournament()->UnlockWrite();


	//Update filter

	if (!table->IsSubMatchTable() && table->GetFilter() && table->GetFilter()->GetType() == IFilter::Type::Weightclass)
	{
		auto weightclass = (Weightclass*)table->GetFilter();

		auto minWeight = HttpServer::DecodeURLEncoded(Request.m_Body, "minWeight");
		auto maxWeight = HttpServer::DecodeURLEncoded(Request.m_Body, "maxWeight");
		int  gender = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "gender"));

		weightclass->SetMinWeight(Weight(minWeight));
		weightclass->SetMaxWeight(Weight(maxWeight));
		weightclass->SetGender((Gender)gender);
	}


	//Update fight system

	bool bo3 = HttpServer::DecodeURLEncoded(Request.m_Body, "bo3") == "true";

	switch (table->GetType())
	{
		case MatchTable::Type::RoundRobin:
		{
			RoundRobin* round_robin = (RoundRobin*)table;

			GetTournament()->LockWrite();

			round_robin->IsBestOfThree(bo3);

			GetTournament()->UnlockWrite();
			break;
		}

		case MatchTable::Type::Custom:
		{
			CustomTable* custom = (CustomTable*)table;

			GetTournament()->LockWrite();

			custom->IsBestOfThree(bo3);

			GetTournament()->UnlockWrite();
			break;
		}

		case MatchTable::Type::SingleElimination:
		{
			SingleElimination* single_table = (SingleElimination*)table;

			GetTournament()->LockWrite();

			single_table->IsBestOfThree(bo3);
			single_table->IsThirdPlaceMatch(HttpServer::DecodeURLEncoded(Request.m_Body, "mf3") == "true");
			single_table->IsFifthPlaceMatch(HttpServer::DecodeURLEncoded(Request.m_Body, "mf5") == "true");

			GetTournament()->UnlockWrite();
			break;
		}

		case MatchTable::Type::Pool:
		{
			Pool* pool = (Pool*)table;

			GetTournament()->LockWrite();

			pool->IsBestOfThree(bo3);
			pool->IsThirdPlaceMatch(HttpServer::DecodeURLEncoded(Request.m_Body, "mf3") == "true");
			pool->IsFifthPlaceMatch(HttpServer::DecodeURLEncoded(Request.m_Body, "mf5") == "true");

			GetTournament()->UnlockWrite();
			break;
		}

		case MatchTable::Type::DoubleElimination:
		{
			DoubleElimination* doubleEli = (DoubleElimination*)table;

			GetTournament()->LockWrite();

			doubleEli->IsBestOfThree(bo3);
			doubleEli->IsThirdPlaceMatch(HttpServer::DecodeURLEncoded(Request.m_Body, "mf3") == "true");
			doubleEli->IsFifthPlaceMatch(HttpServer::DecodeURLEncoded(Request.m_Body, "mf5") == "true");

			GetTournament()->UnlockWrite();
			break;
		}

		default:
			return Error(Error::Type::InternalError);
	}

	if (!GetTournament()->OnUpdateMatchTable(id))
		return Error(Error::Type::OperationFailed);
	return Error();//OK
}



Error Application::Ajax_MoveMatchTable(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	int  schedule_index = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "schedule_index"));
	int  mat = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "mat"));

	if (mat <= 0 && schedule_index < 0)
		return Error::Type::InvalidInput;

	auto guard = LockWriteForScope();

	if (!GetTournament())
		return Error::Type::TournamentNotOpen;

	if (GetTournament()->GetStatus() == Status::Concluded)
		return Error::Type::OperationFailed;

	auto entry = GetTournament()->FindMatchTable(id);

	if (!entry)
		return Error::Type::ItemNotFound;

	if (mat >= 1)
		entry->SetMatID(mat);
	if (schedule_index >= 0 && entry->GetScheduleIndex() != schedule_index)
	{
		entry->SetScheduleIndex(schedule_index);
		GetTournament()->BuildSchedule();
	}

	return Error::Type::NoError;//OK
}



Error Application::Ajax_MoveAllMatchTables(const HttpServer::Request& Request)
{
	int mat = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "mat"));

	if (mat <= 0)
		return Error::Type::InvalidInput;

	auto guard = LockWriteForScope();

	if (!GetTournament())
		return Error::Type::TournamentNotOpen;

	if (GetTournament()->GetStatus() == Status::Concluded)
		return Error::Type::OperationFailed;

	auto& match_tables = GetTournament()->GetMatchTables();
	for (auto table : match_tables)
		table->SetMatID(mat);

	return Error::Type::NoError;//OK
}



std::string Application::Ajax_GetMatchTable(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	auto tournament_guard = GetTournament()->LockReadForScope();

	auto match_table = GetTournament()->FindMatchTable(id);

	if (!match_table)
		return Error(Error::Type::ItemNotFound);

	YAML::Emitter ret;
	ret << YAML::BeginMap;
	match_table->ToString(ret);
	ret << YAML::EndMap;

	return ret.c_str();
}



std::string Application::Ajax_GetMatchTables(const HttpServer::Request& Request)
{
	auto guard = LockReadForScope();
	auto tournament = GetTournament();

	if (!tournament)
		return Error(Error::Type::TournamentNotOpen);

	auto tournament_guard = tournament->LockReadForScope();

	auto match_tables = tournament->GetMatchTables();

	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	for (auto table : match_tables)
	{
		ret << YAML::BeginMap;
		table->ToString(ret);
		ret << YAML::EndMap;
	}

	ret << YAML::EndSeq;
	return ret.c_str();
}



std::string Application::Ajax_GetParticipantsFromMatchTable(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	auto table = GetTournament()->FindMatchTable(id);

	if (!table)
		return Error(Error::Type::ItemNotFound);


	GetTournament()->LockRead();

	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	for (auto judoka : table->GetParticipants())
		judoka->ToString(ret);

	ret << YAML::EndSeq;
	GetTournament()->UnlockRead();
	
	return ret.c_str();
}



std::string Application::Ajax_GetMatchesFromMatchTable(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	auto table = GetTournament()->FindMatchTable(id);

	if (!table)
		return Error(Error::Type::ItemNotFound);


	GetTournament()->LockRead();

	YAML::Emitter ret;
	ret << YAML::BeginSeq;
	for (auto match : table->GetSchedule())
		match->ToString(ret);
	ret << YAML::EndSeq;

	GetTournament()->UnlockRead();

	return ret.c_str();
}



Error Application::Ajax_SetStartPosition(const HttpServer::Request& Request)
{
	UUID id        = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	UUID judoka_id = HttpServer::DecodeURLEncoded(Request.m_Query, "judoka");
	int  startpos  = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "startpos"));

	if (startpos < 0)
		return Error::Type::InvalidInput;

	auto guard = LockWriteForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	if (GetTournament()->GetStatus() != Status::Scheduled)
		return Error::Type::OperationFailed;

	auto table = GetTournament()->FindMatchTable(id);

	if (!table)
		return Error::Type::ItemNotFound;

	auto judoka = table->FindParticipant(judoka_id);

	if (!judoka)
		return Error::Type::ItemNotFound;

	table->SetStartPosition(judoka, startpos);
	GetTournament()->GenerateSchedule();

	return Error::Type::NoError;//OK
}



Error Application::Ajax_DistributeMatchTablesEvenly(const HttpServer::Request& Request)
{
	int mats = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "mats"));
	int tables_simultaneous = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "simultaneous"));

	if (mats <= 0)
		mats = 1;
	if (tables_simultaneous <= 0)
		tables_simultaneous = 1;

	auto guard = LockWriteForScope();

	auto tournament = GetTournament();
	if (!tournament)
		return Error(Error::Type::TournamentNotOpen);

	if (tournament->GetStatus() == Status::Concluded)
		return Error::Type::OperationFailed;

	auto age_groups = GetTournament()->GetAgeGroups();
	auto tables = GetTournament()->GetMatchTables();

	//Sort by age group
	std::sort(tables.begin(), tables.end(),
		[](const auto& a, const auto& b) {
			auto age1 = a->GetAgeGroup();
			auto age2 = b->GetAgeGroup();

			if (!age1 && !age2)
				return a->GetUUID() < b->GetUUID();
			if (age1 && !age2)
				return true;
			if (!age1 && age2)
				return false;
			if (age1->GetMinAge() != age2->GetMinAge())
				return age1->GetMinAge() < age2->GetMinAge();
			return age1->GetUUID() < age2->GetUUID();
		});


	//Distribute evenly accross time and mats
	int sub_index = 0;
	int index = 0;
	int mat_index = 0;
	for (auto table : tables)
	{
		table->SetMatID(mat_index + 1);
		table->SetScheduleIndex(index);

		sub_index++;
		if (sub_index % tables_simultaneous == 0)
		{
			mat_index++;//Next mat

			if (mat_index % mats == 0)//Out of mats?
			{
				index++;//Next schedule index
				mat_index = 0;//Start with first mat
			}
		}
	}

	return Error::Type::NoError;//OK
}



Error Application::Ajax_PerformLottery()
{
	auto guard = LockWriteForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	if (!GetTournament()->PerformLottery())
		return Error::Type::OperationFailed;

	return Error::Type::NoError;//OK
}



std::string Application::Ajax_GetLotteryTier()
{
	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	YAML::Emitter ret;

	ret << YAML::BeginMap;
	if (GetTournament()->GetOrganizer())
	{
		ret << YAML::Key << "organizer"      << YAML::Value << (std::string)GetTournament()->GetOrganizer()->GetUUID();
		ret << YAML::Key << "organizer_name" << YAML::Value << (std::string)GetTournament()->GetOrganizer()->GetName();
		ret << YAML::Key << "organizer_tier" << YAML::Value << GetTournament()->GetOrganizer()->GetLevel();
	}
	ret << YAML::Key << "tier" << YAML::Value << GetTournament()->GetLotteryTier();
	ret << YAML::EndMap;

	return ret.c_str();
}



Error Application::Ajax_SetLotteryTier(const HttpServer::Request& Request)
{
	int tier = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "tier"));

	if (tier < 0)
		return Error::Type::InvalidInput;

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	GetTournament()->SetLotteryTier(tier);

	return Error::Type::NoError;
}



std::string Application::Ajax_ListLots()
{
	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	auto lots = GetTournament()->GetLots();

	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	for (auto [assoc_id, lot] : lots)
	{
		ret << YAML::BeginMap;
		ret << YAML::Key << "uuid" << YAML::Value << (std::string)assoc_id;
		ret << YAML::Key << "lot" << YAML::Value << lot;

		auto assoc = GetTournament()->FindAssociation(assoc_id);
		if (!assoc)
			assoc = GetTournament()->FindClub(assoc_id);

		if (assoc)
			ret << YAML::Key << "name" << YAML::Value << assoc->GetName();

		ret << YAML::EndMap;
	}

	ret << YAML::EndSeq;
	return ret.c_str();
}



std::string Application::Ajax_ListSoundFiles()
{
	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	ZED::Core::Indexer([&](const std::string& Filename) {
		auto pos = Filename.find_last_of(ZED::Core::Separator);
		if (pos == std::string::npos) return true;

		auto onlyFilename = Filename.substr(pos + 1);
		auto pos_dot = onlyFilename.find_last_of('.');
		if (pos_dot == std::string::npos) return true;

		if (onlyFilename.length() < 5) return true;

		auto name = onlyFilename.substr(0, pos_dot);
		auto extension = onlyFilename.substr(pos_dot + 1);

		if (extension != "wav") return true;

		ret << YAML::BeginMap;
		ret << YAML::Key << "filename" << YAML::Value << name;
		ret << YAML::EndMap;
		return true;
	}, "assets/sounds");

	ret << YAML::EndSeq;
	return ret.c_str();
}



Error Application::Ajax_PlaySoundFile(const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	auto filename = HttpServer::DecodeURLEncoded(Request.m_Query, "filename");
	int audio_device = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "sound_device"));

	auto guard = LockReadForScope();

	auto mat = FindMat(id);
	if (!mat)
		return Error::Type::MatNotFound;

	auto temp_enabled  = mat->IsSoundEnabled();
	auto temp_filename = mat->GetSoundFilename();
	auto temp_device   = mat->GetAudioDeviceID();

	mat->SetAudio(true, filename, audio_device);

	mat->QueueSoundFile();

	ZED::Core::Pause(100);

	mat->SetAudio(temp_enabled, temp_filename, temp_device);

	return Error::Type::NoError;
}



Error Application::Ajax_StartMatch(const HttpServer::Request& Request)
{
	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	int matID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (matID <= 0)
		return Error::Type::InvalidID;

	auto app_guard = LockReadForScope();

	auto mat = FindMat(matID);

	if (!mat)
		return Error::Type::MatNotFound;

	auto tournament_guard = GetTournament()->LockReadForScope();

	auto match = GetTournament()->GetNextMatch(mat->GetMatID());
	if (!match || !mat->StartMatch(match))
		return Error::Type::OperationFailed;

	return Error::Type::NoError;//OK
}



Error Application::Ajax_EndMatch(const HttpServer::Request& Request)
{
	int matID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (matID <= 0)
		return Error::Type::InvalidID;

	auto guard = LockReadForScope();

	auto mat = FindMat(matID);

	if (!mat)
		return Error::Type::MatNotFound;

	if (!mat->EndMatch())
		return Error::Type::OperationFailed;

	return Error::Type::NoError;//OK
}



Error Application::Ajax_StopMatch(const HttpServer::Request& Request)
{
	int matID = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (matID <= 0)
		return Error::Type::InvalidID;

	auto guard = LockReadForScope();

	auto mat = FindMat(matID);

	if (!mat)
		return Error::Type::MatNotFound;

	if (!mat->StopMatch())
		return Error::Type::OperationFailed;

	return Error::Type::NoError;//OK
}



std::string Application::Ajax_GetSetup(bool IsAdmin)
{
	YAML::Emitter ret;

	ret << YAML::BeginMap;

	auto guard = LockReadForScope();

	ret << YAML::Key << "version"        << YAML::Value << Version;
	ret << YAML::Key << "uptime"         << YAML::Value << (Timer::GetTimestamp() - m_StartupTimestamp);
	ret << YAML::Key << "language"       << YAML::Value << (int)Localizer::GetLanguage();
	ret << YAML::Key << "mat_count"      << YAML::Value << GetDatabase().GetMatCount();
	ret << YAML::Key << "port"           << YAML::Value << GetDatabase().GetServerPort();
	ret << YAML::Key << "ippon_style"    << YAML::Value << (int)GetDatabase().GetIpponStyle();
	ret << YAML::Key << "osaekomi_style" << YAML::Value << (int)GetDatabase().GetOsaekomiStyle();
	ret << YAML::Key << "timer_style"    << YAML::Value << (int)GetDatabase().GetTimerStyle();
	ret << YAML::Key << "name_style"     << YAML::Value << (int)GetDatabase().GetNameStyle();

	if (IsAdmin)
	{
		ret << YAML::Key << "results_server"     << YAML::Value << GetDatabase().IsResultsServer();
		ret << YAML::Key << "results_server_url" << YAML::Value << GetDatabase().GetResultsServer();
		ret << YAML::Key << "http_workers_free"  << YAML::Value << std::to_string(m_Server.GetFreeWorkerCount()) + "/" + std::to_string(m_Server.GetWorkerCount());

		auto state = License::GetLicenseState();
		ret << YAML::Key << "license_valid"   << YAML::Value << (state == License::State::Valid);
		ret << YAML::Key << "license_expired" << YAML::Value << (state == License::State::Expired);
		ret << YAML::Key << "license_expiration_date" << YAML::Value << License::GetLicenseExpiration();
		ret << YAML::Key << "license_type" << YAML::Value << (int)License::GetLicenseType();
		ret << YAML::Key << "license_id"   << YAML::Value << License::GetUserID();
	}

	ret << YAML::EndMap;
	return ret.c_str();
}



Error Application::Ajax_SetSetup(const HttpServer::Request& Request)
{
	int language      = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "language"));
	int mat_count     = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "mat_count"));
	int port          = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "port"));
	int ipponStyle    = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "ipponStyle"));
	int osaekomiStyle = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "osaekomiStyle"));
	int timerStyle    = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "timerStyle"));
	int nameStyle     = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "nameStyle"));
	bool results_server     = HttpServer::DecodeURLEncoded(Request.m_Body, "results_server") == "true";
	auto results_server_url = HttpServer::DecodeURLEncoded(Request.m_Body, "results_server_url");

	Localizer::SetLanguage((Language)language);
	
	auto guard = LockWriteForScope();

	if (mat_count >= 0)
		GetDatabase().SetMatCount(mat_count);
	if (port > 0)
		GetDatabase().SetServerPort(port);
	GetDatabase().SetIpponStyle((Mat::IpponStyle)ipponStyle);
	GetDatabase().SetOsaekomiStyle((Mat::OsaekomiStyle)osaekomiStyle);
	GetDatabase().SetTimerStyle((Mat::TimerStyle)timerStyle);
	GetDatabase().SetNameStyle((NameStyle)nameStyle);
	GetDatabase().SetNameStyle((NameStyle)nameStyle);
	GetDatabase().IsResultsServer(results_server);
	GetDatabase().SetResultsServer(results_server_url);
  GetDatabase().Save();

	return Error::Type::NoError;
}



std::string Application::Ajax_Execute(const HttpServer::Request& Request)
{
	auto command = HttpServer::DecodeURLEncoded(Request.m_Query, "cmd");

#ifdef _WIN32
	FILE* pipe = _popen(command.c_str(), "r");
#else
	FILE* pipe = popen(command.c_str(), "r");
#endif
	if (!pipe)
		return "";

	std::string result;
	char buffer[128];
	while (fgets(buffer, sizeof buffer, pipe) != NULL)
	{
		result += buffer;
	}

#ifdef _WIN32
	_pclose(pipe);
#else
	pclose(pipe);
#endif
	return result;
}



Error Application::Ajax_AddDisqualification(Fighter Whom, const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (id <= 0)
		return Error::Type::InvalidID;

	auto guard = LockReadForScope();

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

	auto guard = LockReadForScope();

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

	auto guard = LockReadForScope();

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

	auto guard = LockReadForScope();

	auto mat = FindMat(id);

	if (!mat)
		return Error::Type::MatNotFound;

	mat->RemoveNoDisqualification(Whom);
	return Error();//OK
}



Error Application::Ajax_MoveMatchUp(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	int mat = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "mat"));

	if (mat <= -1)
		mat = 0;

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error::Type::TournamentNotOpen;

	if (!GetTournament()->MoveMatchUp(id, mat))
		return Error::Type::OperationFailed;

	return Error::Type::NoError;
}



Error Application::Ajax_MoveMatchDown(const HttpServer::Request& Request)
{
	UUID id = HttpServer::DecodeURLEncoded(Request.m_Query, "id");
	int mat = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "mat"));

	if (mat <= -1)
		mat = 0;

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error::Type::TournamentNotOpen;

	if (!GetTournament()->MoveMatchDown(id, mat))
		return Error::Type::OperationFailed;

	return Error::Type::NoError;
}



Error Application::Ajax_MoveMatchTo(const HttpServer::Request& Request)
{
	UUID from = HttpServer::DecodeURLEncoded(Request.m_Query, "from");
	UUID to = HttpServer::DecodeURLEncoded(Request.m_Query, "to");
	auto position = HttpServer::DecodeURLEncoded(Request.m_Query, "position");

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error::Type::TournamentNotOpen;

	if (!GetTournament()->MoveMatchTo(from, to, position == "above"))
		return Error::Type::OperationFailed;

	return Error::Type::NoError;
}



std::string Application::Ajax_GetHansokumake() const
{
	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	for (auto mat : GetMats())
	{
		if (!mat)
			continue;

		const auto match = mat->GetMatch();
		if (!match)
			continue;


		for (Fighter fighter = Fighter::White; fighter <= Fighter::Blue; ++fighter)
		{
			if (mat->GetScoreboard(fighter).m_HansokuMake && mat->GetScoreboard(fighter).m_HansokuMake_Direct)
			{
				ret << YAML::BeginMap;

				ret << YAML::Key << "match" << YAML::Value;
				match->ToString(ret);
				ret << YAML::Key << "fighter" << YAML::Value << (int)fighter;
				ret << YAML::Key << "disqualification_state" << YAML::Value << (int)mat->GetScoreboard(fighter).m_Disqualification;

				ret << YAML::EndMap;
			}
		}
	}

	ret << YAML::EndSeq;
	return ret.c_str();
}



std::string Application::Ajax_GetNamesOnMat(const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));

	if (id <= 0)
		return Error(Error::Type::InvalidID);

	auto guard = LockReadForScope();

	if (!GetTournament())
		return Error(Error::Type::TournamentNotOpen);

	auto next_matches = GetTournament()->GetNextMatches(id);

	auto mat = FindMat(id);

	if (!mat)
		return Error(Error::Type::MatNotFound);

	YAML::Emitter ret;
	ret << YAML::BeginMap;

	ret << YAML::Key << "mat_name" << YAML::Value << mat->GetName();

	auto current_match = mat->GetMatch();

	ret << YAML::Key << "white_name" << YAML::Value;
	if (current_match && current_match->GetFighter(Fighter::White))
		ret << current_match->GetFighter(Fighter::White)->GetName(NameStyle::GivenName);
	else
		ret << "- - -";

	ret << YAML::Key << "blue_name" << YAML::Value;
	if (current_match && current_match->GetFighter(Fighter::Blue))
		ret << current_match->GetFighter(Fighter::Blue)->GetName(NameStyle::GivenName);
	else
		ret << "- - -";

	ret << YAML::Key << "match_table_name" << YAML::Value;
	if (current_match && current_match->GetMatchTable())
		ret << current_match->GetMatchTable()->GetDescription();
	else
		ret << "- - -";

	ret << YAML::Key << "next_matches" << YAML::Value << YAML::BeginSeq;

	for (const auto match : next_matches)
	{
		ret << YAML::BeginMap;

		ret << YAML::Key << "uuid" << YAML::Value << (std::string)match->GetUUID();
		ret << YAML::Key << "current_breaktime" << YAML::Value << match->GetCurrentBreaktime();
		ret << YAML::Key << "breaktime"         << YAML::Value << match->GetRuleSet().GetBreakTime();

		ret << YAML::Key << "white_name" << YAML::Value;
		if (match->GetFighter(Fighter::White))
			ret << match->GetFighter(Fighter::White)->GetName(NameStyle::GivenName);
		else
			ret << "- - -";

		ret << YAML::Key << "blue_name" << YAML::Value;
		if (match->GetFighter(Fighter::Blue))
			ret << match->GetFighter(Fighter::Blue)->GetName(NameStyle::GivenName);
		else
			ret << "- - -";

		if (match->GetMatchTable())
			ret << YAML::Key << "match_table_desc" << YAML::Value << match->GetMatchTable()->GetDescription();

		ret << YAML::EndMap;
	}

	ret << YAML::EndSeq;
	ret << YAML::EndMap;
	return ret.c_str();
}