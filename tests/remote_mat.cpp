#include "tests.h"



TEST(RemoteMat, OpenAndClose)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

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
		Application master(8080 + rand() % 10000);
		Application slave(8080 + rand() % 10000);

		ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
		ASSERT_TRUE(slave.StartLocalMat(1));

		IMat* m = master.FindMat(1);
		ASSERT_TRUE(m);
	}
	EXPECT_LE(ZED::Core::CurrentTimestamp() - time, 2500u);
}



TEST(RemoteMat, SlaveOpensMatTwice)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(2));

	master.CloseTournament();
	master.SetTournamentList().clear();

	master.StartLocalMat(1);
	slave.StartLocalMat(2);

	IMat* mat[2];
	mat[0] = master.FindMat(1);
	mat[1] = master.FindMat(2);

	EXPECT_TRUE(mat[0]);
	EXPECT_TRUE(mat[1]);
}



TEST(RemoteMat, ForcedCloseDuringMatch)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));

	master.CloseTournament();
	master.SetTournamentList().clear();

	master.StartLocalMat(1);
	ASSERT_TRUE(slave.StartLocalMat(2));

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
	auto tourney = new Tournament(tournament_name, new RuleSet("Test", 60, 0, 20, 10));
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

	IMat* mat[2];
	mat[0] = master.FindMat(1);
	mat[1] = master.FindMat(2);

	ZED::Core::Pause(2000);

	for (int i = 0;i < 2;i++)
	{
		ASSERT_TRUE(mat[i]);

		auto match = tourney->GetNextMatch(mat[i]->GetMatID());
		EXPECT_TRUE(match);
		EXPECT_TRUE(mat[i]->StartMatch(match));

		ZED::Core::Pause(5000);

		mat[i]->Hajime();

		ZED::Core::Pause(1000);

		Fighter f = Fighter::White;
		if (rand() % 2 == 0)
			f = Fighter::Blue;

		mat[i]->AddIppon(f);

		ZED::Core::Pause(3000);

		EXPECT_TRUE(mat[i]->EndMatch());
		ZED::Core::Pause(3000);
	}
}



TEST(RemoteMat, StartMatch)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	master.GetDatabase().AddJudoka(j1);
	master.GetDatabase().AddJudoka(j2);

	Match match(nullptr, j1, j2);
	match.SetMatID(1);
	EXPECT_TRUE(m->StartMatch(&match));

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 0);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 0);
		EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == 0);
		EXPECT_TRUE(m->GetScoreboard(f).m_Koka == 0);

		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 0);
		EXPECT_FALSE(m->GetScoreboard(f).m_HansokuMake);
		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 0);

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	auto rules = new RuleSet("Test", 5, 0, 30, 20, true, true, true, 0);

	master.GetDatabase().AddJudoka(j1);
	master.GetDatabase().AddJudoka(j2);
	master.GetDatabase().AddRuleSet(rules);

	Match match(nullptr, j1, j2);
	match.SetMatID(1);
	match.SetRuleSet(rules);

	ASSERT_TRUE(m);
	ASSERT_TRUE(m->StartMatch(&match));		

	auto remote_rules = slave.GetMats()[0]->GetMatch()->GetRuleSet();
	EXPECT_EQ(remote_rules.GetName(), "Test");
	EXPECT_EQ(remote_rules.GetMatchTime(), 5);
}



TEST(RemoteMat, SendHajime)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	auto j1 = new Judoka(GetRandomName(), GetRandomName());
	auto j2 = new Judoka(GetRandomName(), GetRandomName());
	auto rules = new RuleSet("Test", 5, 0, 30, 20, true, true, true, 0);

	master.GetDatabase().AddJudoka(j1);
	master.GetDatabase().AddJudoka(j2);
	master.GetDatabase().AddRuleSet(rules);

	Match match(nullptr, j1, j2);
	match.SetMatID(1);
	match.SetRuleSet(rules);

	ASSERT_TRUE(m);
	ASSERT_TRUE(m->StartMatch(&match));
	m->Hajime();

	auto remote_mat = slave.GetMats()[0];	
	EXPECT_TRUE(remote_mat->IsHajime());
}



TEST(RemoteMat, CorrectWinner)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);
	auto rules = new RuleSet("Test", 5, 0, 30, 20, true, true, true, 0);
	master.GetDatabase().AddRuleSet(rules);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		for (int i = 0; i <= 5; i++)
		{
			auto j1 = new Judoka(GetRandomName(), GetRandomName());
			auto j2 = new Judoka(GetRandomName(), GetRandomName());
			master.GetDatabase().AddJudoka(j1);
			master.GetDatabase().AddJudoka(j2);

			Match* match = new Match(nullptr, j1, j2);
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
				m->AddNotDisqualification(!f);
			}

			ZED::Core::Pause(5500);

			if (i == 4)
				m->SetAsDraw();

			EXPECT_TRUE(m->EndMatch());

			ZED::Core::Pause(500);

			if (i == 4)
				EXPECT_EQ(match->GetMatchResult().m_Winner, Winner::Draw);
			else
				EXPECT_EQ(match->GetMatchResult().m_Winner, Fighter2Winner(f));
		}
	}
}



TEST(RemoteMat, ForceClose)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	Judoka j1("White", "LastnameW");
	Judoka j2("Blue",  "LastnameB");
	master.GetDatabase().AddJudoka(&j1);
	master.GetDatabase().AddJudoka(&j2);

	ZED::Core::Pause(500);

	Match match(nullptr, &j1, &j2);
	match.SetMatID(1);
	EXPECT_TRUE(m->StartMatch(&match));
}



TEST(RemoteMat, RemoveIpponShouldRecoverPreviousWazaari)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

		m->AddWazaAri(f);
		m->AddIppon(f);

		m->RemoveIppon(f);

		EXPECT_FALSE(m->HasConcluded());

		EXPECT_FALSE(m->IsHajime());

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon   == 0);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 1);

		m->AddIppon(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, RemoveWazariShouldRemoveIppon)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 0);
		m->AddIppon(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 1);
		m->RemoveIppon(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 0);

				
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 0);
		m->AddWazaAri(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 1);
		m->RemoveWazaAri(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 0);


		m->AddIppon(f);
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Shido)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));


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
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 3);
		EXPECT_TRUE(m->GetScoreboard(f).m_HansokuMake);

		m->RemoveShido(f);

		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 2);
		EXPECT_FALSE(m->GetScoreboard(f).m_HansokuMake);

		m->AddShido(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Shido == 3);
		EXPECT_TRUE(m->GetScoreboard(f).m_HansokuMake);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, ShidoDoesntEndGoldenScore)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 5, 60, 30, 20, false, false, true, 0));
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();

		ZED::Core::Pause(6 * 1000);
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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		for (int i = 0; i <= 3; i++)
		{
			Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 5, 60, 30, 20, true, true, true, 0));

			EXPECT_TRUE(m->StartMatch(&match));
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
			EXPECT_EQ(match.GetMatchResult().m_Winner, Fighter2Winner(f));
		}
	}
}



TEST(RemoteMat, ShidosResultInIndirectHansokumake)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

		
		m->AddShido(f);
		m->AddShido(f);
		m->AddShido(f);		

		EXPECT_TRUE(m->EndMatch());

		auto& events = match.GetLog().GetEvents();

		EXPECT_TRUE(events[events.size() - 3].m_BiasedEvent == MatchLog::BiasedEvent::AddHansokuMake_Indirect);
		EXPECT_TRUE(events[events.size() - 3].m_Group == f);

		EXPECT_TRUE(events[events.size() - 2].m_BiasedEvent == MatchLog::BiasedEvent::AddIppon);
		EXPECT_TRUE(events[events.size() - 2].m_Group == !f);
	}
}



TEST(RemoteMat, HansokumakeResultsInDirectHansokumake)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));


		m->AddHansokuMake(f);
		m->AddDisqualification(f);

		EXPECT_TRUE(m->EndMatch());

		auto& events = match.GetLog().GetEvents();

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));


		m->AddHansokuMake(f);

		EXPECT_FALSE(m->HasConcluded());
		EXPECT_FALSE(m->EndMatch());
	}
}



TEST(RemoteMat, DirectHansokumakeAndDisqDoesConcludeMatch)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));


		m->AddHansokuMake(f);
		m->AddDisqualification(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Gachi)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

		m->AddGachi(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, GachiResultsInMate)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

		m->AddIppon(f);
		m->AddIppon(!f);

		EXPECT_FALSE(m->HasConcluded());
		m->EnableGoldenScore();

		EXPECT_TRUE(m->IsGoldenScore());

		EXPECT_FALSE(m->IsHajime());

		m->AddIppon(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, DoubleIpponFightersKeepWazaari)
{
	//IJF 2018: In the case where both contestants score ippon simultaneously during the time
	//allotted for regular time, the contest shall be decided by a “golden score” period
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 1, 60, 30, 20, false, false, false, 0));

		EXPECT_TRUE(m->StartMatch(&match));
		
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
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 1, 60, 30, 20, false, false, false, 0));

		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();
		m->AddWazaAri(f);
		m->AddWazaAri(!f);
		ZED::Core::Pause(2000);

		EXPECT_TRUE(m->EnableGoldenScore());
		EXPECT_TRUE(m->IsGoldenScore());

		m->Hajime();
		m->AddIppon(f);
		m->AddIppon(!f);

		EXPECT_FALSE(m->IsHajime());
		EXPECT_FALSE(m->HasConcluded());

		EXPECT_TRUE(m->GetScoreboard(Fighter::White).m_WazaAri == 1);
		EXPECT_TRUE(m->GetScoreboard(Fighter::Blue ).m_WazaAri == 1);

		m->Hajime();
		m->AddIppon(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, IpponResultsInMate)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

		m->AddIppon(f);

		EXPECT_FALSE(m->IsHajime());

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, HansokumakeResultsInMate)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

		m->AddHansokuMake(f);

		EXPECT_FALSE(m->IsHajime());

		m->AddNotDisqualification(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, ThirdShidoIsHansokumake)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));


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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	//IJF rules 2018:
	//In regular time or golden score if both athletes receive three shido, both athletes
	//will be considered losers, the contest result will be recorded as 0 - 0

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));


		m->AddShido(f);
		m->AddShido(f);
		m->AddShido(!f);
		m->AddShido(!f);

		m->AddShido(f);
		m->AddShido(!f);

		EXPECT_TRUE(m->GetScoreboard(f).m_HansokuMake);
		EXPECT_TRUE(m->GetScoreboard(!f).m_HansokuMake);

		m->AddNotDisqualification(f);
		m->AddNotDisqualification(!f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		EXPECT_TRUE(match.GetMatchResult().m_Winner == Judoboard::Winner::Draw);
		EXPECT_TRUE((int)match.GetMatchResult().m_Score == 0);
	}
}



TEST(RemoteMat, DoubleGachi)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	//In the case when both contestants do not show up for the match

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));


		m->AddGachi(f);
		m->AddGachi(!f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		EXPECT_TRUE(match.GetMatchResult().m_Winner == Judoboard::Winner::Draw);
		EXPECT_TRUE((int)match.GetMatchResult().m_Score == 0);
	}
}



TEST(RemoteMat, Hansokumake)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));


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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));


		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 0);
		m->AddMedicalExamination(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 1);
		m->RemoveMedicalExamination(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 0);
		m->AddMedicalExamination(f);


		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 1);
		m->AddMedicalExamination(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 2);
		m->RemoveMedicalExamination(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 1);
		m->AddMedicalExamination(f);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 2);
		m->AddMedicalExamination(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 2);
		m->RemoveMedicalExamination(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 1);
		m->AddMedicalExamination(f);

		EXPECT_TRUE(m->GetScoreboard(f).m_MedicalExamination == 2);

		EXPECT_FALSE(m->EndMatch());
	}
}



TEST(RemoteMat, MatchTime)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 120; time <= 5*60 ; time += 80 + rand()%60)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", time, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();

		for (int k = 0; k < time - 1; k++)
		{
			EXPECT_FALSE(m->IsOutOfTime());
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(2000);
		EXPECT_TRUE(m->IsOutOfTime());
	}
}



TEST(RemoteMat, GoldenScoreTime)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 30; time <= 3 * 60; time += 60 + rand() % 50)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 5, time, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();

		ZED::Core::Pause(6 * 1000);
		m->EnableGoldenScore();
		m->Hajime();

		for (int k = 0; k < time - 1; k++)
		{
			EXPECT_FALSE(m->IsOutOfTime());
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(2000);
		EXPECT_TRUE(m->IsOutOfTime());
	}
}



TEST(RemoteMat, OsaekomiTime)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 1; time <= 30; time += 6 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 100, 0, time, 20, false, false, false, 0));
			EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 1; time <= 25; time += 5 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 100, 0, 100, time, false, false, false, 0));
			EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);
	
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 100, 0, 100, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();
		m->Osaekomi(f);

		ZED::Core::Pause(5000);

		m->AddIppon(!f);

		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		EXPECT_TRUE(match.GetMatchResult().m_Winner == !f);
		EXPECT_TRUE(match.GetMatchResult().m_Score  == Match::Score::Ippon);
	}
}



TEST(RemoteMat, OsaekomiToriGivesUp)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 100, 0, 100, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();
		m->Osaekomi(f);

		ZED::Core::Pause(5000);

		m->AddGachi(f);

		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		EXPECT_TRUE(match.GetMatchResult().m_Winner == !f);
		EXPECT_TRUE(match.GetMatchResult().m_Score  == Match::Score::Ippon);
	}
}



TEST(RemoteMat, OsaekomiWithWazaAriRemoved)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 1; time <= 25; time += 5 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 100, 0, time*2, time, false, false, false, 0));
			EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

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
			Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 5, 60, 2*time, time, false, false, false, 0));
			EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 500, 0, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 500, 0, 20, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();
		m->Osaekomi(f);

		ZED::Core::Pause(5000);
		m->Tokeda();
		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_FALSE(m->IsOsaekomiRunning());

		EXPECT_TRUE(m->GetOsaekomiList().size() == 1);
		EXPECT_TRUE(m->GetOsaekomiList()[0].m_Who == f);
		EXPECT_TRUE(std::abs((int)m->GetOsaekomiList()[0].m_Time - 5000) < 20);

		ZED::Core::Pause(5000);
		m->Osaekomi(f);
		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());

		ZED::Core::Pause(5000);
		m->Tokeda();
		EXPECT_FALSE(m->IsOsaekomi());
		EXPECT_FALSE(m->IsOsaekomiRunning());

		EXPECT_TRUE(m->GetOsaekomiList().size() == 2);
		EXPECT_TRUE(m->GetOsaekomiList()[1].m_Who == f);
		EXPECT_TRUE(std::abs((int)m->GetOsaekomiList()[1].m_Time - 5000) < 20);

		ZED::Core::Pause(10 * 1000);

		m->Osaekomi(f);
		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());
		ZED::Core::Pause(15 * 1000);

		EXPECT_FALSE(m->HasConcluded());

		ZED::Core::Pause(6 * 1000);

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 1);
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, OsaekomiSwitch)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 500, 0, 25, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();
		m->Osaekomi(f);

		ZED::Core::Pause(10 * 1000);
		m->Osaekomi(!f);
		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());

		ZED::Core::Pause(10 * 1000);
		m->Osaekomi(f);
		EXPECT_TRUE(m->IsOsaekomi());
		EXPECT_TRUE(m->IsOsaekomiRunning());

		ZED::Core::Pause(10 * 1000);

		EXPECT_FALSE(m->IsOsaekomiRunning());

		EXPECT_TRUE(m->GetOsaekomiList().size() == 1);
		EXPECT_TRUE(m->GetOsaekomiList()[0].m_Who == f);

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 1);
		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());
	}
}


TEST(RemoteMat, MatchContinuesDuringOsaekomi)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 10, 0, 10, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();
		ZED::Core::Pause(8 * 1000);
		m->Osaekomi(f);

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, true, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));


		EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == 0);
		for (int k = 1; k < 25; k++)
		{
			m->AddYuko(f);
			EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == k);
		}
		for (int k = 1; k < 25; k++)
			m->RemoveYuko(f);
		EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == 0);

		m->AddIppon(f);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, Yuko2)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));


		EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == 0);
		for (int k = 1; k < 25; k++)
		{
			m->AddYuko(f);
			EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == 0);
		}
		for (int k = 1; k < 25; k++)
		{
			m->RemoveYuko(f);
			EXPECT_TRUE(m->GetScoreboard(f).m_Yuko == 0);
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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_TRUE(m->Open());

		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

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
	//or to discuss somethiong with the other judges in give tori a shido

	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter osaekomi_holder = Fighter::White; osaekomi_holder <= Fighter::Blue; osaekomi_holder++)
	{
		EXPECT_TRUE(m->Open());

		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter osaekomi_holder = Fighter::White; osaekomi_holder <= Fighter::Blue; osaekomi_holder++)
	{
		for (Fighter hansokumake_committer = Fighter::White; hansokumake_committer <= Fighter::Blue; hansokumake_committer++)
		{
			EXPECT_TRUE(m->Open());

			Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0));
			EXPECT_TRUE(m->StartMatch(&match));

			m->Hajime();
			m->Osaekomi(osaekomi_holder);

			ZED::Core::Pause(3000);
			m->Sonomama();
			ZED::Core::Pause(5000);
			m->Mate();
			ZED::Core::Pause(3000);
			m->AddHansokuMake(hansokumake_committer);
			m->AddNotDisqualification(hansokumake_committer);
			ZED::Core::Pause(3000);

			EXPECT_FALSE(m->IsHajime());
			EXPECT_FALSE(m->IsOsaekomi());
			EXPECT_FALSE(m->IsOsaekomiRunning());
			EXPECT_TRUE(m->GetScoreboard(hansokumake_committer).m_HansokuMake);


			EXPECT_TRUE(m->EndMatch());
			EXPECT_TRUE(match.GetMatchResult().m_Winner == !hansokumake_committer);
			EXPECT_TRUE(match.GetMatchResult().m_Score  == Match::Score::Ippon);
		}
	}
}



TEST(RemoteMat, Koka)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = new RuleSet("Test", 60, 60, 30, 20, false, true, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(nullptr, j1, j2);
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
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		auto j1 = new Judoka(GetRandomName(), GetRandomName());
		auto j2 = new Judoka(GetRandomName(), GetRandomName());
		master.GetDatabase().AddJudoka(j1);
		master.GetDatabase().AddJudoka(j2);

		auto rules = new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0);
		master.GetDatabase().AddRuleSet(rules);

		Match* match = new Match(nullptr, j1, j2);
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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		EXPECT_TRUE(m->StartMatch(&match));

			
		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 0);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 0);
		EXPECT_FALSE(m->EndMatch());

		m->AddWazaAri(f);				

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 0);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 1);
		EXPECT_FALSE(m->EndMatch());

		m->AddWazaAri(f);

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 1);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 2);

		m->RemoveWazaAri(f);

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 0);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 1);

		m->AddWazaAri(f);

		EXPECT_TRUE(m->GetScoreboard(f).m_Ippon == 1);
		EXPECT_TRUE(m->GetScoreboard(f).m_WazaAri == 2);

		EXPECT_TRUE(m->EndMatch());
	}
}



TEST(RemoteMat, GoldenScore)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 10, 10, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m->StartMatch(&match));

	m->Hajime();

	ZED::Core::Pause(11 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	m->EnableGoldenScore();

	m->Hajime();

	EXPECT_FALSE(m->IsOutOfTime());

	ZED::Core::Pause(11 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());
	EXPECT_FALSE(m->EndMatch());

	m->Hantei(Fighter::White);

	EXPECT_TRUE(m->HasConcluded());
	EXPECT_TRUE(m->EndMatch());
}



TEST(RemoteMat, GoldenScore2)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 10, 0, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m->StartMatch(&match));

	m->Hajime();

	ZED::Core::Pause(11 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	EXPECT_FALSE(m->EnableGoldenScore());

	m->Hajime();

	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());
	EXPECT_FALSE(m->EndMatch());

	m->Hantei(Fighter::White);

	EXPECT_TRUE(m->HasConcluded());
	EXPECT_TRUE(m->EndMatch());
}



TEST(RemoteMat, GoldenScoreResetTime)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 5, 5, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m->StartMatch(&match));

	m->Hajime();

	ZED::Core::Pause(6 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	EXPECT_TRUE(m->EnableGoldenScore());

	EXPECT_TRUE(m->GetTimeElapsed() == 0);

	m->AddIppon(Fighter::White);

	EXPECT_TRUE(m->HasConcluded());
	EXPECT_TRUE(m->EndMatch());
}



TEST(RemoteMat, GoldenScoreKeepsShidosAndMedicalExaminations)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 5, 5, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m->StartMatch(&match));

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
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 10, 60, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m->StartMatch(&match));

	m->Hajime();

	for (int k = 0; k < 9; k++)
	{
		EXPECT_FALSE(m->IsOutOfTime());
		ZED::Core::Pause(1000);
	}

	ZED::Core::Pause(2 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	m->SetAsDraw(true);

	EXPECT_TRUE(m->HasConcluded());
	EXPECT_TRUE(m->EndMatch());
}



TEST(RemoteMat, Draw2)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 10, 60, 30, 20, false, false, false, 0));
	EXPECT_TRUE(m->StartMatch(&match));

	m->Hajime();

	for (int k = 0; k < 9; k++)
	{
		EXPECT_FALSE(m->IsOutOfTime());
		ZED::Core::Pause(1000);
	}

	ZED::Core::Pause(2 * 1000);
	EXPECT_TRUE(m->IsOutOfTime());

	EXPECT_FALSE(m->HasConcluded());

	m->SetAsDraw(true);

	EXPECT_FALSE(m->HasConcluded());
	EXPECT_FALSE(m->EndMatch());

	m->Hantei(Fighter::White);
	EXPECT_TRUE(m->EndMatch());
}



TEST(RemoteMat, Hantei)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave(8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Match match(nullptr, new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"));
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 10, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();

		ZED::Core::Pause(11 * 1000);
		EXPECT_TRUE(m->IsOutOfTime());

		EXPECT_FALSE(m->HasConcluded());

		m->Hantei(f);

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		EXPECT_TRUE(match.GetMatchResult().m_Winner == Fighter2Winner(f));
		EXPECT_TRUE(match.GetMatchResult().m_Score  == Match::Score::Hantei);
		EXPECT_TRUE(match.GetMatchResult().m_Score  == (Match::Score)1);
	}
}



TEST(RemoteMat, BreakTime)
{
	initialize();
	Application master(8080 + rand() % 10000);
	Application slave( 8080 + rand() % 10000);

	ASSERT_TRUE(slave.ConnectToMaster("127.0.0.1", master.GetPort()));
	ASSERT_TRUE(slave.StartLocalMat(1));

	IMat* m = master.FindMat(1);

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 55; time <= 3 * 60; time += 60 + rand() % 60)
	{
		RuleSet* rule_set = new RuleSet("Test", 10, 10, 30, 20, false, false, false, time);

		Judoka* j1 = new Judoka("Needs", "Break");
		Judoka* j2 = new Judoka("White", "LastnameW");
		Judoka* j3 = new Judoka("Blue",  "LastnameB");

		Match match(nullptr, j1, j2);
		match.SetMatID(1);
		match.SetRuleSet(rule_set);
		EXPECT_TRUE(m->StartMatch(&match));

		m->Hajime();
		m->AddIppon((Fighter)(rand()%2));

		EXPECT_TRUE(m->HasConcluded());
		EXPECT_TRUE(m->EndMatch());

		Match match2(nullptr, j1, j3, 1);
		match2.SetRuleSet(rule_set);

		for (int k = 0; k < time-1; k++)
		{
			EXPECT_FALSE(m->StartMatch(&match2));
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(2000);

		EXPECT_TRUE(m->StartMatch(&match2));
	}
}