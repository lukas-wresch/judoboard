#include <iostream>
#include <csignal>
#include "app.h"
#include "database.h"
#include "tournament.h"
#include "weightclass.h"
#include "round_robin.h"
#include "standing_data.h"
#include "../ZED/include/log.h"
#include "../ZED/include/http_client.h"
#include "../ZED/include/http_server.h"
#include "../ZED/include/socket_tcp.h"
#include "../ZED/include/socket_ssl.h"



Judoboard::Application* g_app = nullptr;//Used for signal handling



Judoboard::Judoka CreateRandomJudoka(const Judoboard::StandingData* db)
{
	const std::string firstname_male[] =
	{ "Ben", "Friedrich", "Phillipp", "Tim", "Lukas", "Marco", "Peter", "Martin", "Detlef", "Andreas", "Dominik", "Mathias", "Stephan", u8"S\u00f6ren", "Eric", "Finn", "Felix", "Julian", "Maximilian", "Jannik"};
	const std::string firstname_female[] =
	{ "Emma", "Stephanie", "Julia", "Jana", "Uta", "Petra", "Sophie", "Kerstin", "Lena", "Jennifer", "Kathrin", "Katherina", "Anna", "Carla", "Paulina", "Clara", "Hanna" };
	const std::string lastname[] =
	{ "Ehrlichmann", "Dresdner", "Biermann", "Fisher", "Vogler", "Pfaff", "Eberhart", "Frankfurter", u8"K\u00f6nig", "Pabst", "Ziegler", "Hartmann", "Pabst", "Kortig", "Schweitzer", "Luft", "Wexler", "Kaufmann", u8"Fr\u00fchauf", "Bieber", "Schumacher", u8"M\u00fcncher", "Schmidt", "Meier", "Fischer", "Weber", "Meyer", "Wagner", "Becker", "Schulz", "Hoffmann" };

	Judoboard::Judoka ret("", "");

	if (rand() & 1)
	{
		auto fname = firstname_male[rand() % (sizeof(firstname_male) / sizeof(firstname_male[0]) - 1)];
		auto lname = lastname[rand() % (sizeof(lastname) / sizeof(std::string) - 1)];
		ret = Judoboard::Judoka(fname, lname, 25 + rand() % 60, Judoboard::Gender::Male);
	}
	else
	{
		auto fname = firstname_female[rand() % (sizeof(firstname_female) / sizeof(firstname_female[0]) - 1)];
		auto lname = lastname[rand() % (sizeof(lastname) / sizeof(std::string) - 1)];
		ret = Judoboard::Judoka(fname, lname, 25 + rand() % 60, Judoboard::Gender::Female);
	}

	ret.SetBirthyear(1990 + rand()%25);

	if (db && db->GetAllClubs().size() >= 1)
	{
		int club_index = rand() % db->GetAllClubs().size();
		auto club = db->GetAllClubs()[club_index];
		ret.SetClub(club);
	}

	return ret;
}



#ifdef _WIN32
int APIENTRY WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hprevinstance,
	_In_ LPSTR lpcmdline,
	_In_ int nCmdShow)
#else
int main(int argc, char** argv)
#endif
{
	ZED::Log::Open("log.txt");

	//Log start time
	auto date = ZED::Core::GetDate();
	char buffer[128];
	snprintf(buffer, sizeof(buffer), "Application start at %u.%u.%u at %u:%u:%u",
		date.day, date.month, date.year, date.hour, date.minute, date.second);
	ZED::Log::Info(buffer);

	int port = 0;
	bool show_test_screen = false;
	bool demo = false;
	bool nowindow = false;
	bool version  = false;
	bool slave    = false;
	std::string master_ip;
	int master_port = 8080;

	std::vector<std::string> commands;

#ifdef _WIN32
	std::string cmd(lpcmdline);

	//Decompose statement
	size_t pos = 0;
	for (size_t initialPos = 0; pos != std::string::npos; initialPos = pos + 1)
	{
		pos = cmd.find(' ', initialPos);
		if (pos != std::string::npos)
			commands.emplace_back(cmd.substr(initialPos, pos - initialPos));
		else//Add the last one
			commands.emplace_back(cmd.substr(initialPos, std::min(pos, cmd.size()) - initialPos + 1));
	}
#else
	for (int i = 1; i < argc; i++)
		commands.push_back(std::string(argv[i]));
#endif

	for (auto it = commands.begin(); it != commands.end(); ++it)
	{
		if (*it == "--port" && ++it != commands.end())
			port = ZED::Core::ToInt(*it);
		else if (*it == "--slave" && ++it != commands.end())
		{
			slave = true;
			master_ip = *it;
			if (it+1 != commands.end())
				master_port = ZED::Core::ToInt(*++it);
		}
		else if (*it == "--testscreen")
			show_test_screen = true;
		else if (*it == "--demo")
			demo = true;
		else if (*it == "--nowindow")
			nowindow = true;
		else if (*it == "--version")
			version = true;
		else if (*it == "--sign")
		{
			auto user_id = *(++it);
			int  type    = ZED::Core::ToInt(*(++it));
			auto expires = *(++it);
			auto name    = *(++it);

			Judoboard::License::Sign(user_id.c_str(), (Judoboard::License::Type)type, expires.c_str(), name.c_str());
			return 0;
		}

#ifdef _DEBUG
		else if (*it == "--dummy")//CREATE DUMMY TOURNAMENT
		{
			Judoboard::Tournament tournament1("dummy-mustermann1");
			for (int i = 0; i < 150; i++)
			{
				Judoboard::Judoka* white = new Judoboard::Judoka("Max" + std::to_string(2 * i + 1), "Mustermann");
				Judoboard::Judoka* blue  = new Judoboard::Judoka("Max" + std::to_string(2 * i + 2), "Mustermann");
				Judoboard::Match* match  = new Judoboard::Match(white, blue, &tournament1, (i % 2) + 1);
				tournament1.AddMatch(match);
			}
			for (auto table : tournament1.GetMatchTables())
				table->SetName(" ");


			Judoboard::Tournament tournament2("dummy-mustermann2");
			for (int i = 0; i < 150; i++)
			{
				Judoboard::Judoka* white = new Judoboard::Judoka("Max" + std::to_string(2 * i + 1), "Mustermann");
				Judoboard::Judoka* blue  = new Judoboard::Judoka("Max" + std::to_string(2 * i + 2), "Mustermann");
				Judoboard::Match* match  = new Judoboard::Match(white, blue, &tournament2, (i % 2) + 1);
				tournament2.AddMatch(match);
			}
			for (auto table : tournament2.GetMatchTables())
				table->SetName(" ");
			
			return 0;
		}//END OF CREATE DUMMY TOURNAMENT
#endif
	}



	if (version)
	{
#ifdef _WIN32
		if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole())
		{
			FILE* temp;
			freopen_s(&temp, "CONOUT$", "w", stdout);
			freopen_s(&temp, "CONOUT$", "w", stderr);
		}
#endif
		std::cout << Judoboard::Application::Version;

#ifdef _WIN32
		FreeConsole();
#endif
		return 0;
	}

	if (nowindow)
		Judoboard::Application::NoWindow = true;

	if (show_test_screen)
	{
		ZED::Log::Info("Initializing application");
		Judoboard::Application app;

		ZED::Log::Info("Showing test screen");
		app.StartLocalMat(1);

		Judoboard::Mat* mat = (Judoboard::Mat*)app.GetLocalMat();

		mat->SetFullscreen();
		ZED::Core::Pause(5000);

		app.GetDatabase().AddClub(new Judoboard::Club("Altenhagen"));
		app.GetDatabase().AddClub(new Judoboard::Club("Brackwede"));
		app.GetDatabase().AddClub(new Judoboard::Club("Senne"));

		srand(ZED::Core::CurrentTimestamp());
		auto j1 = CreateRandomJudoka(&app.GetDatabase());
		auto j2 = CreateRandomJudoka(&app.GetDatabase());
		Judoboard::Match match(&j1, &j2, nullptr, mat->GetMatID());
		Judoboard::RuleSet rules("ScreenTest", 1, 3*60, 20, 10, false, false);
		Judoboard::AgeGroup age_group("U18", 15, 18, &rules);
		match.SetRuleSet(&rules);
		Judoboard::RoundRobin* table = new Judoboard::RoundRobin(new Judoboard::Weightclass(10, 100));
		table->SetAgeGroup(&age_group);
		match.SetMatchTable(table);

		mat->StartMatch(&match);
		ZED::Core::Pause(500);

		mat->Hajime();

		for (int i = 0; i < 15; i++)
		{
			mat->AddYuko(Judoboard::Fighter::White);
			mat->AddKoka(Judoboard::Fighter::Blue);
		}

		for (int i = 0; i < 2; i++)
		{
			mat->AddShido(Judoboard::Fighter::White);
			ZED::Core::Pause(1000);
			mat->AddShido(Judoboard::Fighter::Blue);
			ZED::Core::Pause(1000);
			mat->AddMedicalExamination(Judoboard::Fighter::White);
			ZED::Core::Pause(1000);
			mat->AddMedicalExamination(Judoboard::Fighter::Blue);
			ZED::Core::Pause(1000);
		}

		mat->AddWazaAri(Judoboard::Fighter::White);
		mat->AddWazaAri(Judoboard::Fighter::Blue);

		mat->EnableGoldenScore();
		mat->Hajime();

		mat->Osaekomi(Judoboard::Fighter::White);
		ZED::Core::Pause(13000);
		mat->Sonomama();

		mat->AddShido(Judoboard::Fighter::White);
		mat->AddShido(Judoboard::Fighter::Blue);

		ZED::Core::Pause(2*60*1000);

		return 0;
	}

	else if (demo)
	{
		ZED::Log::Info("Initializing application");
		Judoboard::Application app;

		ZED::Log::Info("Starting demo mode");
		app.StartLocalMat(1);

		Judoboard::Mat* mat = (Judoboard::Mat*)app.GetLocalMat();

		mat->GetWindow().Fullscreen();
		ZED::Core::Pause(5000);

		srand(ZED::Core::CurrentTimestamp());

		auto tourney = new Judoboard::Tournament("Demo Tournament");
		tourney->EnableAutoSave(false);

		auto rule_set  = Judoboard::RuleSet("Demo", 180, 60, 20, 10);
		auto age_group = Judoboard::AgeGroup("U18", 0, 100, &rule_set);

		auto m1 = new Judoboard::RoundRobin(new Judoboard::Weightclass(0, 120));
		m1->SetMatID(1);
		m1->SetAgeGroup(&age_group);
		tourney->AddMatchTable(m1);

		app.GetDatabase().AddClub(new Judoboard::Club("Altenhagen"));
		app.GetDatabase().AddClub(new Judoboard::Club("Brackwede"));
		app.GetDatabase().AddClub(new Judoboard::Club("Senne"));

		for (int i = 0; i < 5; i++)
			tourney->AddParticipant(new Judoboard::Judoka(CreateRandomJudoka(&app.GetDatabase())));

		app.AddTournament(tourney);

		ZED::Core::Pause(7000);


		while (true)
		{
			ZED::Core::Pause(11*1000);
			auto match = tourney->GetNextMatch(mat->GetMatID());
			if (!match)
				break;

			mat->StartMatch(match);

			ZED::Core::Pause(9000);

			mat->Hajime();

			ZED::Core::Pause(5000 + rand()%1500);

			while (!mat->HasConcluded())
			{
				Judoboard::Fighter f = Judoboard::Fighter::White;
				if (rand() % 2 == 0)
					f = Judoboard::Fighter::Blue;

				int rnd = rand() % 100;

				if (rnd <= 5)
					mat->AddIppon(f);
				else if (rnd <= 10)
				{
					mat->Mate();
					ZED::Core::Pause(3000);
					mat->AddHansokuMake(f);
					mat->AddNoDisqualification(f);
					ZED::Core::Pause(3000);
				}
				else if (rnd <= 30)
					mat->AddWazaAri(f);
				else if (rnd <= 60)
				{
					mat->Mate();
					ZED::Core::Pause(2000);
					mat->AddShido(f);
					ZED::Core::Pause(3000);
					mat->Hajime();
				}
				else if (rnd <= 75)
				{
					mat->Mate();
					ZED::Core::Pause(1000);
					mat->AddMedicalExamination(f);
					ZED::Core::Pause(4000);
					mat->Hajime();
				}
				else
				{
					mat->Osaekomi(f);
					ZED::Core::Pause(6 * 1000);

					if (rand()%2 == 0)
						mat->Tokeda();

					else if (rand()%2 == 0)
					{
						ZED::Core::Pause(2000);
						mat->Sonomama();
						ZED::Core::Pause(6000);
						mat->Hajime();//Yoshi
					}

					ZED::Core::Pause(6 * 1000);
				}

				ZED::Core::Pause(10 * 1000);
			}

			ZED::Core::Pause(2000);

			mat->EndMatch();
			ZED::Core::Pause(15*1000);
		}

		return 0;
	}

	ZED::HttpServer server(1);
	server.AddListeningPort(new ZED::SocketTCP, 8081);
	server.AddListeningPort(new ZED::SocketSSL, 8082);
	server.Start();
	

	ZED::Log::Info("Initializing application");
	Judoboard::Application app;

	if (!app.LoadDataFromDisk())
	{
		ZED::Log::Error("Could not load application data from disk");
		if (port <= 0)//Port not set via command line
			port = 8080;
	}
	else if (port <= 0)//Port not set via command line
		port = app.GetDatabase().GetServerPort();

	app.StartHttpServer(port);


	Judoboard::License::Check(&app);

	bool do_update = false;
	switch (Judoboard::License::GetLicenseState())
	{
	case Judoboard::License::State::FileNotExist:
		do_update = true;
#ifdef _WIN32
		MessageBox(NULL, L"Demo version! The program will close after 30 minutes.", L"Judoboard", MB_OK | MB_ICONINFORMATION);
#endif
		break;
	case Judoboard::License::State::Expired:
		do_update = true;
#ifdef _WIN32
		MessageBox(NULL, L"License expired!", L"Judoboard", MB_OK | MB_ICONERROR);
#endif
		break;
	case Judoboard::License::State::Valid:
		//do_update = true;
		//MessageBox(NULL, L"License valid!", L"Judoboard", MB_OK | MB_ICONINFORMATION);
		break;
	}

	if (do_update)
	{
		std::thread([&app]() {
			char url[512];
#ifdef _WIN32
			char computer_name[MAX_COMPUTERNAME_LENGTH + 1] = {};
			DWORD size = MAX_COMPUTERNAME_LENGTH;
			GetComputerNameA(computer_name, &size);
#else
			char computer_name[] = "Linux";
#endif
			snprintf(url, sizeof(url), "/judoboard/license.php?id=%s&name=%s", Judoboard::License::GetUserID(), computer_name);

			ZED::HttpClient client("wresch.spdns.eu");

			auto packet = client.GET(url);
			if (packet.body.length() > 64 && packet.body.length() < 256)
			{
				FILE* file = nullptr;
				fopen_s(&file, "license.key", "wb");
				if (file)
				{
					fwrite(packet.body.c_str(), sizeof(char), packet.body.length(), file);
					fclose(file);

					if (Judoboard::License::GetLicenseState() != Judoboard::License::State::Valid)
						Judoboard::License::Check(&app);
				}
			}
		}).detach();
	}


	if (slave)
	{
		if (!app.ConnectToMaster(master_ip, master_port))
		{
			ZED::Log::Error("Could not connect to master " + master_ip + " " + std::to_string(master_port));
			return -1;
		}

		ZED::Log::Info("Connected to master");

		auto monitors = Judoboard::Window::EnumerateMonitors();

		for (uint32_t i = 1; i <= app.GetDatabase().GetMatCount(); i++)
		{
			auto mat = app.StartLocalMat(i);
			if (monitors.size() > 0)
				mat->SetFullscreen(mat->IsFullscreen(), i%monitors.size());
		}
	}
	else
	{
		auto monitors = Judoboard::Window::EnumerateMonitors();

		for (uint32_t i = 1; i <= app.GetDatabase().GetMatCount(); i++)
		{
			auto mat = app.StartLocalMat(i);
			if (monitors.size() > 0)
				mat->SetFullscreen(mat->IsFullscreen(), i%monitors.size());
		}

		if (app.GetDatabase().GetNumAccounts() == 0)
			app.GetDatabase().AddAccount(Judoboard::Account("admin", "1234", Judoboard::Account::AccessLevel::Admin));
	}

#ifdef _DEBUG
	if (app.GetDatabase().GetNumJudoka() < 5)
	{
		auto inter = new Judoboard::Association("International", nullptr);

		auto de = new Judoboard::Association("Deutschland", inter);

		auto dn = new Judoboard::Association("Deutschland-Nord", de);
		auto ds = new Judoboard::Association(u8"Deutschland-S\u00fcd", de);

		auto nord  = new Judoboard::Association("Nord", dn);
		auto west  = new Judoboard::Association("West", dn);
		auto nost  = new Judoboard::Association("Nordost", dn);
		auto sued  = new Judoboard::Association(u8"S\u00fcd", ds);
		auto swest = new Judoboard::Association(u8"S\u00fcdwest", ds);

		auto nieder  = new Judoboard::Association("Niedersachsen", nord);
		auto hamburg = new Judoboard::Association("Hamburg", nord);
		auto berlin  = new Judoboard::Association("Berlin", nost);
		auto nrw     = new Judoboard::Association("Nordrhein-Westfalen", west);

		app.GetDatabase().AddAssociation(nieder);
		app.GetDatabase().AddAssociation(hamburg);
		app.GetDatabase().AddAssociation(berlin);
		app.GetDatabase().AddAssociation(nrw);

		auto detmold = new Judoboard::Association("Detmold", nrw);

		auto biegue = new Judoboard::Association(u8"Bielefeld/G\u00fctersloh", detmold);


		app.GetDatabase().AddClub(new Judoboard::Club("Altenhagen", biegue));
		app.GetDatabase().AddClub(new Judoboard::Club("Brackwede", biegue));
		app.GetDatabase().AddClub(new Judoboard::Club("Senne", biegue));

		ZED::Log::Debug("Adding debug judoka");

		for (int i = 0; i < 50; i++)
			app.GetDatabase().AddJudoka(CreateRandomJudoka(&app.GetDatabase()));

		app.GetDatabase().Save();
	}
#endif

	ZED::Core::Pause(2000);

#ifdef WIN32
#ifndef _DEBUG
	if (!slave)
		ShellExecute(NULL, L"open", L"http://localhost:8080", NULL, NULL, 0);
#endif
#endif

	//Register single handler
	g_app = &app;
	signal(SIGINT, [](int signum) {
		if (g_app)
			g_app->Shutdown();
	});
	signal(SIGTERM, [](int signum) {
		if (g_app)
			g_app->Shutdown();
	});

	ZED::Log::Info("Application has started");
	app.Run();

	ZED::Log::Info("Application closed");

	return 0;
}