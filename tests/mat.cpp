#include "tests.h"



TEST(Mat, OpenAndClose)
{
	initialize();
	Application app;
	Mat m(1);

	EXPECT_TRUE(m.GetType() == IMat::Type::LocalMat);
	EXPECT_TRUE(m.IsOpen());

	EXPECT_TRUE(m.Close());

	EXPECT_FALSE(m.IsOpen());

	m.Open();

	EXPECT_TRUE(m.IsOpen());

	EXPECT_TRUE(m.Close());

	m.Open();

	EXPECT_TRUE(m.IsOpen());

	EXPECT_FALSE(m.HasConcluded());
	EXPECT_TRUE(m.GetOsaekomiList().size() == 0);
	//EXPECT_GE(m.RequestScreenshot().GetSize(), 1024);
}



TEST(Mat, QuickClose)
{
	initialize();
	DWORD time = ZED::Core::CurrentTimestamp();
	{
		Application app;
		Mat m(1);
	}
	EXPECT_TRUE(ZED::Core::CurrentTimestamp() - time < 1500);
}



TEST(Mat, ForcedCloseDuringMatch)
{
	initialize();
	Application app;

	app.CloseTournament();
	app.SetTournamentList().clear();

	app.StartLocalMat(1);

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
	auto tourney = new Tournament(tournament_name, new RuleSet("Test", 60, 0, 20, 10));
	tourney->EnableAutoSave(false);
	EXPECT_TRUE(app.AddTournament(tourney));

	ASSERT_TRUE(app.FindTournamentByName(tournament_name));
	EXPECT_TRUE(app.OpenTournament(app.FindTournamentByName(tournament_name)->GetUUID()));

	tourney->AddParticipant(&j1);
	tourney->AddParticipant(&j2);
	tourney->AddParticipant(&j3);
	tourney->AddParticipant(&j4);
	tourney->AddParticipant(&j5);
	tourney->AddParticipant(&j6);

	MatchTable* m1 = new Weightclass(0, 49);
	MatchTable* m2 = new Weightclass(50, 100);
	m1->SetMatID(1);
	m2->SetMatID(1);
	tourney->AddMatchTable(m1);
	tourney->AddMatchTable(m2);

	auto mat = app.GetDefaultMat();

	ZED::Core::Pause(6000);
	auto match = tourney->GetNextMatch(mat->GetMatID());
	EXPECT_TRUE(match != nullptr);
	EXPECT_TRUE(mat->StartMatch(match));

	ZED::Core::Pause(8000);

	mat->Hajime();

	ZED::Core::Pause(1000);

	Fighter f = Fighter::White;
	if (rand() % 2 == 0)
		f = Fighter::Blue;

	mat->AddIppon(f);

	ZED::Core::Pause(3000);

	EXPECT_TRUE(mat->EndMatch());
	ZED::Core::Pause(8000);
}



TEST(Mat, StartMatch)
{
	initialize();
	Application app;
	Mat m(1);

	Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
	match.SetMatID(1);
	EXPECT_TRUE(m.StartMatch(&match));

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		EXPECT_EQ(m.GetScoreboard(f).m_Ippon, 0);
		EXPECT_EQ(m.GetScoreboard(f).m_WazaAri, 0);
		EXPECT_EQ(m.GetScoreboard(f).m_Yuko, 0);
		EXPECT_EQ(m.GetScoreboard(f).m_Koka, 0);

		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 0);
		EXPECT_FALSE(m.GetScoreboard(f).m_HansokuMake);
		EXPECT_EQ(m.GetScoreboard(f).m_MedicalExamination, 0);

		EXPECT_FALSE(m.GetScoreboard(f).m_Hantei);
	}

	EXPECT_FALSE(m.HasConcluded());
	EXPECT_FALSE(m.EndMatch());
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.HasConcluded());
	EXPECT_TRUE(m.EndMatch());
	EXPECT_FALSE(m.HasConcluded());
}



TEST(Mat, CorrectWinner)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		for (int i = 0; i <= 5; i++)
		{
			Application app;
			Mat m(1);

			Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 2, 0, 30, 20, true, true, true, 0));

			EXPECT_TRUE(m.StartMatch(&match));
			m.Hajime();
					
			if (i == 0)
				m.AddIppon(f);
			else if (i == 1)
				m.AddWazaAri(f);
			else if (i == 2)
				m.AddYuko(f);
			else if (i == 3)
				m.AddKoka(f);
			else if (i == 4)
				m.AddShido(!f);
			else
			{
				m.AddHansokuMake(!f);
				m.AddNoDisqualification(!f);
			}

			ZED::Core::Pause(2500);

			if (i == 4)
				m.SetAsDraw();

			EXPECT_TRUE(m.EndMatch());

			if (i == 4)
				EXPECT_EQ(match.GetResult().m_Winner, Winner::Draw);
			else
				EXPECT_EQ(match.GetResult().m_Winner, Fighter2Winner(f));
		}
	}
}



TEST(Mat, ForceClose)
{
	initialize();
	Application app;
	Mat m(1);

	ZED::Core::Pause(500);

	Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
	match.SetMatID(1);
	EXPECT_TRUE(m.StartMatch(&match));
}



TEST(Mat, RemoveIpponShouldRecoverPreviousWazaari)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		m.AddWazaAri(f);
		m.AddIppon(f);

		m.RemoveIppon(f);

		EXPECT_FALSE(m.HasConcluded());

		EXPECT_FALSE(m.IsHajime());

		EXPECT_EQ(m.GetScoreboard(f).m_Ippon,   0);
		EXPECT_EQ(m.GetScoreboard(f).m_WazaAri, 1);

		m.AddIppon(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, RemoveWazariShouldRemoveIppon)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		m.AddWazaAri(f);
		m.AddWazaAri(f);

		m.RemoveWazaAri(f);

		EXPECT_FALSE(m.HasConcluded());

		EXPECT_FALSE(m.IsHajime());

		EXPECT_EQ(m.GetScoreboard(f).m_Ippon,   0);
		EXPECT_EQ(m.GetScoreboard(f).m_WazaAri, 1);

		m.AddIppon(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, Scores)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		EXPECT_EQ(m.GetScoreboard(f).m_Ippon, 0);
		m.AddIppon(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Ippon, 1);
		m.RemoveIppon(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Ippon, 0);

				
		EXPECT_EQ(m.GetScoreboard(f).m_WazaAri, 0);
		m.AddWazaAri(f);
		EXPECT_EQ(m.GetScoreboard(f).m_WazaAri, 1);
		m.RemoveWazaAri(f);
		EXPECT_EQ(m.GetScoreboard(f).m_WazaAri, 0);


		m.AddIppon(f);
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, Shido)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 0);
		m.AddShido(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 1);
		m.RemoveShido(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 0);
		m.AddShido(f);


		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 1);
		m.AddShido(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 2);
		m.RemoveShido(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 1);
		m.AddShido(f);

		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 2);
		m.AddShido(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 3);
		m.RemoveShido(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 2);
		m.AddShido(f);

		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 3);
		m.AddShido(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 3);
		EXPECT_TRUE(m.GetScoreboard(f).m_HansokuMake);

		m.RemoveShido(f);

		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 2);
		EXPECT_FALSE(m.GetScoreboard(f).m_HansokuMake);

		m.AddShido(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Shido, 3);
		EXPECT_TRUE(m.GetScoreboard(f).m_HansokuMake);

		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, ShidoDoesntEndGoldenScore)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 2, 60, 30, 20, false, false, true, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();

		ZED::Core::Pause(2500);
		EXPECT_TRUE(m.IsOutOfTime());

		EXPECT_FALSE(m.HasConcluded());

		m.EnableGoldenScore();

		m.Hajime();
		m.AddShido(f);

		EXPECT_TRUE(m.IsHajime());
		EXPECT_FALSE(m.HasConcluded());

		m.AddIppon(f);

		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, ScoreEndsGoldenScore)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		for (int i = 0; i <= 3; i++)
		{
			Application app;
			Mat m(1);

			Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 2, 60, 30, 20, true, true, true, 0));

			EXPECT_TRUE(m.StartMatch(&match));
			m.Hajime();

			ZED::Core::Pause(2500);

			EXPECT_TRUE(m.EnableGoldenScore());
			m.Hajime();

			if (i == 0)
				m.AddIppon(f);
			else if (i == 1)
				m.AddWazaAri(f);
			else if (i == 2)
				m.AddYuko(f);
			else
				m.AddKoka(f);

			EXPECT_FALSE(m.IsHajime());
			EXPECT_TRUE(m.EndMatch());
			EXPECT_EQ(match.GetResult().m_Winner, Fighter2Winner(f));
		}
	}
}



TEST(Mat, ShidosResultInIndirectHansokumake)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		
		m.AddShido(f);
		m.AddShido(f);
		m.AddShido(f);		

		EXPECT_TRUE(m.EndMatch());

		auto& events = match.GetLog().GetEvents();

		EXPECT_TRUE(events[events.size() - 3].m_BiasedEvent == MatchLog::BiasedEvent::AddHansokuMake_Indirect);
		EXPECT_EQ(events[events.size() - 3].m_Group, f);

		EXPECT_TRUE(events[events.size() - 2].m_BiasedEvent == MatchLog::BiasedEvent::AddIppon);
		EXPECT_EQ(events[events.size() - 2].m_Group, !f);
	}
}



TEST(Mat, HansokumakeResultsInDirectHansokumake)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		m.AddHansokuMake(f);
		m.AddDisqualification(f);

		EXPECT_TRUE(m.EndMatch());

		auto& events = match.GetLog().GetEvents();

		EXPECT_TRUE(events[events.size() - 4].m_BiasedEvent == MatchLog::BiasedEvent::AddHansokuMake_Direct);
		EXPECT_EQ(events[events.size() - 4].m_Group, f);

		EXPECT_TRUE(events[events.size() - 3].m_BiasedEvent == MatchLog::BiasedEvent::AddIppon);
		EXPECT_EQ(events[events.size() - 3].m_Group, !f);

		EXPECT_TRUE(events[events.size() - 2].m_BiasedEvent == MatchLog::BiasedEvent::AddDisqualification);
		EXPECT_EQ(events[events.size() - 2].m_Group, f);
	}
}



TEST(Mat, DirectHansokumakeDoesNotConcludeMatch)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		m.AddHansokuMake(f);

		EXPECT_FALSE(m.HasConcluded());
		EXPECT_FALSE(m.EndMatch());
	}
}



TEST(Mat, DirectHansokumakeAndDisqDoesConcludeMatch)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		m.AddHansokuMake(f);
		m.AddDisqualification(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, DisqualificationCanBeRemoved)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		m.AddHansokuMake(f);
		EXPECT_FALSE(m.GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(m.GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(m.GetScoreboard(f).IsUnknownDisqualification());

		m.AddDisqualification(f);
		EXPECT_TRUE(m.GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(m.GetScoreboard(f).IsNotDisqualified());
		EXPECT_FALSE(m.GetScoreboard(f).IsUnknownDisqualification());

		m.AddNoDisqualification(f);
		EXPECT_FALSE(m.GetScoreboard(f).IsDisqualified());
		EXPECT_TRUE(m.GetScoreboard(f).IsNotDisqualified());
		EXPECT_FALSE(m.GetScoreboard(f).IsUnknownDisqualification());

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, Gachi)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		m.AddGachi(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, GachiResultsInMate)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		m.AddGachi(f);

		EXPECT_FALSE(m.IsHajime());

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, DoubleIppon)
{
	//IJF 2018: In the case where both contestants score ippon simultaneously during the time
	//allotted for regular time, the contest shall be decided by a “golden score” period
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		m.AddIppon(f);
		m.AddIppon(!f);

		EXPECT_FALSE(m.HasConcluded());
		m.EnableGoldenScore();

		EXPECT_TRUE(m.IsGoldenScore());

		EXPECT_FALSE(m.IsHajime());

		m.AddIppon(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, DoubleIpponFightersKeepWazaari)
{
	//IJF 2018: In the case where both contestants score ippon simultaneously during the time
	//allotted for regular time, the contest shall be decided by a “golden score” period
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		m.AddWazaAri(f);
		m.AddWazaAri(!f);

		m.AddIppon(f);
		m.AddIppon(!f);

		EXPECT_FALSE(m.HasConcluded());
		m.EnableGoldenScore();

		EXPECT_TRUE(m.IsGoldenScore());

		EXPECT_FALSE(m.IsHajime());

		EXPECT_EQ(m.GetScoreboard(Fighter::White).m_WazaAri, 1);
		EXPECT_EQ(m.GetScoreboard(Fighter::Blue ).m_WazaAri, 1);

		m.AddIppon(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, DoubleIpponDuringGoldenScore)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 1, 60, 30, 20, false, false, false, 0));

		EXPECT_TRUE(m.StartMatch(&match));
		
		m.Hajime();
		ZED::Core::Pause(2000);

		EXPECT_TRUE(m.EnableGoldenScore());
		EXPECT_TRUE(m.IsGoldenScore());

		m.Hajime();
		m.AddIppon(f);
		m.AddIppon(!f);

		EXPECT_FALSE(m.IsHajime());
		EXPECT_FALSE(m.HasConcluded());

		m.Hajime();
		m.AddIppon(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, DoubleIpponDuringGoldenScoreFightersKeepWazaari)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 1, 60, 30, 20, false, false, false, 0));

		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.AddWazaAri(f);
		m.AddWazaAri(!f);
		ZED::Core::Pause(2000);

		EXPECT_TRUE(m.EnableGoldenScore());
		EXPECT_TRUE(m.IsGoldenScore());

		m.Hajime();
		m.AddIppon(f);
		m.AddIppon(!f);

		EXPECT_FALSE(m.IsHajime());
		EXPECT_FALSE(m.HasConcluded());

		EXPECT_TRUE(m.GetScoreboard(Fighter::White).m_WazaAri == 1);
		EXPECT_TRUE(m.GetScoreboard(Fighter::Blue ).m_WazaAri == 1);

		m.Hajime();
		m.AddIppon(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, IpponResultsInMate)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		m.AddIppon(f);

		EXPECT_FALSE(m.IsHajime());

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, HansokumakeResultsInMate)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

		m.AddHansokuMake(f);

		EXPECT_FALSE(m.IsHajime());

		m.AddNoDisqualification(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, ThirdShidoIsHansokumake)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		EXPECT_TRUE(m.Open());

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		m.AddShido(f);
		m.AddShido(f);
		m.AddShido(f);

		EXPECT_TRUE(m.GetScoreboard(f).m_HansokuMake);				

		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, DoubleHansokumake)
{
	initialize();

	//IJF rules 2018:
	//In regular time or golden score if both athletes receive three shido, both athletes
	//will be considered losers, the contest result will be recorded as 0 - 0

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		EXPECT_TRUE(m.Open());

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		m.AddShido(f);
		m.AddShido(f);
		m.AddShido(!f);
		m.AddShido(!f);

		m.AddShido(f);
		m.AddShido(!f);

		EXPECT_TRUE(m.GetScoreboard(f).m_HansokuMake);
		EXPECT_TRUE(m.GetScoreboard(!f).m_HansokuMake);

		m.AddNoDisqualification(f);
		m.AddNoDisqualification(!f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());

		EXPECT_EQ(match.GetResult().m_Winner, Judoboard::Winner::Draw);
		EXPECT_EQ((int)match.GetResult().m_Score, 0);
	}
}



TEST(Mat, DoubleGachi)
{
	initialize();

	//In the case when both contestants do not show up for the match

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		EXPECT_TRUE(m.Open());

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		m.AddGachi(f);
		m.AddGachi(!f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());

		EXPECT_TRUE(match.GetResult().m_Winner == Judoboard::Winner::Draw);
		EXPECT_TRUE((int)match.GetResult().m_Score == 0);
	}
}



TEST(Mat, Hansokumake)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		EXPECT_FALSE(m.GetScoreboard(f).m_HansokuMake);
		m.AddHansokuMake(f);
		EXPECT_TRUE(m.GetScoreboard(f).m_HansokuMake);
		m.AddDisqualification(f);
		EXPECT_TRUE(m.HasConcluded());

		m.RemoveHansokuMake(f);
		EXPECT_FALSE(m.GetScoreboard(f).m_HansokuMake);

		m.RemoveIppon(!f);
		EXPECT_FALSE(m.HasConcluded());

		EXPECT_FALSE(m.EndMatch());

		m.AddHansokuMake(f);
		EXPECT_TRUE(m.GetScoreboard(f).m_HansokuMake);

		m.AddDisqualification(f);
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, MedicalExaminiations)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));


		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 0);
		m.AddMedicalExamination(f);
		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 1);
		m.RemoveMedicalExamination(f);
		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 0);
		m.AddMedicalExamination(f);


		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 1);
		m.AddMedicalExamination(f);
		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 2);
		m.RemoveMedicalExamination(f);
		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 1);
		m.AddMedicalExamination(f);

		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 2);
		m.AddMedicalExamination(f);
		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 2);
		m.RemoveMedicalExamination(f);
		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 1);
		m.AddMedicalExamination(f);

		EXPECT_TRUE(m.GetScoreboard(f).m_MedicalExamination == 2);

		EXPECT_FALSE(m.EndMatch());
	}
}



TEST(Mat, MatchTime)
{
	initialize();
	srand(ZED::Core::CurrentTimestamp());
	for (int time = 120; time <= 4*60 + 30 ; time += 90 + rand()%60)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", time, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();

		for (int k = 0; k < time - 1; k++)
		{
			EXPECT_FALSE(m.IsOutOfTime());
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(2000);
		EXPECT_TRUE(m.IsOutOfTime());
	}
}



TEST(Mat, GoldenScoreTime)
{
	initialize();
	srand(ZED::Core::CurrentTimestamp());
	for (int time = 30; time <= 3 * 60; time += 65 + rand() % 50)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 2, time, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();

		ZED::Core::Pause(3000);
		m.EnableGoldenScore();
		m.Hajime();

		for (int k = 0; k < time - 1; k++)
		{
			EXPECT_FALSE(m.IsOutOfTime());
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(2000);
		EXPECT_TRUE(m.IsOutOfTime());
	}
}



TEST(Mat, OsaekomiTime)
{
	initialize();
	srand(ZED::Core::CurrentTimestamp());
	for (int time = 5; time <= 30; time += 6 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			Application app;
			Mat m(1);

			Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 100, 0, time, 20, false, false, false, 0));
			EXPECT_TRUE(m.StartMatch(&match));

			m.Hajime();
			m.Osaekomi(f);

			for (int k = 0; k < time - 1; k++)
			{
				EXPECT_FALSE(m.IsOutOfTime());
				EXPECT_FALSE(m.HasConcluded());
				EXPECT_FALSE(m.EndMatch());
				ZED::Core::Pause(1000);
			}

			ZED::Core::Pause(2000);
			EXPECT_TRUE(m.HasConcluded());
			EXPECT_TRUE(m.EndMatch());
		}
	}
}



TEST(Mat, OsaekomiWithWazaAriTime)
{
	initialize();
	srand(ZED::Core::CurrentTimestamp());
	for (int time = 5; time <= 25; time += 5 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			Application app;
			Mat m(1);

			Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 100, 0, 100, time, false, false, false, 0));
			EXPECT_TRUE(m.StartMatch(&match));

			m.Hajime();
			m.AddWazaAri(f);
			m.Osaekomi(f);

			for (int k = 0; k < time - 1; k++)
			{
				EXPECT_FALSE(m.IsOutOfTime());
				EXPECT_FALSE(m.HasConcluded());
				EXPECT_FALSE(m.EndMatch());
				ZED::Core::Pause(1000);
			}

			ZED::Core::Pause(2000);
			EXPECT_TRUE(m.HasConcluded());
			EXPECT_TRUE(m.EndMatch());
		}
	}
}



TEST(Mat, OsaekomiUkeGainsIppon)
{
	initialize();
	
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 100, 0, 100, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(f);

		ZED::Core::Pause(2000);

		m.AddIppon(!f);

		EXPECT_FALSE(m.IsOsaekomi());
		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());

		EXPECT_TRUE(match.GetResult().m_Winner == !f);
		EXPECT_TRUE(match.GetResult().m_Score  == Match::Score::Ippon);
	}
}



TEST(Mat, OsaekomiToriGivesUp)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 100, 0, 100, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(f);

		ZED::Core::Pause(2000);

		m.AddGachi(f);

		EXPECT_FALSE(m.IsOsaekomi());
		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());

		EXPECT_EQ(match.GetResult().m_Winner, !f);
		EXPECT_EQ(match.GetResult().m_Score, Match::Score::Ippon);
	}
}



TEST(Mat, OsaekomiWithWazaAriRemoved)
{
	initialize();
	srand(ZED::Core::CurrentTimestamp());
	for (int time = 5; time <= 25; time += 6 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			Application app;
			Mat m(1);

			Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 100, 0, time*2, time, false, false, false, 0));
			EXPECT_TRUE(m.StartMatch(&match));

			m.Hajime();
			m.AddWazaAri(f);
			m.Osaekomi(f);

			for (int k = 0; k < time - 1; k++)
			{
				EXPECT_FALSE(m.IsOutOfTime());
				EXPECT_FALSE(m.HasConcluded());
				EXPECT_FALSE(m.EndMatch());
				ZED::Core::Pause(1000);
			}

			m.RemoveWazaAri(f);

			for (int k = 0; k < time - 1; k++)
			{
				EXPECT_FALSE(m.IsOutOfTime());
				EXPECT_FALSE(m.HasConcluded());
				EXPECT_FALSE(m.EndMatch());
				ZED::Core::Pause(1000);
			}

			ZED::Core::Pause(3000);

			EXPECT_TRUE(m.HasConcluded());
			EXPECT_TRUE(m.EndMatch());
		}
	}
}



TEST(Mat, OsaekomiTillEndDuringGoldenScore)
{
	initialize();

	//Osaekomi should go the full distance even during golden score
	//and now get cancelled after reaching the wazaari time

	srand(ZED::Core::CurrentTimestamp());
	for (int time = 2; time <= 25; time += 5 + rand() % 8)
	{
		for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
		{
			Application app;
			Mat m(1);

			Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 2, 60, 2*time, time, false, false, false, 0));
			EXPECT_TRUE(m.StartMatch(&match));

			m.Hajime();
			ZED::Core::Pause(2500);

			m.EnableGoldenScore();
			m.Hajime();

			m.Osaekomi(f);

			for (int k = 0; k < 2*time - 1; k++)
			{
				EXPECT_FALSE(m.IsOutOfTime());
				EXPECT_FALSE(m.HasConcluded());
				EXPECT_FALSE(m.EndMatch());
				ZED::Core::Pause(1000);
			}

			ZED::Core::Pause(2000);
			EXPECT_TRUE(m.HasConcluded());
			EXPECT_TRUE(m.EndMatch());
		}
	}
}



TEST(Mat, Sonomama)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 500, 0, 15, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(f);

		ZED::Core::Pause(5000);
		m.Sonomama();
		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_FALSE(m.IsOsaekomiRunning());
		EXPECT_EQ(m.GetOsaekomiHolder(), f);

		ZED::Core::Pause(5000);
		m.Hajime();
		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_TRUE(m.IsOsaekomiRunning());
		EXPECT_EQ(m.GetOsaekomiHolder(), f);

		ZED::Core::Pause(5000);
		m.Sonomama();
		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_FALSE(m.IsOsaekomiRunning());
		EXPECT_EQ(m.GetOsaekomiHolder(), f);

		ZED::Core::Pause(10 * 1000);

		m.Hajime();
		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_TRUE(m.IsOsaekomiRunning());
		EXPECT_EQ(m.GetOsaekomiHolder(), f);
		ZED::Core::Pause(3000);

		EXPECT_FALSE(m.HasConcluded());

		ZED::Core::Pause(5 * 1000);

		EXPECT_EQ(m.GetScoreboard(f).m_Ippon, 1);
		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, Tokeda)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		EXPECT_TRUE(m.Open());

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 500, 0, 20, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(f);

		ZED::Core::Pause(5000);
		m.Tokeda();
		EXPECT_FALSE(m.IsOsaekomi());
		EXPECT_FALSE(m.IsOsaekomiRunning());

		ASSERT_EQ(m.GetOsaekomiList().size(), 1);
		EXPECT_EQ(m.GetOsaekomiList()[0].m_Who, f);
		EXPECT_TRUE(std::abs((int)m.GetOsaekomiList()[0].m_Time - 5000) < 20);

		ZED::Core::Pause(5000);
		m.Osaekomi(f);
		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_TRUE(m.IsOsaekomiRunning());

		ZED::Core::Pause(5000);
		m.Tokeda();
		EXPECT_FALSE(m.IsOsaekomi());
		EXPECT_FALSE(m.IsOsaekomiRunning());

		ASSERT_EQ(m.GetOsaekomiList().size(), 2);
		EXPECT_EQ(m.GetOsaekomiList()[1].m_Who, f);
		EXPECT_TRUE(std::abs((int)m.GetOsaekomiList()[1].m_Time - 5000) < 20);

		ZED::Core::Pause(10 * 1000);

		m.Osaekomi(f);
		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_TRUE(m.IsOsaekomiRunning());
		ZED::Core::Pause(15 * 1000);

		EXPECT_FALSE(m.HasConcluded());

		ZED::Core::Pause(6000);

		EXPECT_EQ(m.GetScoreboard(f).m_Ippon, 1);
		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, OsaekomiTokedaOsaekomi)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 500, 0, 10, 10, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(f);

		ZED::Core::Pause(3 * 1000);

		m.Tokeda();
		m.Osaekomi(!f);

		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_TRUE(m.IsOsaekomiRunning());

		ZED::Core::Pause(3 * 1000);
		m.Tokeda();

		EXPECT_FALSE(m.IsOsaekomiRunning());

		ASSERT_EQ(m.GetOsaekomiList().size(), 2);
		EXPECT_EQ(m.GetOsaekomiList()[0].m_Who, f);
		EXPECT_EQ(m.GetOsaekomiList()[1].m_Who, !f);

		m.AddIppon(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Ippon, 1);
		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, TokedaDuringSonomama)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 500, 0, 10, 10, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(f);

		ZED::Core::Pause(3 * 1000);

		m.Sonomama();
		m.Tokeda();
		m.Hajime();

		EXPECT_FALSE(m.IsOsaekomi());
		EXPECT_FALSE(m.IsOsaekomiRunning());

		ASSERT_EQ(m.GetOsaekomiList().size(), 1);
		EXPECT_EQ(m.GetOsaekomiList()[0].m_Who, f);

		m.AddIppon(f);
		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, OsaekomiSwitch)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 500, 0, 12, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(f);

		ZED::Core::Pause(5 * 1000);
		m.Osaekomi(!f);
		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_TRUE(m.IsOsaekomiRunning());

		ZED::Core::Pause(5 * 1000);
		m.Osaekomi(f);
		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_TRUE(m.IsOsaekomiRunning());

		ZED::Core::Pause(5 * 1000);

		EXPECT_FALSE(m.IsOsaekomiRunning());

		ASSERT_EQ(m.GetOsaekomiList().size(), 1);
		EXPECT_EQ(m.GetOsaekomiList()[0].m_Who, f);

		EXPECT_EQ(m.GetScoreboard(f).m_Ippon, 1);
		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, OsaekomiSwitchDuringSonomama)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 500, 0, 5, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(f);

		ZED::Core::Pause(3 * 1000);
		m.Sonomama();
		m.Osaekomi(!f);
		m.Hajime();
		EXPECT_TRUE(m.IsOsaekomi());
		EXPECT_TRUE(m.IsOsaekomiRunning());

		ZED::Core::Pause(5 * 1000);

		EXPECT_FALSE(m.IsOsaekomiRunning());

		ASSERT_EQ(m.GetOsaekomiList().size(), 1);
		EXPECT_EQ(m.GetOsaekomiList()[0].m_Who, !f);

		EXPECT_EQ(m.GetScoreboard(!f).m_Ippon, 1);
		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, MatchContinuesDuringOsaekomi)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 10, 0, 10, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		ZED::Core::Pause(8 * 1000);
		m.Osaekomi(f);

		for (int k = 0; k < 9; k++)
		{
			EXPECT_FALSE(m.IsOutOfTime());
			EXPECT_FALSE(m.HasConcluded());
			EXPECT_FALSE(m.EndMatch());
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(2000);
		EXPECT_EQ(m.GetScoreboard(f).m_Ippon, 1);
		EXPECT_TRUE(m.IsOutOfTime());
		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, Yuko)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, true, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));


		EXPECT_EQ(m.GetScoreboard(f).m_Yuko, 0);
		for (int k = 1; k < 25; k++)
		{
			m.AddYuko(f);
			EXPECT_EQ(m.GetScoreboard(f).m_Yuko, k);
		}
		for (int k = 1; k < 25; k++)
			m.RemoveYuko(f);
		EXPECT_EQ(m.GetScoreboard(f).m_Yuko, 0);

		m.AddIppon(f);

		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, Yuko2)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));


		EXPECT_EQ(m.GetScoreboard(f).m_Yuko, 0);
		for (int k = 1; k < 25; k++)
		{
			m.AddYuko(f);
			EXPECT_EQ(m.GetScoreboard(f).m_Yuko, 0);
		}
		for (int k = 1; k < 25; k++)
		{
			m.RemoveYuko(f);
			EXPECT_EQ(m.GetScoreboard(f).m_Yuko, 0);
		}

		m.AddIppon(f);

		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, ShidoForToriDuringOsaekomi)
{
	//IJF 2018: When osaekomi is being applied and tori commits an infringement meriting a
	//penalty(shido) :
	//If the osaekomi time is less than 10 seconds, the referee shall announce
	//mate, return the contestants to their standing positions, award the penalty then
	//recommence the contest by announcing hajime.
	
	//Is a shido is commited by Tori during osaekomi, mate is called, shido is given and then a score (if applicable)

	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		EXPECT_TRUE(m.Open());

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(f);
		ZED::Core::Pause(5000);
		m.Mate();
		ZED::Core::Pause(1000);
		m.AddShido(f);

		EXPECT_FALSE(m.IsHajime());
		EXPECT_FALSE(m.IsOsaekomi());
		EXPECT_FALSE(m.IsOsaekomiRunning());
		EXPECT_TRUE(m.GetScoreboard(f).m_Shido == 1);

		ZED::Core::Pause(5000);
		m.AddIppon(f);
				

		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, MateDuringSonomama)
{
	//Mate can indeed be called during sonomama in the case the judges want to give a hansokumake
	//or to discuss something with the other judges or to give tori a shido

	initialize();
	for (Fighter osaekomi_holder = Fighter::White; osaekomi_holder <= Fighter::Blue; osaekomi_holder++)
	{
		Application app;
		Mat m(1);

		EXPECT_TRUE(m.Open());

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.Osaekomi(osaekomi_holder);

		ZED::Core::Pause(3000);
		m.Sonomama();
		ZED::Core::Pause(5000);
		m.Mate();
		ZED::Core::Pause(5000);

		EXPECT_FALSE(m.IsHajime());
		EXPECT_FALSE(m.IsOsaekomi());
		EXPECT_FALSE(m.IsOsaekomiRunning());
		EXPECT_FALSE(m.HasConcluded());

		m.AddIppon(osaekomi_holder);
		EXPECT_TRUE(m.EndMatch());			
	}
}



TEST(Mat, HansokumakeDuringOsaekomi)
{
	//IJF 2018: However, should the penalty to be awarded be hansoku-make, the referee shall,
	//after announcing sono - mama, consult with the judges, announce mate to return
	//the contestants to their starting positions, then award hansoku - make and end the
	//contest by announcing sore - made.

	initialize();
	for (Fighter osaekomi_holder = Fighter::White; osaekomi_holder <= Fighter::Blue; osaekomi_holder++)
	{
		for (Fighter hansokumake_committer = Fighter::White; hansokumake_committer <= Fighter::Blue; hansokumake_committer++)
		{
			Application app;
			Mat m(1);

			EXPECT_TRUE(m.Open());

			Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
			match.SetMatID(1);
			match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0));
			EXPECT_TRUE(m.StartMatch(&match));

			m.Hajime();
			m.Osaekomi(osaekomi_holder);

			ZED::Core::Pause(3000);
			m.Sonomama();
			ZED::Core::Pause(5000);
			m.Mate();
			ZED::Core::Pause(3000);
			m.AddHansokuMake(hansokumake_committer);
			m.AddNoDisqualification(hansokumake_committer);
			ZED::Core::Pause(3000);

			EXPECT_FALSE(m.IsHajime());
			EXPECT_FALSE(m.IsOsaekomi());
			EXPECT_FALSE(m.IsOsaekomiRunning());
			EXPECT_TRUE(m.GetScoreboard(hansokumake_committer).m_HansokuMake);


			EXPECT_TRUE(m.EndMatch());
			EXPECT_TRUE(match.GetResult().m_Winner == !hansokumake_committer);
			EXPECT_TRUE(match.GetResult().m_Score  == Match::Score::Ippon);
		}
	}
}



TEST(Mat, Koka)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, true, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));


		EXPECT_TRUE(m.GetScoreboard(f).m_Koka == 0);
		for (int k = 1; k < 25; k++)
		{
			m.AddKoka(f);
			EXPECT_TRUE(m.GetScoreboard(f).m_Koka == k);
		}
		for (int k = 1; k < 25; k++)
			m.RemoveKoka(f);
		EXPECT_TRUE(m.GetScoreboard(f).m_Koka == 0);

		m.AddIppon(f);

		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, Koka2)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 60, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));


		EXPECT_TRUE(m.GetScoreboard(f).m_Koka == 0);
		for (int k = 1; k < 25; k++)
		{
			m.AddKoka(f);
			EXPECT_TRUE(m.GetScoreboard(f).m_Koka == 0);
		}
		for (int k = 1; k < 25; k++)
		{
			m.RemoveKoka(f);
			EXPECT_TRUE(m.GetScoreboard(f).m_Koka == 0);
		}

		m.AddIppon(f);

		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, WazariAwaseteIppon)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		EXPECT_TRUE(m.StartMatch(&match));

			
		EXPECT_TRUE(m.GetScoreboard(f).m_Ippon == 0);
		EXPECT_TRUE(m.GetScoreboard(f).m_WazaAri == 0);
		EXPECT_FALSE(m.EndMatch());

		m.AddWazaAri(f);				

		EXPECT_TRUE(m.GetScoreboard(f).m_Ippon == 0);
		EXPECT_TRUE(m.GetScoreboard(f).m_WazaAri == 1);
		EXPECT_FALSE(m.EndMatch());

		m.AddWazaAri(f);

		EXPECT_TRUE(m.GetScoreboard(f).m_Ippon == 1);
		EXPECT_TRUE(m.GetScoreboard(f).m_WazaAri == 2);

		m.RemoveWazaAri(f);

		EXPECT_TRUE(m.GetScoreboard(f).m_Ippon == 0);
		EXPECT_TRUE(m.GetScoreboard(f).m_WazaAri == 1);

		m.AddWazaAri(f);

		EXPECT_TRUE(m.GetScoreboard(f).m_Ippon == 1);
		EXPECT_TRUE(m.GetScoreboard(f).m_WazaAri == 2);

		EXPECT_TRUE(m.EndMatch());
	}
}



TEST(Mat, GoldenScore)
{
	initialize();
	Application app;
	Mat m(1);

	Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 2, 10, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m.StartMatch(&match));

	m.Hajime();

	ZED::Core::Pause(3000);
	EXPECT_TRUE(m.IsOutOfTime());

	EXPECT_FALSE(m.HasConcluded());

	m.EnableGoldenScore();

	m.Hajime();

	EXPECT_FALSE(m.IsOutOfTime());

	ZED::Core::Pause(11 * 1000);
	EXPECT_TRUE(m.IsOutOfTime());

	EXPECT_FALSE(m.HasConcluded());
	EXPECT_FALSE(m.EndMatch());

	m.Hantei(Fighter::White);

	EXPECT_TRUE(m.HasConcluded());
	EXPECT_TRUE(m.EndMatch());
}



TEST(Mat, GoldenScore2)
{
	initialize();
	Application app;
	Mat m(1);

	Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 10, 0, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m.StartMatch(&match));

	m.Hajime();

	ZED::Core::Pause(11 * 1000);
	EXPECT_TRUE(m.IsOutOfTime());

	EXPECT_FALSE(m.HasConcluded());

	EXPECT_FALSE(m.EnableGoldenScore());

	m.Hajime();

	EXPECT_TRUE(m.IsOutOfTime());

	EXPECT_FALSE(m.HasConcluded());
	EXPECT_FALSE(m.EndMatch());

	m.Hantei(Fighter::White);

	EXPECT_TRUE(m.HasConcluded());
	EXPECT_TRUE(m.EndMatch());
}



TEST(Mat, GoldenScoreResetTime)
{
	initialize();
	Application app;
	Mat m(1);

	Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 5, 5, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m.StartMatch(&match));

	m.Hajime();

	ZED::Core::Pause(6 * 1000);
	EXPECT_TRUE(m.IsOutOfTime());

	EXPECT_FALSE(m.HasConcluded());

	EXPECT_TRUE(m.EnableGoldenScore());

	EXPECT_TRUE(m.GetTimeElapsed() == 0);

	m.AddIppon(Fighter::White);

	EXPECT_TRUE(m.HasConcluded());
	EXPECT_TRUE(m.EndMatch());
}



TEST(Mat, GoldenScoreKeepsShidosAndMedicalExaminations)
{
	initialize();
	Application app;
	Mat m(1);

	Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 5, 5, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m.StartMatch(&match));

	m.Hajime();

	m.AddShido(Fighter::White);
	m.AddShido(Fighter::Blue);
	m.AddMedicalExamination(Fighter::White);
	m.AddMedicalExamination(Fighter::Blue);

	ZED::Core::Pause(6 * 1000);
	EXPECT_TRUE(m.IsOutOfTime());

	EXPECT_FALSE(m.HasConcluded());

	EXPECT_TRUE(m.EnableGoldenScore());
	m.Hajime();

	EXPECT_TRUE(m.GetScoreboard(Fighter::White).m_Shido == 1);
	EXPECT_TRUE(m.GetScoreboard(Fighter::Blue ).m_Shido == 1);
	EXPECT_TRUE(m.GetScoreboard(Fighter::White).m_MedicalExamination == 1);
	EXPECT_TRUE(m.GetScoreboard(Fighter::Blue ).m_MedicalExamination == 1);

	m.AddIppon(Fighter::White);

	EXPECT_TRUE(m.HasConcluded());
	EXPECT_TRUE(m.EndMatch());
}



TEST(Mat, Draw)
{
	initialize();
	Application app;
	Mat m(1);

	Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 10, 60, 30, 20, false, false, true, 0));
	EXPECT_TRUE(m.StartMatch(&match));

	m.Hajime();

	for (int k = 0; k < 9; k++)
	{
		EXPECT_FALSE(m.IsOutOfTime());
		ZED::Core::Pause(1000);
	}

	ZED::Core::Pause(2 * 1000);
	EXPECT_TRUE(m.IsOutOfTime());

	EXPECT_FALSE(m.HasConcluded());

	m.SetAsDraw(true);

	EXPECT_TRUE(m.HasConcluded());
	EXPECT_TRUE(m.EndMatch());
}



TEST(Mat, Draw2)
{
	initialize();
	Application app;
	Mat m(1, &app);

	Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
	match.SetMatID(1);
	match.SetRuleSet(new RuleSet("Test", 10, 60, 30, 20, false, false, false, 0));
	EXPECT_TRUE(m.StartMatch(&match));

	m.Hajime();

	for (int k = 0; k < 9; k++)
	{
		EXPECT_FALSE(m.IsOutOfTime());
		ZED::Core::Pause(1000);
	}

	ZED::Core::Pause(2 * 1000);
	EXPECT_TRUE(m.IsOutOfTime());

	EXPECT_FALSE(m.HasConcluded());

	m.SetAsDraw(true);

	EXPECT_FALSE(m.HasConcluded());
	EXPECT_FALSE(m.EndMatch());

	m.Hantei(Fighter::White);
	EXPECT_TRUE(m.EndMatch());
}



TEST(Mat, Hantei)
{
	initialize();
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;
		Mat m(1);

		Match match(new Judoka("White", "LastnameW"), new Judoka("Blue", "LastnameB"), nullptr);
		match.SetMatID(1);
		match.SetRuleSet(new RuleSet("Test", 2, 60, 30, 20, false, false, false, 0));
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();

		ZED::Core::Pause(3000);
		EXPECT_TRUE(m.IsOutOfTime());

		EXPECT_FALSE(m.HasConcluded());

		m.Hantei(f);

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());

		EXPECT_TRUE(match.GetResult().m_Winner == Fighter2Winner(f));
		EXPECT_TRUE(match.GetResult().m_Score  == Match::Score::Hantei);
		EXPECT_TRUE(match.GetResult().m_Score  == (Match::Score)1);
	}
}



TEST(Mat, BreakTime)
{
	initialize();
	srand(ZED::Core::CurrentTimestamp());
	for (int time = 55; time <= 3 * 60; time += 60 + rand() % 60)
	{
		Application app;
		Mat m(1);

		RuleSet* rule_set = new RuleSet("Test", 10, 10, 30, 20, false, false, false, time);

		Judoka* j1 = new Judoka("Needs", "Break");
		Judoka* j2 = new Judoka("White", "LastnameW");
		Judoka* j3 = new Judoka("Blue",  "LastnameB");

		Match match(j1, j2, nullptr);
		match.SetMatID(1);
		match.SetRuleSet(rule_set);
		EXPECT_TRUE(m.StartMatch(&match));

		m.Hajime();
		m.AddIppon((Fighter)(rand()%2));

		EXPECT_TRUE(m.HasConcluded());
		EXPECT_TRUE(m.EndMatch());

		Match match2(j1, j3, nullptr, 1);
		match2.SetRuleSet(rule_set);

		for (int k = 0; k < time-1; k++)
		{
			EXPECT_FALSE(m.StartMatch(&match2));
			ZED::Core::Pause(1000);
		}

		ZED::Core::Pause(2500);

		EXPECT_TRUE(m.StartMatch(&match2));
	}
}