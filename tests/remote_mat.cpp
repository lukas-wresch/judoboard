#include "tests.h"



TEST(RemoteMat, SalveMatGetCorrectID)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	master.StartLocalMat(1);

	ZED::Core::Pause(100);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));

	ASSERT_TRUE(slave.StartLocalMat(1));

	ZED::Core::Pause(100);

	auto mat1 = master.GetLocalMat();
	auto mat2 = slave.GetLocalMat();

	EXPECT_EQ(mat1->GetMatID(), 1);
	EXPECT_EQ(mat2->GetMatID(), 2);
}



TEST(RemoteMat, OpenAndClose)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	ZED::Core::Pause(100);

	IMat* m = master.FindMat(1);
	ASSERT_TRUE(m);

	EXPECT_TRUE(m->GetType() == IMat::Type::RemoteMat);
	EXPECT_TRUE(m->IsOpen());

	EXPECT_TRUE(m->Close());

	EXPECT_FALSE(m->IsOpen());

	m->Open();

	EXPECT_TRUE(m->IsOpen());

	EXPECT_TRUE(m->Close());

	m->Open();

	EXPECT_TRUE(m->IsOpen());

	EXPECT_FALSE(m->HasConcluded());
	EXPECT_EQ(m->GetOsaekomiList().size(), 0);
	//EXPECT_GE(m->RequestScreenshot().GetSize(), 1024u);//Would only work in software renderer
}



TEST(RemoteMat, QuickClose)
{
	initialize();
	DWORD time = ZED::Core::CurrentTimestamp();
	{
		Application master;
		Application slave;
		ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
		ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

		ASSERT_TRUE(master.GetHttpServer().IsRunning());
		ASSERT_TRUE( slave.GetHttpServer().IsRunning());

		ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
		ASSERT_TRUE(slave.StartLocalMat(1));
		ZED::Core::Pause(1000);

		IMat* m = master.FindMat(1);
		ASSERT_TRUE(m);
	}
	EXPECT_LE(ZED::Core::CurrentTimestamp() - time, 5000u);
}



TEST(RemoteMat, SlaveOpensMatTwice)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(2));

	master.CloseTournament();
	master.SetTournamentList().clear();

	master.StartLocalMat(1);
	slave.StartLocalMat(2);
	ZED::Core::Pause(100);

	IMat* mat[2];
	mat[0] = master.FindMat(1);
	mat[1] = master.FindMat(2);

	EXPECT_TRUE(mat[0]);
	EXPECT_TRUE(mat[1]);
}



TEST(RemoteMat, ForcedCloseDuringMatch)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));

	master.CloseTournament();
	master.SetTournamentList().clear();

	master.StartLocalMat(1);
	ASSERT_TRUE(slave.StartLocalMat(2));

	ZED::Core::Pause(2000);

	Judoka* j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), rand() % 50);
	Judoka* j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), rand() % 50);
	Judoka* j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), rand() % 50);

	Judoka* j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
	Judoka* j5 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
	Judoka* j6 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);

	auto tournament_name = GetRandomName();
	auto tourney = new Tournament(tournament_name, std::make_shared<RuleSet>("Test", 60, 0, 20, 10));
	tourney->EnableAutoSave(false);
	EXPECT_TRUE(master.AddTournament(tourney));

	ASSERT_TRUE(master.FindTournamentByName(tournament_name));
	EXPECT_TRUE(master.OpenTournament(master.FindTournamentByName(tournament_name)->GetUUID()));

	tourney->AddParticipant(j1);
	tourney->AddParticipant(j2);
	tourney->AddParticipant(j3);
	tourney->AddParticipant(j4);
	tourney->AddParticipant(j5);
	tourney->AddParticipant(j6);

	MatchTable* m1 = new RoundRobin(0, 49);
	MatchTable* m2 = new RoundRobin(50, 100);
	m1->SetMatID(1);
	m2->SetMatID(2);
	tourney->AddMatchTable(m1);
	tourney->AddMatchTable(m2);

	IMat* mat[2];
	mat[0] = master.FindMat(1);
	mat[1] = master.FindMat(2);

	ZED::Core::Pause(1000);

	for (int i = 0;i < 2;i++)
	{
		ASSERT_TRUE(mat[i]);

		auto match = tourney->GetNextMatch(mat[i]->GetMatID());
		EXPECT_TRUE(match);
		EXPECT_TRUE(mat[i]->StartMatch(match));

		ZED::Core::Pause(2000);

		mat[i]->Hajime();

		ZED::Core::Pause(500);

		Fighter f = Fighter::White;
		if (rand() % 2 == 0)
			f = Fighter::Blue;

		mat[i]->AddIppon(f);

		ZED::Core::Pause(2000);

		EXPECT_TRUE(mat[i]->EndMatch());
		ZED::Core::Pause(2000);
	}
}



TEST(RemoteMat, StartMatch)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	master.GetTournament()->GetDatabase().AddJudoka(j1);
	master.GetTournament()->GetDatabase().AddJudoka(j2);

	master.GetTournament()->AddMatch(Match(j1, j2, nullptr));
	auto match = master.GetTournament()->GetSchedule()[0];

	match->SetMatID(1);
	EXPECT_TRUE(m->StartMatch(match));

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 0);
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 0);
		EXPECT_EQ(m->GetScoreboard(f).m_Yuko, -1);
		EXPECT_EQ(m->GetScoreboard(f).m_Koka, -1);

		EXPECT_EQ(m->GetScoreboard(f).m_Shido, 0);
		EXPECT_FALSE(m->GetScoreboard(f).m_HansokuMake);
		EXPECT_EQ(m->GetScoreboard(f).m_MedicalExamination, 0);

		EXPECT_FALSE(m->GetScoreboard(f).m_Hantei);
	}

	EXPECT_FALSE(m->HasConcluded());
	EXPECT_FALSE(m->EndMatch());
	m->AddIppon(Fighter::White);
	EXPECT_TRUE(m->HasConcluded());
	EXPECT_TRUE(m->EndMatch());
	EXPECT_FALSE(m->HasConcluded());
}



TEST(RemoteMat, SendRuleSet)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	auto rules = std::make_shared<RuleSet>("Test", 5, 0, 30, 20, true, true, true, 0);

	master.GetTournament()->GetDatabase().AddJudoka(j1);
	master.GetTournament()->GetDatabase().AddJudoka(j2);
	master.GetTournament()->GetDatabase().AddRuleSet(rules);

	Match match(j1, j2, nullptr);
	match.SetMatID(1);
	match.SetRuleSet(rules);
	master.GetTournament()->AddMatch(std::move(match));

	ASSERT_TRUE(m);
	ASSERT_TRUE(m->StartMatch(master.GetTournament()->GetSchedule()[0]));

	auto remote_rules = slave.GetMats()[0]->GetMatch()->GetRuleSet();
	EXPECT_EQ(remote_rules.GetName(), "Test");
	EXPECT_EQ(remote_rules.GetMatchTime(), 5);
}



TEST(RemoteMat, SendMatchTable)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	master.GetTournament()->AddParticipant(j1);
	master.GetTournament()->AddParticipant(j2);

	MatchTable* m1 = new RoundRobin(0, 1000);
	m1->SetMatID(1);
	master.GetTournament()->AddMatchTable(m1);

	ASSERT_TRUE(m);
	ASSERT_TRUE(m->StartMatch(master.GetTournament()->GetSchedule()[0]));

	auto remote_table = slave.GetMats()[0]->GetMatch()->GetMatchTable();
	ASSERT_TRUE(remote_table);
}



TEST(RemoteMat, SendHajime)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	auto rules = std::make_shared<RuleSet>("Test", 5, 0, 30, 20, true, true, true, 0);

	master.GetTournament()->GetDatabase().AddJudoka(j1);
	master.GetTournament()->GetDatabase().AddJudoka(j2);
	master.GetTournament()->GetDatabase().AddRuleSet(rules);

	Match match(j1, j2, nullptr);
	match.SetMatID(1);
	match.SetRuleSet(rules);
	master.GetTournament()->AddMatch(std::move(match));

	ASSERT_TRUE(m);
	ASSERT_TRUE(m->StartMatch(master.GetTournament()->GetSchedule()[0]));
	m->Hajime();

	auto remote_mat = slave.GetMats()[0];	
	EXPECT_TRUE(remote_mat->IsHajime());
}



TEST(RemoteMat, CorrectWinner)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);
	auto rules = std::make_shared<RuleSet>("Test", 2, 0, 30, 20, true, true, true, 0);
	master.GetDatabase().AddRuleSet(rules);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		for (int i = 0; i <= 5; i++)
		{
			auto j1 = new Judoka(GetRandomName(), GetRandomName());
			auto j2 = new Judoka(GetRandomName(), GetRandomName());
			master.GetDatabase().AddJudoka(j1);
			master.GetDatabase().AddJudoka(j2);

			Match* match = new Match(j1, j2, nullptr);
			match->SetMatID(1);
			match->SetRuleSet(rules);
			master.GetTournament()->AddMatch(match);

			ASSERT_TRUE(m);
			ASSERT_TRUE(m->StartMatch(match));
			m->Hajime();
					
			if (i == 0)
				m->AddIppon(f);
			else if (i == 1)
				m->AddWazaAri(f);
			else if (i == 2)
				m->AddYuko(f);
			else if (i == 3)
				m->AddKoka(f);
			else if (i == 4)
				m->AddShido(!f);
			else
			{
				m->AddHansokuMake(!f);
				m->AddNoDisqualification(!f);
			}

			ZED::Core::Pause(500);

			if (i == 4)
				m->SetAsDraw();

			EXPECT_TRUE(m->EndMatch());

			ZED::Core::Pause(200);

			if (i == 4)
				EXPECT_EQ(match->GetResult().m_Winner, Winner::Draw);
			else
				EXPECT_EQ(match->GetResult().m_Winner, Fighter2Winner(f));
		}
	}
}



TEST(RemoteMat, ForceClose)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	Judoka j1("White", "LastnameW");
	Judoka j2("Blue",  "LastnameB");
	master.GetTournament()->GetDatabase().AddJudoka(&j1);
	master.GetTournament()->GetDatabase().AddJudoka(&j2);

	ZED::Core::Pause(500);

	Match match(&j1, &j2, nullptr);
	match.SetMatID(1);
	EXPECT_TRUE(m->StartMatch(&match));
}



TEST(RemoteMat, RemoveIpponShouldRecoverPreviousWazaari)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddWazaAri(f);
		m->AddIppon(f);

		m->RemoveIppon(f);

		EXPECT_FALSE(m->HasConcluded());

		EXPECT_FALSE(m->IsHajime());

		EXPECT_EQ(m->GetScoreboard(f).m_Ippon,   0);
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 1);

		m->AddIppon(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, RemoveWazariShouldRemoveIppon)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddWazaAri(f);
		m->AddWazaAri(f);

		m->RemoveWazaAri(f);

		EXPECT_FALSE(m->HasConcluded());

		EXPECT_FALSE(m->IsHajime());

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon   == 0);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 1);

		m->AddIppon(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Scores)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 0);
		m->AddIppon(f);
		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 1);
		m->RemoveIppon(f);
		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 0);

				
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 0);
		m->AddWazaAri(f);
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 1);
		m->RemoveWazaAri(f);
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 0);


		m->AddIppon(f);
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Shido)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 0);
		m->AddShido(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 1);
		m->RemoveShido(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 0);
		m->AddShido(f);


		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 1);
		m->AddShido(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 2);
		m->RemoveShido(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 1);
		m->AddShido(f);

		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 2);
		m->AddShido(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 3);
		m->RemoveShido(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 2);
		m->AddShido(f);

		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 3);
		m->AddShido(f);
		ZED::Core::Pause(100);
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 3);
		EXPECT_TRUE(m->GetScoreboard(f).m_HansokuMake);

		m->RemoveShido(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 2);
		EXPECT_FALSE(m->GetScoreboard(f).m_HansokuMake);

		m->AddShido(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 3);
		EXPECT_TRUE(m->GetScoreboard(f).m_HansokuMake);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, ShidoDoesntEndGoldenScore)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 2, 60, 30, 20, false, true, true, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();

		ZED::Core::Pause(3 * 1000);
		EXPECT_TRUE(m->IsOutOfTime());

		EXPECT_FALSE(m->HasConcluded());

		m->EnableGoldenScore();

		m->Hajime();
		m->AddShido(f);

		EXPECT_TRUE(m->IsHajime());
		EXPECT_FALSE(m->HasConcluded());

		m->AddIppon(f);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, ScoreEndsGoldenScore)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		for (int i = 0; i <= 3; i++)
		{
			auto j1 = new Judoka(GetRandomName(), GetRandomName());
			auto j2 = new Judoka(GetRandomName(), GetRandomName());
			master.GetDatabase().AddJudoka(j1);
			master.GetDatabase().AddJudoka(j2);

			auto rules = std::make_shared<RuleSet>("Test", 5, 60, 30, 20, true, true, true, 0);
			master.GetDatabase().AddRuleSet(rules);

			Match* match = new Match(j1, j2, nullptr);
			match->SetMatID(1);
			match->SetRuleSet(rules);
			master.GetTournament()->AddMatch(match);

			EXPECT_TRUE(m->StartMatch(match));
			m->Hajime();

			ZED::Core::Pause(5500);

			EXPECT_TRUE(m->EnableGoldenScore());
			m->Hajime();

			if (i == 0)
				m->AddIppon(f);
			else if (i == 1)
				m->AddWazaAri(f);
			else if (i == 2)
				m->AddYuko(f);
			else
				m->AddKoka(f);

			EXPECT_FALSE(m->IsHajime());
			EXPECT_TRUE(m->EndMatch());
			EXPECT_EQ(match->GetResult().m_Winner, Fighter2Winner(f));
		}
	}
}



TEST(RemoteMat, ShidosResultInIndirectHansokumake)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));
		
		m->AddShido(f);
		m->AddShido(f);
		m->AddShido(f);		

		EXPECT_TRUE(m->EndMatch());

		auto& events = match->GetLog().GetEvents();

		ASSERT_GE(events.size(), 3);
		EXPECT_EQ(events[events.size() - 3].m_BiasedEvent, MatchLog::BiasedEvent::AddHansokuMake_Indirect);
		EXPECT_EQ(events[events.size() - 3].m_Group, f);

		EXPECT_EQ(events[events.size() - 2].m_BiasedEvent, MatchLog::BiasedEvent::AddIppon);
		EXPECT_EQ(events[events.size() - 2].m_Group, !f);
	}
}



TEST(RemoteMat, HansokumakeResultsInDirectHansokumake)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddHansokuMake(f);
		m->AddDisqualification(f);

		EXPECT_TRUE(m->EndMatch());

		auto& events = match->GetLog().GetEvents();

		ASSERT_TRUE(events.size() >= 4);
		EXPECT_TRUE(events[events.size() - 4].m_BiasedEvent == MatchLog::BiasedEvent::AddHansokuMake_Direct);
		EXPECT_TRUE(events[events.size() - 4].m_Group == f);

		EXPECT_TRUE(events[events.size() - 3].m_BiasedEvent == MatchLog::BiasedEvent::AddIppon);
		EXPECT_TRUE(events[events.size() - 3].m_Group == !f);

		EXPECT_TRUE(events[events.size() - 2].m_BiasedEvent == MatchLog::BiasedEvent::AddDisqualification);
		EXPECT_TRUE(events[events.size() - 2].m_Group == f);
	}
}



TEST(RemoteMat, DirectHansokumakeDoesNotConcludeMatch)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddHansokuMake(f);

		ZED::Core::Pause(100);

		EXPECT_FALSE(m->HasConcluded());
		EXPECT_FALSE(m->EndMatch());

		m->AddDisqualification(f);
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, DirectHansokumakeAndDisqDoesConcludeMatch)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		m->AddHansokuMake(f);
		m->AddDisqualification(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Gachi)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddGachi(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, GachiResultsInMate)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddGachi(f);

		EXPECT_FALSE(m->IsHajime());

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, DoubleIppon)
{
	//IJF 2018: In the case where both contestants score ippon simultaneously during the time
	//allotted for regular time, the contest shall be decided by a “golden score” period
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddIppon(f);
		m->AddIppon(!f);

		EXPECT_FALSE(m->HasConcluded());
		m->EnableGoldenScore();

		ZED::Core::Pause(100);

		EXPECT_TRUE(m->IsGoldenScore());

		EXPECT_FALSE(m->IsHajime());

		m->AddIppon(f);

		ZED::Core::Pause(100);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, DoubleIpponFightersKeepWazaari)
{
	//IJF 2018: In the case where both contestants score ippon simultaneously during the time
	//allotted for regular time, the contest shall be decided by a “golden score” period
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddWazaAri(f);
		m->AddWazaAri(!f);

		m->AddIppon(f);
		m->AddIppon(!f);

		EXPECT_FALSE(m->HasConcluded());
		m->EnableGoldenScore();

		EXPECT_TRUE(m->IsGoldenScore());

		EXPECT_FALSE(m->IsHajime());

		EXPECT_EQ(m->GetScoreboard(Fighter::White).m_WazaAri, 1);
		EXPECT_EQ(m->GetScoreboard(Fighter::Blue ).m_WazaAri, 1);

		m->AddIppon(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, DoubleIpponDuringGoldenScore)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 1, 60, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));
		
		m->Hajime();
		ZED::Core::Pause(2000);

		EXPECT_TRUE(m->EnableGoldenScore());
		EXPECT_TRUE(m->IsGoldenScore());

		m->Hajime();
		m->AddIppon(f);
		m->AddIppon(!f);

		EXPECT_FALSE(m->IsHajime());
		EXPECT_FALSE(m->HasConcluded());

		m->Hajime();
		m->AddIppon(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, DoubleIpponDuringGoldenScoreFightersKeepWazaari)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 1, 60, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		m->AddWazaAri(f);
		m->AddWazaAri(!f);
		ZED::Core::Pause(2000);

		EXPECT_TRUE(m->EnableGoldenScore());
		ZED::Core::Pause(100);
		EXPECT_TRUE(m->IsGoldenScore());

		m->Hajime();
		m->AddIppon(f);
		m->AddIppon(!f);

		ZED::Core::Pause(100);

		EXPECT_FALSE(m->IsHajime());
		EXPECT_FALSE(m->HasConcluded());

		EXPECT_EQ(m->GetScoreboard(Fighter::White).m_WazaAri, 1);
		EXPECT_EQ(m->GetScoreboard(Fighter::Blue ).m_WazaAri, 1);

		m->Hajime();
		ZED::Core::Pause(100);
		m->AddIppon(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, IpponResultsInMate)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddIppon(f);

		EXPECT_FALSE(m->IsHajime());

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, HansokumakeResultsInMate)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->AddHansokuMake(f);

		EXPECT_FALSE(m->IsHajime());

		m->AddNoDisqualification(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, ThirdShidoIsHansokumake)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		m->AddShido(f);
		m->AddShido(f);
		m->AddShido(f);

		EXPECT_TRUE(m->GetScoreboard(f).m_HansokuMake);				

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, DoubleHansokumake)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	//IJF rules 2018:
	//In regular time or golden score if both athletes receive three shido, both athletes
	//will be considered losers, the contest result will be recorded as 0 - 0

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		m->AddShido(f);
		m->AddShido(f);
		m->AddShido(!f);
		m->AddShido(!f);

		m->AddShido(f);
		m->AddShido(!f);

		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_HansokuMake);
		EXPECT_TRUE(m->GetScoreboard(!f).m_HansokuMake);

		m->AddNoDisqualification(f);
		m->AddNoDisqualification(!f);

		ZED::Core::Pause(100);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		EXPECT_TRUE(match->GetResult().m_Winner == Judoboard::Winner::Draw);
		EXPECT_TRUE((int)match->GetResult().m_Score == 0);
	}
}



TEST(RemoteMat, DoubleGachi)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	//In the case when both contestants do not show up for the match

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		m->AddGachi(f);
		m->AddGachi(!f);

		ZED::Core::Pause(100);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		ZED::Core::Pause(100);

		EXPECT_EQ(match->GetResult().m_Winner, Judoboard::Winner::Draw);
		EXPECT_EQ((int)match->GetResult().m_Score, 0);
	}
}



TEST(RemoteMat, Hansokumake)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		EXPECT_FALSE(m->GetScoreboard(f).m_HansokuMake);
		m->AddHansokuMake(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_HansokuMake);
		m->AddDisqualification(f);
		EXPECT_TRUE(m->HasConcluded());

		m->RemoveHansokuMake(f);
		EXPECT_FALSE(m->GetScoreboard(f).m_HansokuMake);

		m->RemoveIppon(!f);
		EXPECT_FALSE(m->HasConcluded());

		EXPECT_FALSE(m->EndMatch());

		m->AddHansokuMake(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_HansokuMake);

		m->AddDisqualification(f);
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, MedicalExaminiations)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 0);
		m->AddMedicalExamination(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 1);
		m->RemoveMedicalExamination(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 0);
		m->AddMedicalExamination(f);
		ZED::Core::Pause(100);


		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 1);
		m->AddMedicalExamination(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 2);
		m->RemoveMedicalExamination(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 1);
		m->AddMedicalExamination(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 2);
		m->AddMedicalExamination(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 2);
		m->RemoveMedicalExamination(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 1);
		m->AddMedicalExamination(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 2);

		EXPECT_FALSE(m->EndMatch());

		m->AddIppon(f);
		ZED::Core::Pause(100);
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, MatchTime)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 120; time <= 4*60 ; time += 90 + rand()%60)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test" + std::to_string(time), time, 60, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));
		ZED::Core::Pause(100);

		m->Hajime();

		for (int k = 0; k < time - 10; k++)
		{
			EXPECT_FALSE(m->IsOutOfTime());
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(10 * 1000);

		ZED::Core::Pause(2000);
		EXPECT_TRUE(m->IsOutOfTime());
	}
}



TEST(RemoteMat, GoldenScoreTime)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 30; time <= 3 * 60; time += 70 + rand() % 50)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test" + std::to_string(time), 5, time, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		ZED::Core::Pause(6000);

		EXPECT_TRUE(m->EnableGoldenScore());
		ZED::Core::Pause(100);

		m->Hajime();
		ZED::Core::Pause(100);

		for (int k = 0; k < time - 10; k++)
		{
			EXPECT_FALSE(m->IsOutOfTime());
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(10 * 1000);
		ZED::Core::Pause(2000);
		EXPECT_TRUE(m->IsOutOfTime());

		m->AddIppon(Fighter::White);
		ZED::Core::Pause(100);
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, OsaekomiTime)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 1; time <= 30; time += 6 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			auto j1 = new Judoka(GetRandomName(), GetRandomName());
			auto j2 = new Judoka(GetRandomName(), GetRandomName());
			master.GetDatabase().AddJudoka(j1);
			master.GetDatabase().AddJudoka(j2);

			auto rules = std::make_shared<RuleSet>("Test", 100, 0, time, 20, false, false, false, 0);
			master.GetDatabase().AddRuleSet(rules);

			Match* match = new Match(j1, j2, nullptr);
			match->SetMatID(1);
			match->SetRuleSet(rules);
			master.GetTournament()->AddMatch(match);

			EXPECT_TRUE(m->StartMatch(match));

			m->Hajime();
			m->Osaekomi(f);

			for (int k = 0; k < time - 1; k++)
			{
				EXPECT_FALSE(m->IsOutOfTime());
				EXPECT_FALSE(m->HasConcluded());
				EXPECT_FALSE(m->EndMatch());
				ZED::Core::Pause(1000);
			}

			ZED::Core::Pause(2000);
			EXPECT_TRUE(m->HasConcluded());
			EXPECT_TRUE(m->EndMatch());
		}
	}
}



TEST(RemoteMat, OsaekomiWithWazaAriTime)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 1; time <= 25; time += 5 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			auto j1 = new Judoka(GetRandomName(), GetRandomName());
			auto j2 = new Judoka(GetRandomName(), GetRandomName());
			master.GetDatabase().AddJudoka(j1);
			master.GetDatabase().AddJudoka(j2);

			auto rules = std::make_shared<RuleSet>("Test", 100, 0, 100, time, false, false, false, 0);
			master.GetDatabase().AddRuleSet(rules);

			Match* match = new Match(j1, j2, nullptr);
			match->SetMatID(1);
			match->SetRuleSet(rules);
			master.GetTournament()->AddMatch(match);

			EXPECT_TRUE(m->StartMatch(match));

			m->Hajime();
			m->AddWazaAri(f);
			m->Osaekomi(f);

			for (int k = 0; k < time - 1; k++)
			{
				EXPECT_FALSE(m->IsOutOfTime());
				EXPECT_FALSE(m->HasConcluded());
				EXPECT_FALSE(m->EndMatch());
				ZED::Core::Pause(1000);
			}

			ZED::Core::Pause(2000);
			EXPECT_TRUE(m->HasConcluded());
			EXPECT_TRUE(m->EndMatch());
		}
	}
}



TEST(RemoteMat, OsaekomiUkeGainsIppon)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);
	
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 100, 0, 100, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		m->Osaekomi(f);

		ZED::Core::Pause(5000);

		m->AddIppon(!f);

		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		EXPECT_TRUE(match->GetResult().m_Winner == !f);
		EXPECT_TRUE(match->GetResult().m_Score  == Match::Score::Ippon);
	}
}



TEST(RemoteMat, OsaekomiToriGivesUp)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 100, 0, 100, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		m->Osaekomi(f);

		ZED::Core::Pause(5000);

		m->AddGachi(f);

		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		EXPECT_TRUE(match->GetResult().m_Winner == !f);
		EXPECT_TRUE(match->GetResult().m_Score  == Match::Score::Ippon);
	}
}



TEST(RemoteMat, OsaekomiWithWazaAriRemoved)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 1; time <= 25; time += 5 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			auto j1 = new Judoka(GetRandomName(), GetRandomName());
			auto j2 = new Judoka(GetRandomName(), GetRandomName());
			master.GetDatabase().AddJudoka(j1);
			master.GetDatabase().AddJudoka(j2);

			auto rules = std::make_shared<RuleSet>("Test", 100, 0, time*2, time, false, false, false, 0);
			master.GetDatabase().AddRuleSet(rules);

			Match* match = new Match(j1, j2, nullptr);
			match->SetMatID(1);
			match->SetRuleSet(rules);
			master.GetTournament()->AddMatch(match);

			EXPECT_TRUE(m->StartMatch(match));

			m->Hajime();
			m->AddWazaAri(f);
			m->Osaekomi(f);	

			for (int k = 0; k < time - 1; k++)
			{
				EXPECT_FALSE(m->IsOutOfTime());
				EXPECT_FALSE(m->HasConcluded());
				EXPECT_FALSE(m->EndMatch());
				ZED::Core::Pause(1000);
			}

			m->RemoveWazaAri(f);

			for (int k = 0; k < time - 1; k++)
			{
				EXPECT_FALSE(m->IsOutOfTime());
				EXPECT_FALSE(m->HasConcluded());
				EXPECT_FALSE(m->EndMatch());
				ZED::Core::Pause(1000);
			}

			ZED::Core::Pause(3000);

			EXPECT_TRUE(m->HasConcluded());
			EXPECT_TRUE(m->EndMatch());
		}
	}
}



TEST(RemoteMat, OsaekomiTillEndDuringGoldenScore)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	//Osaekomi should go the full distance even during golden score
	//and now get cancelled after reaching the wazaari time

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 1; time <= 25; time += 5 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			auto j1 = new Judoka(GetRandomName(), GetRandomName());
			auto j2 = new Judoka(GetRandomName(), GetRandomName());
			master.GetDatabase().AddJudoka(j1);
			master.GetDatabase().AddJudoka(j2);

			auto rules = std::make_shared<RuleSet>("Test", 5, 60, 2*time, time, false, false, false, 0);
			master.GetDatabase().AddRuleSet(rules);

			Match* match = new Match(j1, j2, nullptr);
			match->SetMatID(1);
			match->SetRuleSet(rules);
			master.GetTournament()->AddMatch(match);

			EXPECT_TRUE(m->StartMatch(match));

			m->Hajime();
			ZED::Core::Pause(6000);

			m->EnableGoldenScore();
			m->Hajime();

			m->Osaekomi(f);

			for (int k = 0; k < 2*time - 1; k++)
			{
				EXPECT_FALSE(m->IsOutOfTime());
				EXPECT_FALSE(m->HasConcluded());
				EXPECT_FALSE(m->EndMatch());
				ZED::Core::Pause(1000);
			}

			ZED::Core::Pause(2000);
			EXPECT_TRUE(m->HasConcluded());
			EXPECT_TRUE(m->EndMatch());
		}
	}
}



TEST(RemoteMat, Sonomama)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 500, 0, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);
	
		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		m->Osaekomi(f);

		ZED::Core::Pause(5000);
		m->Sonomama();
		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_FALSE(m->IsOsaekomiRunning());
		EXPECT_TRUE(m->GetOsaekomiHolder() == f);

		ZED::Core::Pause(5000);
		m->Hajime();
		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());
		EXPECT_TRUE(m->GetOsaekomiHolder() == f);

		ZED::Core::Pause(5000);
		m->Sonomama();
		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_FALSE(m->IsOsaekomiRunning());
		EXPECT_TRUE(m->GetOsaekomiHolder() == f);

		ZED::Core::Pause(25 * 1000);

		m->Hajime();
		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());
		EXPECT_TRUE(m->GetOsaekomiHolder() == f);
		ZED::Core::Pause(5000);

		EXPECT_FALSE(m->HasConcluded());

		ZED::Core::Pause(16 * 1000);

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 1);
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Tokeda)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 500, 0, 20, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);
		
		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		m->Osaekomi(f);

		ZED::Core::Pause(5000);

		m->Tokeda();
		ZED::Core::Pause(100);

		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_FALSE(m->IsOsaekomiRunning());

		ASSERT_EQ(m->GetOsaekomiList().size(), 1);
		EXPECT_TRUE(m->GetOsaekomiList()[0].m_Who == f);
		EXPECT_LE(std::abs((int)m->GetOsaekomiList()[0].m_Time - 5000), 200);

		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_FALSE(m->IsOsaekomiRunning());

		ASSERT_EQ(m->GetOsaekomiList().size(), 1);
		EXPECT_TRUE(m->GetOsaekomiList()[0].m_Who == f);
		EXPECT_LE(std::abs((int)m->GetOsaekomiList()[0].m_Time - 5000), 20);

		ZED::Core::Pause(5000);

		m->Osaekomi(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());

		m->Osaekomi(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());

		ZED::Core::Pause(5000);

		m->Tokeda();
		ZED::Core::Pause(100);

		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_FALSE(m->IsOsaekomiRunning());

		ASSERT_EQ(m->GetOsaekomiList().size(), 2);
		EXPECT_EQ(m->GetOsaekomiList()[1].m_Who, f);
		EXPECT_LE(std::abs((int)m->GetOsaekomiList()[1].m_Time - 5000), 250);

		ZED::Core::Pause(10 * 1000);

		m->Osaekomi(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());
		ZED::Core::Pause(15 * 1000);

		EXPECT_FALSE(m->HasConcluded());

		ZED::Core::Pause(6 * 1000);

		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 1);
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, OsaekomiSwitch)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 500, 0, 25, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);
		
		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		m->Osaekomi(f);

		ZED::Core::Pause(10 * 1000);

		m->Osaekomi(!f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());

		m->Osaekomi(!f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());

		ZED::Core::Pause(10 * 1000);

		m->Osaekomi(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());

		m->Osaekomi(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());

		ZED::Core::Pause(10 * 1000);

		EXPECT_FALSE(m->IsOsaekomiRunning());

		ASSERT_TRUE(m->GetOsaekomiList().size() == 1);
		EXPECT_TRUE(m->GetOsaekomiList()[0].m_Who == f);

		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 1);
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}


TEST(RemoteMat, MatchContinuesDuringOsaekomi)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 10, 0, 10, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		ZED::Core::Pause(8 * 1000);
		m->Osaekomi(f);
		ZED::Core::Pause(100);

		for (int k = 0; k < 9; k++)
		{
			EXPECT_FALSE(m->IsOutOfTime());
			EXPECT_FALSE(m->HasConcluded());
			EXPECT_FALSE(m->EndMatch());
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(2000);
		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 1);
		EXPECT_TRUE(m->IsOutOfTime());
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Yuko)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 60, 60, 30, 20, true, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == 0);
		for (int k = 1; k < 25; k++)
		{
			m->AddYuko(f);
			ZED::Core::Pause(100);
			EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == k);
		}
		for (int k = 1; k < 25; k++)
			m->RemoveYuko(f);
		ZED::Core::Pause(100);
		EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == 0);

		m->AddIppon(f);

		ZED::Core::Pause(100);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Yuko2)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 60, 60, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		EXPECT_EQ(m->GetScoreboard(f).m_Yuko, -1);
		for (int k = 1; k < 25; k++)
		{
			m->AddYuko(f);
			EXPECT_EQ(m->GetScoreboard(f).m_Yuko, -1);
		}
		for (int k = 1; k < 25; k++)
		{
			m->RemoveYuko(f);
			EXPECT_EQ(m->GetScoreboard(f).m_Yuko, -1);
		}

		m->AddIppon(f);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, ShidoForToriDuringOsaekomi)
{
	//IJF 2018: When osaekomi is being applied and tori commits an infringement meriting a
	//penalty(shido) :
	//If the osaekomi time is less than 10 seconds, the referee shall announce
	//mate, return the contestants to their standing positions, award the penalty then
	//recommence the contest by announcing hajime.
	
	//Is a shido is commited by Tori during osaekomi, mate is called, shido is given and then a score (if applicable)

	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 60, 60, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		m->Osaekomi(f);
		ZED::Core::Pause(5000);
		m->Mate();
		ZED::Core::Pause(1000);
		m->AddShido(f);

		EXPECT_FALSE(m->IsHajime());
		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_FALSE(m->IsOsaekomiRunning());
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 1);

		ZED::Core::Pause(5000);
		m->AddIppon(f);				

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, MateDuringSonomama)
{
	//Mate can indeed be called during sonomama in the case the judges want to give a hansokumake
	//or to discuss something with the other judges to give tori a shido

	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter osaekomi_holder = Fighter::White; osaekomi_holder <= Fighter::Blue; osaekomi_holder++)
	{
		EXPECT_TRUE(m->Open());

		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 60, 60, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();
		m->Osaekomi(osaekomi_holder);

		ZED::Core::Pause(3000);
		m->Sonomama();
		ZED::Core::Pause(5000);
		m->Mate();
		ZED::Core::Pause(5000);

		EXPECT_FALSE(m->IsHajime());
		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_FALSE(m->IsOsaekomiRunning());
		EXPECT_FALSE(m->HasConcluded());

		m->AddIppon(osaekomi_holder);
		EXPECT_TRUE(m->EndMatch());			
	}
}



TEST(RemoteMat, HansokumakeDuringOsaekomi)
{
	//IJF 2018: However, should the penalty to be awarded be hansoku-make, the referee shall,
	//after announcing sono - mama, consult with the judges, announce mate to return
	//the contestants to their starting positions, then award hansoku - make and end the
	//contest by announcing sore - made.

	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter osaekomi_holder = Fighter::White; osaekomi_holder <= Fighter::Blue; osaekomi_holder++)
	{
		for (Fighter hansokumake_committer = Fighter::White; hansokumake_committer <= Fighter::Blue; hansokumake_committer++)
		{
			EXPECT_TRUE(m->Open());

			auto j1 = new Judoka(GetRandomName(), GetRandomName());
			auto j2 = new Judoka(GetRandomName(), GetRandomName());
			master.GetDatabase().AddJudoka(j1);
			master.GetDatabase().AddJudoka(j2);

			auto rules = std::make_shared<RuleSet>("Test", 60, 60, 30, 20, false, false, false, 0);
			master.GetDatabase().AddRuleSet(rules);

			Match* match = new Match(j1, j2, nullptr);
			match->SetMatID(1);
			match->SetRuleSet(rules);
			master.GetTournament()->AddMatch(match);

			EXPECT_TRUE(m->StartMatch(match));

			m->Hajime();
			m->Osaekomi(osaekomi_holder);

			ZED::Core::Pause(3000);
			m->Sonomama();
			ZED::Core::Pause(5000);
			m->Mate();
			ZED::Core::Pause(3000);
			m->AddHansokuMake(hansokumake_committer);
			m->AddNoDisqualification(hansokumake_committer);
			ZED::Core::Pause(3000);

			EXPECT_FALSE(m->IsHajime());
			EXPECT_FALSE(m->IsOsaekomi());
			EXPECT_FALSE(m->IsOsaekomiRunning());
			EXPECT_TRUE(m->GetScoreboard(hansokumake_committer).m_HansokuMake);


			EXPECT_TRUE(m->EndMatch());
			EXPECT_EQ(match->GetResult().m_Winner, !hansokumake_committer);
			EXPECT_EQ(match->GetResult().m_Score,  Match::Score::Ippon);
		}
	}
}



TEST(RemoteMat, Koka)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 60, 60, 30, 20, false, true, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		EXPECT_EQ(m->GetScoreboard(f).m_Koka, 0);
		for (int k = 1; k < 25; k++)
		{
			m->AddKoka(f);
			ZED::Core::Pause(100);
			EXPECT_EQ(m->GetScoreboard(f).m_Koka, k);
		}
		for (int k = 1; k < 25; k++)
			m->RemoveKoka(f);
		EXPECT_EQ(m->GetScoreboard(f).m_Koka, 0);

		m->AddIppon(f);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Koka2)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 60, 60, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));


		EXPECT_EQ(m->GetScoreboard(f).m_Koka, -1);
		for (int k = 1; k < 25; k++)
		{
			m->AddKoka(f);
			EXPECT_EQ(m->GetScoreboard(f).m_Koka, -1);
		}
		for (int k = 1; k < 25; k++)
		{
			m->RemoveKoka(f);
			EXPECT_EQ(m->GetScoreboard(f).m_Koka, -1);
		}

		m->AddIppon(f);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, WazariAwaseteIppon)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

			
		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 0);
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 0);
		EXPECT_FALSE(m->EndMatch());

		m->AddWazaAri(f);
		ZED::Core::Pause(100);

		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 0);
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 1);
		EXPECT_FALSE(m->EndMatch());

		m->AddWazaAri(f);
		ZED::Core::Pause(100);

		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 1);
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 2);

		m->RemoveWazaAri(f);
		ZED::Core::Pause(100);

		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 0);
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 1);

		m->AddWazaAri(f);
		ZED::Core::Pause(100);

		EXPECT_EQ(m->GetScoreboard(f).m_Ippon, 1);
		EXPECT_EQ(m->GetScoreboard(f).m_WazaAri, 2);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, GoldenScore)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	master.GetDatabase().AddJudoka(j1);
	master.GetDatabase().AddJudoka(j2);

	auto rules = std::make_shared<RuleSet>("Test", 10, 10, 30, 20, false, false, true, 0);
	master.GetDatabase().AddRuleSet(rules);

	Match* match = new Match(j1, j2, nullptr);
	match->SetMatID(1);
	match->SetRuleSet(rules);
	master.GetTournament()->AddMatch(match);

	EXPECT_TRUE(m->StartMatch(match));

	m->Hajime();

	ZED::Core::Pause(11 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	m->EnableGoldenScore();
	ZED::Core::Pause(100);

	m->Hajime();
	ZED::Core::Pause(100);

	EXPECT_FALSE(m->IsOutOfTime());

	ZED::Core::Pause(11 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());
	EXPECT_FALSE(m->EndMatch());

	m->Hantei(Fighter::White);//This also ends the match
}



TEST(RemoteMat, GoldenScore2)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	master.GetDatabase().AddJudoka(j1);
	master.GetDatabase().AddJudoka(j2);

	auto rules = std::make_shared<RuleSet>("Test", 10, 0, 30, 20, false, false, true, 0);
	master.GetDatabase().AddRuleSet(rules);

	Match* match = new Match(j1, j2, nullptr);
	match->SetMatID(1);
	match->SetRuleSet(rules);
	master.GetTournament()->AddMatch(match);

	EXPECT_TRUE(m->StartMatch(match));

	m->Hajime();

	ZED::Core::Pause(11 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	EXPECT_FALSE(m->EnableGoldenScore());

	m->Hajime();

	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());
	EXPECT_FALSE(m->EndMatch());

	m->Hantei(Fighter::White);//This also ends the match
}



TEST(RemoteMat, GoldenScoreResetTime)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	master.GetDatabase().AddJudoka(j1);
	master.GetDatabase().AddJudoka(j2);

	auto rules = std::make_shared<RuleSet>("Test", 5, 5, 30, 20, false, false, true, 0);
	master.GetDatabase().AddRuleSet(rules);

	Match* match = new Match(j1, j2, nullptr);
	match->SetMatID(1);
	match->SetRuleSet(rules);
	master.GetTournament()->AddMatch(match);

	EXPECT_TRUE(m->StartMatch(match));

	m->Hajime();

	ZED::Core::Pause(6 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	EXPECT_TRUE(m->EnableGoldenScore());

	EXPECT_EQ(m->GetTime2Display(), 0);

	m->AddIppon(Fighter::White);

	EXPECT_TRUE(m->HasConcluded());
	EXPECT_TRUE(m->EndMatch());
}



TEST(RemoteMat, GoldenScoreKeepsShidosAndMedicalExaminations)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	master.GetDatabase().AddJudoka(j1);
	master.GetDatabase().AddJudoka(j2);

	auto rules = std::make_shared<RuleSet>("Test", 5, 5, 30, 20, false, false, true, 0);
	master.GetDatabase().AddRuleSet(rules);

	Match* match = new Match(j1, j2, nullptr);
	match->SetMatID(1);
	match->SetRuleSet(rules);
	master.GetTournament()->AddMatch(match);

	EXPECT_TRUE(m->StartMatch(match));

	m->Hajime();

	m->AddShido(Fighter::White);
	m->AddShido(Fighter::Blue);
	m->AddMedicalExamination(Fighter::White);
	m->AddMedicalExamination(Fighter::Blue);

	ZED::Core::Pause(6 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	EXPECT_TRUE(m->EnableGoldenScore());
	m->Hajime();

	EXPECT_TRUE(m->GetScoreboard(Fighter::White).m_Shido == 1);
	EXPECT_TRUE(m->GetScoreboard(Fighter::Blue ).m_Shido == 1);
	EXPECT_TRUE(m->GetScoreboard(Fighter::White).m_MedicalExamination == 1);
	EXPECT_TRUE(m->GetScoreboard(Fighter::Blue ).m_MedicalExamination == 1);

	m->AddIppon(Fighter::White);

	EXPECT_TRUE(m->HasConcluded());
	EXPECT_TRUE(m->EndMatch());
}



TEST(RemoteMat, Draw)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	master.GetDatabase().AddJudoka(j1);
	master.GetDatabase().AddJudoka(j2);

	auto rules = std::make_shared<RuleSet>("Test", 5, 60, 30, 20, false, false, true, 0);
	master.GetDatabase().AddRuleSet(rules);

	Match* match = new Match(j1, j2, nullptr);
	match->SetMatID(1);
	match->SetRuleSet(rules);
	master.GetTournament()->AddMatch(match);

	EXPECT_TRUE(m->StartMatch(match));

	m->Hajime();

	for (int k = 0; k < 4; k++)
	{
		EXPECT_FALSE(m->IsOutOfTime());
		ZED::Core::Pause(1000);
	}

	ZED::Core::Pause(2000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	m->SetAsDraw(true);//This also ends the match
}



TEST(RemoteMat, Draw2)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	master.GetDatabase().AddJudoka(j1);
	master.GetDatabase().AddJudoka(j2);

	auto rules = std::make_shared<RuleSet>("Test", 5, 60, 30, 20, false, false, false, 0);
	master.GetDatabase().AddRuleSet(rules);

	Match* match = new Match(j1, j2, nullptr);
	match->SetMatID(1);
	match->SetRuleSet(rules);
	master.GetTournament()->AddMatch(match);
	
	EXPECT_TRUE(m->StartMatch(match));
	ZED::Core::Pause(100);

	m->Hajime();

	for (int k = 0; k < 4; k++)
	{
		EXPECT_FALSE(m->IsOutOfTime());
		ZED::Core::Pause(1000);
	}

	ZED::Core::Pause(2000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	m->SetAsDraw(true);
	ZED::Core::Pause(100);

	EXPECT_FALSE(m->HasConcluded());
	EXPECT_FALSE(m->EndMatch());

	m->Hantei(Fighter::White);//This also ends the match
}



TEST(RemoteMat, Hantei)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = std::make_shared<RuleSet>("Test", 2, 60, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rules);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));

		m->Hajime();

		ZED::Core::Pause(3000);
		EXPECT_TRUE(m->IsOutOfTime());

		EXPECT_FALSE(m->HasConcluded());

		m->Hantei(f);
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		EXPECT_EQ(match->GetResult().m_Winner, Fighter2Winner(f));
		EXPECT_EQ(match->GetResult().m_Score,  Match::Score::Hantei);
		EXPECT_EQ(match->GetResult().m_Score,  (Match::Score)1);
	}
}



TEST(RemoteMat, BreakTime)
{
	initialize();
	Application master;
	Application slave;
	ASSERT_TRUE(master.StartHttpServer(8080 + rand() % 10000));
	ASSERT_TRUE(slave.StartHttpServer( 8080 + rand() % 10000));

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 55; time <= 3 * 60; time += 70 + rand() % 40)
	{
		auto rule_set = std::make_shared<RuleSet>("Test" + std::to_string(time), 10, 10, 30, 20, false, false, false, time);
		master.GetDatabase().AddRuleSet(rule_set);

		Judoka* j1 = new Judoka("Needs", "Break");
		Judoka* j2 = new Judoka("White", "LastnameW");
		Judoka* j3 = new Judoka("Blue",  "LastnameB");
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);
		master.GetDatabase().AddJudoka(j3);

		Match* match = new Match(j1, j2, nullptr);
		match->SetMatID(1);
		match->SetRuleSet(rule_set);
		master.GetTournament()->AddMatch(match);

		EXPECT_TRUE(m->StartMatch(match));
		ZED::Core::Pause(100);

		m->Hajime();
		m->AddIppon((Fighter)(rand()%2));
		ZED::Core::Pause(100);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
		ZED::Core::Pause(100);

		Match* match2 = new Match(j1, j3, nullptr);
		match2->SetMatID(1);
		match2->SetRuleSet(rule_set);
		master.GetTournament()->AddMatch(match2);

		for (int k = 0; k < time-10; k++)
		{
			EXPECT_FALSE(m->StartMatch(match2));
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(10 * 1000);
		ZED::Core::Pause(5000);

		EXPECT_TRUE(m->StartMatch(match2));
		ZED::Core::Pause(200);
		m->AddIppon((Fighter)(rand() % 2));
		EXPECT_TRUE(m->EndMatch());
		ZED::Core::Pause(100);
	}
}