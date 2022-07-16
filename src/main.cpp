#include <iostream>
#include "app.h"
#include "database.h"
#include "tournament.h"
#include "weightclass.h"
#include "standing_data.h"
#include "../ZED/include/log.h"



Judoboard::Judoka CreateRandomJudoka(const Judoboard::StandingData* db)
{
	const std::string firstname_male[] =
	{ "Ben", "Friedrich", "Phillipp", "Tim", "Lukas", "Marco", "Peter", "Martin", "Detlef", "Andreas", "Dominik", "Mathias", "Stephan", u8"Sören", "Eric", "Finn", "Felix", "Julian", "Maximilian", "Jannik"};
	const std::string firstname_female[] =
	{ "Emma", "Stephanie", "Julia", "Jana", "Uta", "Petra", "Sophie", "Kerstin", "Lena", "Jennifer", "Kathrin", "Katherina", "Anna", "Carla", "Paulina", "Clara", "Hanna" };
	const std::string lastname[] =
	{ "Ehrlichmann", "Dresdner", "Biermann", "Fisher", "Vogler", "Pfaff", "Eberhart", "Frankfurter", u8"König", "Pabst", "Ziegler", "Hartmann", "Pabst", "Kortig", "Schweitzer", "Luft", "Wexler", "Kaufmann", u8"Frühauf", "Bieber", "Schumacher", u8"Müncher", "Schmidt", "Meier", "Fischer", "Weber", "Meyer", "Wagner", "Becker", "Schulz", "Hoffmann" };

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

	int port = 8080;
	bool show_test_screen = false;
	bool demo = false;
	bool nowindow = false;
	bool version  = false;

#ifdef _WIN32
	std::string cmd(lpcmdline);
	if (cmd.find("--testscreen") != std::string::npos)
		show_test_screen = true;
	else if (cmd.find("--demo") != std::string::npos)
		demo = true;
	else if (cmd.find("--nowindow") != std::string::npos)
		nowindow = true;
	else if (cmd.find("--version") != std::string::npos)
		version = true;
#else
	for (int i = 1; i < argc; i++)
	{
		if (std::string(argv[i]) == "--port" && i+1 < argc)
			port = ZED::Core::ToInt(argv[i+1]);
		else if (std::string(argv[i]) == "--testscreen")
			show_test_screen = true;
		else if (std::string(argv[i]) == "--demo")
			demo = true;
		else if (std::string(argv[i]) == "--nowindow")
			nowindow = true;
		else if (std::string(argv[i]) == "--version")
			version = true;
	}
#endif

	//TODO load config file
	Judoboard::Localizer::SetLanguage(Judoboard::Language::German);

	if (version)
	{
		std::cout << Judoboard::Application::Version;
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

		Judoboard::Mat* mat = (Judoboard::Mat*)app.GetDefaultMat();

		mat->SetFullscreen();
		ZED::Core::Pause(5000);

		app.GetDatabase().AddClub(new Judoboard::Club("Altenhagen"));
		app.GetDatabase().AddClub(new Judoboard::Club("Brackwede"));
		app.GetDatabase().AddClub(new Judoboard::Club("Senne"));

		srand(ZED::Core::CurrentTimestamp());
		auto j1 = CreateRandomJudoka(&app.GetDatabase());
		auto j2 = CreateRandomJudoka(&app.GetDatabase());
		Judoboard::Match match(nullptr, &j1, &j2, mat->GetMatID());
		//Judoboard::RuleSet rules("ScreenTest", 1, 3*60, 20, 10, true, true);
		Judoboard::RuleSet rules("ScreenTest", 1, 3*60, 20, 10, false, false);
		Judoboard::AgeGroup age_group("U18", 15, 18, &rules, app.GetDatabase());
		match.SetRuleSet(&rules);
		Judoboard::Weightclass* table = new Judoboard::Weightclass(nullptr, 10, 100);
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
		ZED::Core::Pause(8000);
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

		Judoboard::Mat* mat = (Judoboard::Mat*)app.GetDefaultMat();

#ifndef _DEBUG
		mat->GetWindow().Fullscreen();
#endif
		ZED::Core::Pause(5000);

		srand(ZED::Core::CurrentTimestamp());

		auto tourney = new Judoboard::Tournament("Demo Tournament");
		tourney->EnableAutoSave(false);

		auto m1 = new Judoboard::Weightclass(tourney, 0, 120);
		tourney->AddMatchTable(m1);
		m1->SetMatID(1);

		for (int i = 0; i < 5; i++)
		{
			//app.GetDatabase().AddJudoka(CreateRandomJudoka());
			tourney->AddParticipant(new Judoboard::Judoka(CreateRandomJudoka(&app.GetDatabase())));
		}

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

			ZED::Core::Pause(6000);

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

	
	ZED::Log::Info("Initializing application");
	Judoboard::Application app(port);

	app.StartLocalMat(1);


	if (!app.LoadDataFromDisk())
		ZED::Log::Error("Could not load application data from disk");

	if (app.GetDatabase().GetNumAccounts() == 0)
		app.GetDatabase().AddAccount(Judoboard::Account("admin", "1234", Judoboard::Account::AccessLevel::Admin));

	//if (app.GetTournamentList().size() == 0)
		//app.AddTournament(new Judoboard::Tournament("Test Tournament", app.GetDatabase().FindRuleSet("Default")));

	ZED::Core::Pause(3000);

#ifdef WIN32
#ifndef _DEBUG
	ShellExecute(NULL, L"open", L"http://localhost:8080", NULL, NULL, 0);
#endif
#endif

#ifdef _DEBUG
	if (app.GetDatabase().GetNumJudoka() < 5)
	{
		app.GetDatabase().AddClub(new Judoboard::Club("Altenhagen"));
		app.GetDatabase().AddClub(new Judoboard::Club("Brackwede"));
		app.GetDatabase().AddClub(new Judoboard::Club("Senne"));

		ZED::Log::Debug("Adding debug judoka");

		for (int i = 0; i < 35; i++)
			app.GetDatabase().AddJudoka(CreateRandomJudoka(&app.GetDatabase()));
	}
#endif

	ZED::Log::Info("Application has started");
	app.Run();

	ZED::Log::Info("Application closed");

	return 0;
}