#include "tests.h"



TEST(App, ServerConnection)
{
	initialize();
	Application app(8080 + rand() % 10000);

	ZED::HttpClient client("127.0.0.1", app.GetPort());
	ASSERT_TRUE(client.IsConnected());

	EXPECT_TRUE(client.SendGETRequest("/test"));

	auto packet = client.RecvResponse();
	EXPECT_GE(packet.header.length(), 10u);
	EXPECT_GE(packet.body.length(), 10u);

	EXPECT_TRUE(app.IsRunning());
	EXPECT_TRUE(app.IsMaster());
	EXPECT_FALSE(app.IsSlave());

	EXPECT_TRUE(app.GetTournament());
	EXPECT_EQ(app.GetTournamentList().size(), 0);
}



/*TEST(App, MemoryLeak)
{
	initialize();
	for (int i = 0; i < 1000; i++)
	{
		Application app;
		app.StartLocalMat();
	}
}*/



TEST(App, AddDM4File)
{
	initialize();
	Application app;

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	app.AddTournament(t);

	DM4 dm4_file("test-data/Verein1_U13 (m).dm4");

	ASSERT_TRUE(dm4_file);

	bool success;
	auto output = app.AddDM4File(dm4_file, false, &success);

	ASSERT_TRUE(success);

	auto judokas = app.GetTournament()->GetParticipants();

	ASSERT_EQ(judokas.size(), 1);

	for (auto j : judokas)
	{
		EXPECT_EQ(j->GetFirstname(), "VornameTN");
		EXPECT_EQ(j->GetLastname(),  "NachnameTN");
		EXPECT_EQ(j->GetBirthyear(), 2007);
		ASSERT_TRUE(j->GetClub());
		EXPECT_EQ(j->GetClub()->GetName(), "Verein1");
	}

	EXPECT_EQ(app.GetDatabase().GetNumJudoka(), 0);
	EXPECT_EQ(app.GetDatabase().GetAllClubs().size(), 0);
}



TEST(App, AddDMFFile)
{
	initialize();
	Application app;

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	app.AddTournament(t);

	DMF dmf_file("test-data/firstage group1 (m).dmf");

	ASSERT_TRUE(dmf_file);

	bool success;
	auto output = app.AddDMFFile(dmf_file, false, &success);

	ASSERT_TRUE(success);

	auto judokas = app.GetTournament()->GetParticipants();

	ASSERT_EQ(judokas.size(), 1);

	for (auto j : judokas)
	{
		EXPECT_EQ(j->GetFirstname(), "firstname");
		EXPECT_EQ(j->GetLastname(),  "lastname");
		EXPECT_EQ(j->GetBirthyear(), 1990);
		ASSERT_TRUE(j->GetClub());
		EXPECT_EQ(j->GetClub()->GetName(), "club name");
	}

	EXPECT_EQ(app.GetDatabase().GetNumJudoka(), 0);
	EXPECT_EQ(app.GetDatabase().GetAllClubs().size(), 0);
}



TEST(App, Tournaments)
{
	initialize();
	Application app;

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	EXPECT_TRUE(app.AddTournament(t));
	EXPECT_TRUE(app.GetTournament());

	EXPECT_EQ(app.GetTournamentList().size(), 1);
	EXPECT_EQ((*app.GetTournamentList().begin())->GetName(), t->GetName());

	EXPECT_TRUE(app.OpenTournament(t->GetUUID()));
	ASSERT_TRUE(app.GetTournament());
	EXPECT_EQ(app.GetTournament()->GetName(), t->GetName());
	EXPECT_TRUE(app.CloseTournament());

	EXPECT_EQ(app.FindTournamentByName("Tournament Name")->GetName(), t->GetName());
}



TEST(App, Tournament_With_Ongoing_Match)
{
	initialize();
	Application app;

	Tournament* t = new Tournament("../test-data/tournament-ongoing-match");
	t->EnableAutoSave(false);
	app.AddTournament(t);
	app.OpenTournament(*t);

	EXPECT_TRUE(app.StartLocalMat(1));

	ZED::Core::Pause(3000);

	EXPECT_TRUE(app.GetMats()[0]->AreFightersOnMat());
}



TEST(App, Tournament_With_Ongoing_Match2)
{
	initialize();
	Application app;

	EXPECT_TRUE(app.StartLocalMat(1));

	ZED::Core::Pause(2000);

	Tournament* t = new Tournament("../test-data/tournament-ongoing-match");
	t->EnableAutoSave(false);
	app.AddTournament(t);
	app.OpenTournament(*t);

	ZED::Core::Pause(3500);

	EXPECT_TRUE(app.GetMats()[0]->AreFightersOnMat());
}



TEST(App, Tournaments_OpenLastTournament)
{
	initialize();

	{
		Application app;

		Tournament* t = new Tournament("deleteMe");
		t->AddMatch(new Match(nullptr, nullptr, t));

		EXPECT_TRUE(app.AddTournament(t));
		t->Save();
	}

	{
		Application app;

		app.LoadDataFromDisk();

		EXPECT_TRUE(app.FindTournamentByName("deleteMe"));
		ASSERT_TRUE(app.GetTournament());
		EXPECT_EQ(app.GetTournament()->GetName(), "deleteMe");

		EXPECT_TRUE(app.CloseTournament());
	}

	{
		Application app;

		app.LoadDataFromDisk();

		EXPECT_TRUE(app.FindTournamentByName("deleteMe"));
		ASSERT_TRUE(app.GetTournament());
		EXPECT_NE(app.GetTournament()->GetName(), "deleteMe");
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(App, DeleteTournament)
{
	initialize();
	Application app;

	{
		Tournament* t = new Tournament("deleteMe");

		Judoka j(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
		t->AddParticipant(&j);

		EXPECT_TRUE(app.AddTournament(t));
		ASSERT_TRUE(app.GetTournament());

		EXPECT_EQ(app.GetTournamentList().size(), 1);
		EXPECT_EQ((*app.GetTournamentList().begin())->GetName(), t->GetName());

		EXPECT_TRUE(app.CloseTournament());
		EXPECT_TRUE(app.DeleteTournament(t->GetUUID()));
	}
	

	std::ifstream file("tournaments/deleteMe.yaml");
	EXPECT_FALSE(file);
}



TEST(App, Mats)
{
	initialize();

	{
		Application app;

		EXPECT_EQ(app.GetMats().size(), 0);
		EXPECT_EQ(app.FindDefaultMatID(), 0);
		EXPECT_FALSE(app.GetDefaultMat());
		EXPECT_FALSE(app.FindMat(1));

		EXPECT_TRUE(app.StartLocalMat(1));

		EXPECT_EQ(app.GetMats().size(), 1);
		EXPECT_EQ(app.FindDefaultMatID(), 1);
		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.FindMat(1));

		EXPECT_TRUE(app.CloseMat(1));

		EXPECT_EQ(app.GetMats().size(), 1);
		EXPECT_EQ(app.FindDefaultMatID(), 1);
		EXPECT_TRUE(app.GetDefaultMat() != nullptr);
		EXPECT_TRUE(app.FindMat(1) != nullptr);
	}

	ZED::Core::Pause(5000);
}



TEST(App, Mutex)
{
	initialize();

	{
		ZED::RecursiveReadWriteMutex mutex;

		auto read_thread = [&mutex]()
		{
			for (int i = 0; i < 1000; i++)
			{
				mutex.LockRead();
				ZED::Core::Pause(1);
				mutex.UnlockRead();
				ZED::Core::Pause(1);
			}
		};

		auto write_thread = [&mutex]()
		{
			for (int i = 0; i < 1000; i++)
			{
				mutex.LockWrite();
				ZED::Core::Pause(1);
				mutex.UnlockWrite();
				ZED::Core::Pause(1);
			}
		};

		auto double_write_thread = [&mutex]()
		{
			for (int i = 0; i < 1000; i++)
			{
				mutex.LockWrite();
				ZED::Core::Pause(1);

				mutex.LockWrite();
				ZED::Core::Pause(1);
				mutex.UnlockWrite();
				ZED::Core::Pause(1);

				mutex.UnlockWrite();
				ZED::Core::Pause(1);
			}
		};

		auto upgrade_thread = [&mutex]()
		{
			for (int i = 0; i < 500; i++)
			{
				mutex.LockRead();
				ZED::Core::Pause(1);
				
				mutex.LockWrite();
				ZED::Core::Pause(1);
				mutex.UnlockWrite();
				ZED::Core::Pause(1);

				mutex.UnlockRead();
				ZED::Core::Pause(1);
			}
		};


		std::thread read_threads[20];
		std::thread write_threads[5];
		std::thread upgrade_threads[3];
		std::thread double_write_threads[3];

		for (auto& thread : read_threads)
			thread = std::thread(read_thread);
		for (auto& thread : write_threads)
			thread = std::thread(write_thread);
		for (auto& thread : upgrade_threads)
			thread = std::thread(upgrade_thread);
		for (auto& thread : double_write_threads)
			thread = std::thread(double_write_thread);

		ZED::Core::Pause(5000);

		for (auto& thread : read_threads)
			thread.join();
		for (auto& thread : write_threads)
			thread.join();
		for (auto& thread : upgrade_threads)
			thread.join();
		for (auto& thread : double_write_threads)
			thread.join();
	}
}



TEST(App, FullTournament)
{
	initialize();

	{
		Application app;

		app.CloseTournament();
		app.SetTournamentList().clear();

		app.StartLocalMat(1);

		Account acc("admin", "1234", Account::AccessLevel::Admin);
		app.GetDatabase().AddAccount(acc);

		ZED::Core::Pause(1000);

		Judoka j1(GetFakeFirstname(), GetFakeLastname(), rand() % 50);
		Judoka j2(GetFakeFirstname(), GetFakeLastname(), rand() % 50);
		Judoka j3(GetFakeFirstname(), GetFakeLastname(), rand() % 50);

		Judoka j4(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
		Judoka j5(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
		Judoka j6(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);

		app.GetDatabase().AddJudoka(std::move(j1));
		app.GetDatabase().AddJudoka(std::move(j2));
		app.GetDatabase().AddJudoka(std::move(j3));

		app.GetDatabase().AddJudoka(std::move(j4));
		app.GetDatabase().AddJudoka(std::move(j5));
		app.GetDatabase().AddJudoka(std::move(j6));

		auto tournament_name = GetRandomName();
		auto tourney = new Tournament(tournament_name, new RuleSet("Test", 3 * 60, 3 * 60, 20, 10));
		tourney->EnableAutoSave(false);
		EXPECT_TRUE(app.AddTournament(tourney));

		ASSERT_TRUE(app.FindTournamentByName(tournament_name));
		EXPECT_TRUE(app.OpenTournament(app.FindTournamentByName(tournament_name)->GetUUID()));

		EXPECT_GE(j1.GetLengthOfBreak(), 60u);
		EXPECT_GE(j2.GetLengthOfBreak(), 60u);
		EXPECT_GE(j3.GetLengthOfBreak(), 60u);
		EXPECT_GE(j4.GetLengthOfBreak(), 60u);
		EXPECT_GE(j5.GetLengthOfBreak(), 60u);
		EXPECT_GE(j6.GetLengthOfBreak(), 60u);

		tourney->AddParticipant(&j1);
		tourney->AddParticipant(&j2);
		tourney->AddParticipant(&j3);
		tourney->AddParticipant(&j4);
		tourney->AddParticipant(&j5);
		tourney->AddParticipant(&j6);

		MatchTable* m1 = new RoundRobin(Weight(0), Weight(49));
		MatchTable* m2 = new RoundRobin(Weight(0), Weight(49));
		m1->SetMatID(1);
		m2->SetMatID(1);
		tourney->AddMatchTable(m1);
		tourney->AddMatchTable(m2);

		auto mat = app.GetDefaultMat();
		ASSERT_TRUE(mat);

		for (int i = 0; i < 6; i++)
		{
			ZED::Core::Pause(6000);
			auto match = tourney->GetNextMatch(mat->GetMatID());
			ASSERT_TRUE(match);
			EXPECT_TRUE(mat->StartMatch(match));

			ZED::Core::Pause(5000);

			mat->Hajime();

			ZED::Core::Pause(6000);

			while (!mat->HasConcluded())
			{
				Fighter f = Fighter::White;
				if (rand() % 2 == 0)
					f = Fighter::Blue;

				int rnd = rand() % 6;

				if (rnd == 0)
					mat->AddIppon(f);
				else if (rnd == 1)
					mat->AddWazaAri(f);
				else if (rnd == 2)
					mat->AddWazaAri(f);
				else if (rnd == 3)
					mat->AddShido(f);
				else if (rnd == 4)
					mat->AddMedicalExamination(f);
				else
				{
					mat->Osaekomi(f);
					ZED::Core::Pause(12 * 1000);
				}

				ZED::Core::Pause(10 * 1000);
			}

			ZED::Core::Pause(3000);

			EXPECT_TRUE(mat->EndMatch());
			ZED::Core::Pause(8000);
		}
	}
}



TEST(App, FullTournament_SingleElimination14)
{
	initialize();

	{
		Application app;

		app.CloseTournament();
		app.SetTournamentList().clear();

		app.StartLocalMat(1);

		Account acc("admin", "1234", Account::AccessLevel::Admin);
		app.GetDatabase().AddAccount(acc);

		ZED::Core::Pause(5000);

		auto tournament_name = GetRandomName();
		auto tourney = new Tournament(tournament_name, new RuleSet("Test", 3 * 60, 3 * 60, 20, 10));
		tourney->EnableAutoSave(false);
		EXPECT_TRUE(app.AddTournament(tourney));

		ASSERT_TRUE(app.FindTournamentByName(tournament_name));
		EXPECT_TRUE(app.OpenTournament(app.FindTournamentByName(tournament_name)->GetUUID()));

		for (int i = 0; i < 14; ++i)
			tourney->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i));

		tourney->AddMatchTable(new SingleElimination(10, 200, tourney));
		tourney->GetMatchTables()[0]->SetMatID(1);
		tourney->GenerateSchedule();

		auto mat = app.GetDefaultMat();
		ASSERT_TRUE(mat);

		ZED::Core::Pause(2000);

		for (auto match : tourney->GetSchedule())
		{
			ASSERT_TRUE(match);
			ASSERT_TRUE(mat->StartMatch(match));			
			ZED::Core::Pause(1000);

			ASSERT_TRUE(mat->AreFightersOnMat());
			
			mat->Hajime();

			ZED::Core::Pause(1000);

			while (!mat->HasConcluded())
			{
				Fighter f = Fighter::White;
				if (rand() % 2 == 0)
					f = Fighter::Blue;

				int rnd = rand() % 6;

				if (rnd == 0)
					mat->AddIppon(f);
				else if (rnd == 1)
					mat->AddWazaAri(f);
				else if (rnd == 2)
					mat->AddWazaAri(f);
				else if (rnd == 3)
					mat->AddShido(f);
				else if (rnd == 4)
					mat->AddMedicalExamination(f);
				else
				{
					mat->Osaekomi(f);
					ZED::Core::Pause(11 * 1000);
				}

				ZED::Core::Pause(11 * 1000);
			}

			EXPECT_TRUE(mat->EndMatch());
			ZED::Core::Pause(11 * 1000);
		}

		ZED::Core::Pause(5000);
	}
}



TEST(App, FullTournament_SingleElimination7_BO3)
{
	initialize();

	{
		Application app;

		app.CloseTournament();
		app.SetTournamentList().clear();

		app.StartLocalMat(1);

		Account acc("admin", "1234", Account::AccessLevel::Admin);
		app.GetDatabase().AddAccount(acc);

		auto tournament_name = GetRandomName();
		auto tourney = new Tournament(tournament_name, new RuleSet("Test", 3 * 60, 3 * 60, 20, 10));
		tourney->EnableAutoSave(false);
		EXPECT_TRUE(app.AddTournament(tourney));

		ASSERT_TRUE(app.FindTournamentByName(tournament_name));
		EXPECT_TRUE(app.OpenTournament(app.FindTournamentByName(tournament_name)->GetUUID()));

		for (int i = 0; i < 7; ++i)
			tourney->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i));

		auto group = new SingleElimination(10, 200, tourney);
		group->SetMatID(1);
		group->IsBestOfThree(true);
		tourney->AddMatchTable(group);
		tourney->GenerateSchedule();

		auto mat = app.GetDefaultMat();
		ASSERT_TRUE(mat);

		ZED::Core::Pause(3000);

		for (auto match : tourney->GetSchedule())
		{
			if (!mat->StartMatch(match))
				continue;

			ZED::Core::Pause(1000);

			ASSERT_TRUE(mat->AreFightersOnMat());

			mat->Hajime();

			ZED::Core::Pause(1000);

			while (!mat->HasConcluded())
			{
				Fighter f = Fighter::White;
				if (rand() % 2 == 0)
					f = Fighter::Blue;

				int rnd = rand() % 6;

				if (rnd == 0)
					mat->AddIppon(f);
				else if (rnd == 1)
					mat->AddWazaAri(f);
				else if (rnd == 2)
					mat->AddWazaAri(f);
				else
				{
					mat->Osaekomi(f);
					ZED::Core::Pause(12 * 1000);
				}

				ZED::Core::Pause(10 * 1000);
			}

			ZED::Core::Pause(3000);

			EXPECT_TRUE(mat->EndMatch());
			ZED::Core::Pause(15 * 1000);
		}

		ZED::Core::Pause(5000);
	}
}



TEST(App, VeryLongNameTest)
{
	initialize();
	Application app;
	Mat m(1);

	Judoka j1("Very-Long-Firstname", "Very-Long-Lastname", rand() % 50);
	Judoka j2("Very-Long-Firstname", "Very-Long-Lastname", rand() % 50);

	Match match(&j1, &j2, nullptr);
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 25, 0, 30, 20, true, true, true, 0));

	ZED::Core::Pause(5000);

	EXPECT_TRUE(m.StartMatch(&match));
	m.Hajime();

	ZED::Core::Pause(30*1000);
	m.SetAsDraw();
}



TEST(App, MasterSlave)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080  + rand() % 10000);

	EXPECT_TRUE(master.IsMaster());
	EXPECT_FALSE(master.IsSlave());

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));

	EXPECT_TRUE(master.IsMaster());
	EXPECT_FALSE(master.IsSlave());

	EXPECT_FALSE(slave.IsMaster());
	EXPECT_TRUE(slave.IsSlave());
}



TEST(App, MasterClosesMatOfSlave)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080  + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));

	master.StartLocalMat(1);
	master.CloseMat(1);

	slave.StartLocalMat(2);

	ZED::Core::Pause(2000);
	EXPECT_TRUE(slave.GetDefaultMat()->IsOpen());

	master.CloseMat(2);

	ZED::Core::Pause(2000);

	EXPECT_FALSE(slave.GetDefaultMat()->IsOpen());
}



TEST(App, MatchOnSlave)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));

	master.StartLocalMat(1);
	master.CloseMat(1);

	slave.StartLocalMat(2);

	ZED::Core::Pause(100);

	Judoka j1("White", "LastnameW");
	Judoka j2("Blue",  "LastnameB");
	Match* match = new Match(&j1, &j2, nullptr);
	match->SetMatID(2);

	master.GetTournament()->AddMatch(match);

	auto mat = master.FindMat(2);

	ASSERT_TRUE(mat);
	ASSERT_TRUE(mat->StartMatch(match));

	ZED::Core::Pause(2000);

	EXPECT_TRUE(mat->AreFightersOnMat());

	mat->Hajime();

	mat->AddIppon(Fighter::White);

	ZED::Core::Pause(100);

	EXPECT_TRUE(mat->EndMatch());

	ZED::Core::Pause(100);

	EXPECT_EQ(match->GetResult().m_Winner, Fighter::White);
	EXPECT_EQ(match->GetResult().m_Score, Match::Score::Ippon);
}



/*TEST(App, MasterSlaveForceClose)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080  + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));

	master.StartLocalMat(1);

	slave.StartLocalMat(2);

	ZED::Core::Pause(5000);


	master.CloseTournament();
	master.SetTournamentList().clear();

	Account acc("admin", "1234", Account::AccessLevel::Admin);
	master.GetDatabase().AddAccount(acc);

	ZED::Core::Pause(5000);

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), rand() % 50);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), rand() % 50);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), rand() % 50);

	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
	Judoka j5(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
	Judoka j6(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);

	master.GetDatabase().AddJudoka(std::move(j1));
	master.GetDatabase().AddJudoka(std::move(j2));
	master.GetDatabase().AddJudoka(std::move(j3));

	master.GetDatabase().AddJudoka(std::move(j4));
	master.GetDatabase().AddJudoka(std::move(j5));
	master.GetDatabase().AddJudoka(std::move(j6));

	auto tournament_name = GetRandomName();
	auto tourney = new Tournament(tournament_name);
	tourney->EnableAutoSave(false);
	EXPECT_TRUE(master.AddTournament(tourney));

	master.OpenTournament(master.FindTournamentIndex(tournament_name));

	tourney->AddParticipant(&j1);
	tourney->AddParticipant(&j2);
	tourney->AddParticipant(&j3);
	tourney->AddParticipant(&j4);
	tourney->AddParticipant(&j5);
	tourney->AddParticipant(&j6);

	MatchTable* m1 = new Weightclass(tourney, 0, 49);
	MatchTable* m2 = new Weightclass(tourney, 50, 100);
	m1->SetMatID(1);
	m2->SetMatID(2);
	tourney->AddMatchTable(m1);
	tourney->AddMatchTable(m2);
}



TEST(App, MasterSlaveFullTournament)
{
	initialize();

	Application master(8080 + rand()%10000);
	Application slave(8080  + rand()%10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));

	master.StartLocalMat(1);
	slave.StartLocalMat(2);

	ZED::Core::Pause(5000);

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), rand() % 50);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), rand() % 50);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), rand() % 50);

	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
	Judoka j5(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
	Judoka j6(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);

	master.GetDatabase().AddJudoka(std::move(j1));
	master.GetDatabase().AddJudoka(std::move(j2));
	master.GetDatabase().AddJudoka(std::move(j3));

	master.GetDatabase().AddJudoka(std::move(j4));
	master.GetDatabase().AddJudoka(std::move(j5));
	master.GetDatabase().AddJudoka(std::move(j6));

	auto tournament_name = GetRandomName();
	auto tourney = new Tournament(tournament_name);
	tourney->EnableAutoSave(false);
	EXPECT_TRUE(master.AddTournament(tourney));

	ASSERT_TRUE(master.GetTournament());
	EXPECT_EQ(master.GetTournament()->GetName(), tourney->GetName());

	tourney->AddParticipant(&j1);
	tourney->AddParticipant(&j2);
	tourney->AddParticipant(&j3);
	tourney->AddParticipant(&j4);
	tourney->AddParticipant(&j5);
	tourney->AddParticipant(&j6);

	MatchTable* m1 = new Weightclass(tourney, 0, 49);
	MatchTable* m2 = new Weightclass(tourney, 50, 100);
	m1->SetMatID(2);
	m2->SetMatID(2);
	tourney->AddMatchTable(m1);
	tourney->AddMatchTable(m2);

	auto mat = master.FindMat(2);

	for (int i = 0; i < 6; i++)
	{
		ZED::Core::Pause(6000);
		auto match = tourney->GetNextMatch(mat->GetMatID());
		ASSERT_TRUE(match);
		EXPECT_TRUE(mat->StartMatch(match));

		ZED::Core::Pause(8000);

		mat->Hajime();

		ZED::Core::Pause(6000);

		while (!mat->HasConcluded())
		{
			Fighter f = Fighter::White;
			if (rand() % 2 == 0)
				f = Fighter::Blue;

			int rnd = rand() % 6;

			if (rnd == 0)
				mat->AddIppon(f);
			else if (rnd == 1)
				mat->AddWazaAri(f);
			else if (rnd == 2)
				mat->AddWazaAri(f);
			else if (rnd == 3)
				mat->AddShido(f);
			else if (rnd == 4)
				mat->AddMedicalExamination(f);
			else
			{
				mat->Osaekomi(f);
				ZED::Core::Pause(12 * 1000);
			}

			ZED::Core::Pause(10 * 1000);
		}

		ZED::Core::Pause(3000);

		EXPECT_TRUE(mat->EndMatch());
		ZED::Core::Pause(8000);
	}
}*/