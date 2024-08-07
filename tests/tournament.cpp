#include "tests.h"



TEST(Tournament, DontDuplicateParticipants)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_TRUE(d.GetNumJudoka() == 0);

		Judoka* j1 = new Judoka("Firstname", "Lastname", 50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 60, Gender::Female);

		d.AddJudoka(j1);
		d.AddJudoka(j2);

		Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
		tourney.Reset();
		tourney.EnableAutoSave(false);

		EXPECT_TRUE(tourney.AddParticipant(j1));
		EXPECT_EQ(tourney.GetParticipants().size(), 1);
		EXPECT_FALSE(tourney.AddParticipant(j1));
		EXPECT_EQ(tourney.GetParticipants().size(), 1);

		EXPECT_TRUE(tourney.AddParticipant(j2));
		EXPECT_EQ(tourney.GetParticipants().size(), 2);
		EXPECT_FALSE(tourney.AddParticipant(j2));
		EXPECT_EQ(tourney.GetParticipants().size(), 2);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}




TEST(Tournament, MatchAgainstOneself)
{
	initialize();
	Database d;
	d.EnableAutoSave(false);


	Judoka* j1 = new Judoka("Firstname", "Lastname", 50, Gender::Male);

	d.AddJudoka(j1);

	Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
	tourney->Reset();
	tourney->EnableAutoSave(false);

	EXPECT_FALSE(tourney->AddMatch(new Match(j1, j1, tourney)));

	delete tourney;
}



TEST(Tournament, MoveSchedule)
{
	initialize();


	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname", "Lastname", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	auto match1 = new Match(&j1, &j2, &tourney, 1);
	auto match2 = new Match(&j1, &j2, &tourney, 1);
	auto match3 = new Match(&j1, &j2, &tourney, 1);
	auto match4 = new Match(&j1, &j2, &tourney, 2);
	auto match5 = new Match(&j1, &j2, &tourney, 2);
	auto match6 = new Match(&j1, &j2, &tourney, 2);

	tourney.AddMatch(match1);
	tourney.AddMatch(match2);
	tourney.AddMatch(match3);
	tourney.AddMatch(match4);
	tourney.AddMatch(match5);
	tourney.AddMatch(match6);

	EXPECT_EQ(*tourney.GetSchedule()[0], *match1);
	EXPECT_EQ(*tourney.GetSchedule()[1], *match2);

	EXPECT_FALSE(tourney.MoveMatchUp(*match1));
	EXPECT_TRUE(tourney.MoveMatchUp(*match2));

	EXPECT_EQ(*tourney.GetSchedule()[0], *match2);
	EXPECT_EQ(*tourney.GetSchedule()[1], *match1);

	EXPECT_TRUE(tourney.MoveMatchUp(*match4));
	EXPECT_TRUE(tourney.MoveMatchUp(*match4));
	EXPECT_TRUE(tourney.MoveMatchUp(*match4));
	EXPECT_FALSE(tourney.MoveMatchUp(*match4));

	EXPECT_EQ(*tourney.GetSchedule()[0], *match4);
	EXPECT_EQ(*tourney.GetSchedule()[1], *match2);
	EXPECT_EQ(*tourney.GetSchedule()[2], *match1);
	EXPECT_EQ(*tourney.GetSchedule()[3], *match3);
	EXPECT_EQ(*tourney.GetSchedule()[4], *match5);
	EXPECT_EQ(*tourney.GetSchedule()[5], *match6);

	EXPECT_TRUE(tourney.MoveMatchUp(*match5, 2));

	EXPECT_EQ(*tourney.GetSchedule()[0], *match5);
	EXPECT_EQ(*tourney.GetSchedule()[1], *match2);
	EXPECT_EQ(*tourney.GetSchedule()[2], *match1);
	EXPECT_EQ(*tourney.GetSchedule()[3], *match3);
	EXPECT_EQ(*tourney.GetSchedule()[4], *match4);
	EXPECT_EQ(*tourney.GetSchedule()[5], *match6);

	EXPECT_FALSE(tourney.MoveMatchDown(*match6));

	EXPECT_TRUE(tourney.MoveMatchUp(*match4));
	EXPECT_TRUE(tourney.MoveMatchUp(*match4));

	EXPECT_EQ(*tourney.GetSchedule()[0], *match5);
	EXPECT_EQ(*tourney.GetSchedule()[1], *match2);
	EXPECT_EQ(*tourney.GetSchedule()[2], *match4);
	EXPECT_EQ(*tourney.GetSchedule()[3], *match1);
	EXPECT_EQ(*tourney.GetSchedule()[4], *match3);
	EXPECT_EQ(*tourney.GetSchedule()[5], *match6);

	EXPECT_TRUE(tourney.MoveMatchDown(*match4, 2));

	EXPECT_EQ(*tourney.GetSchedule()[0], *match5);
	EXPECT_EQ(*tourney.GetSchedule()[1], *match2);
	EXPECT_EQ(*tourney.GetSchedule()[2], *match6);
	EXPECT_EQ(*tourney.GetSchedule()[3], *match1);
	EXPECT_EQ(*tourney.GetSchedule()[4], *match3);
	EXPECT_EQ(*tourney.GetSchedule()[5], *match4);
}



TEST(Tournament, DeleteMatch)
{
	{
		initialize();

		Judoka j1("Firstname1", "Lastname1", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka j3("Firstname3", "Lastname3", 52, Gender::Male);
		Judoka j4("Firstname4", "Lastname4", 53, Gender::Male);

		ZED::Core::RemoveFile("tournaments/deleteMe.yml");

		Tournament tourney("deleteMe");
		tourney.Reset();
		tourney.EnableAutoSave(false);

		EXPECT_TRUE(tourney.AddParticipant(&j1));
		EXPECT_TRUE(tourney.AddParticipant(&j2));
		EXPECT_TRUE(tourney.AddParticipant(&j3));
		EXPECT_TRUE(tourney.AddParticipant(&j4));

		auto matchtable = new SingleElimination(Weight(50), Weight(60));
		matchtable->IsThirdPlaceMatch(true);
		tourney.AddMatchTable(matchtable);
		tourney.GenerateSchedule();

		EXPECT_EQ(tourney.GetMatchTables().size(),  1);
		EXPECT_EQ(matchtable->GetSchedule().size(), 4);
		EXPECT_EQ(tourney.GetSchedule().size(),     4);

		tourney.RemoveMatch(tourney.GetSchedule()[3]->GetUUID());

		EXPECT_EQ(tourney.GetMatchTables().size(),  1);
		EXPECT_EQ(matchtable->GetSchedule().size(), 3);
		EXPECT_EQ(tourney.GetSchedule().size(),     3);

		tourney.AddMatch(new Match(&j1, &j2, nullptr));

		EXPECT_EQ(tourney.GetMatchTables().size(),  2);
		EXPECT_EQ(matchtable->GetSchedule().size(), 3);
		EXPECT_EQ(tourney.GetSchedule().size(),     4);
	}

	ZED::Core::RemoveFile("deleteMe.yml");
}



TEST(Tournament, Disqualification)
{
	initialize();

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Match* match = new Match(&j1, &j2, &tourney);
	tourney.AddMatch(match);

	tourney.Disqualify(j1);

	EXPECT_TRUE(match->HasConcluded());
	EXPECT_EQ(match->GetResult().m_Winner, Winner::Blue);
	EXPECT_EQ(match->GetResult().m_Score, Match::Score::Ippon);
	EXPECT_EQ(match->GetResult().m_Time, 0);
	EXPECT_EQ(match->GetLog().GetEvents().size(), 0);

	EXPECT_TRUE(tourney.IsDisqualified(j1));
	EXPECT_FALSE(tourney.IsDisqualified(j2));
}



TEST(Tournament, AddMatchAfterDisqualification)
{
	initialize();

	Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
	Judoka* j2 = new Judoka("Firstname2", "Lastname2", 50, Gender::Male);
	Judoka* j3 = new Judoka("Firstname3", "Lastname3", 50, Gender::Male);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	Tournament tourney("deleteMe");
	//tourney.Reset();
	tourney.EnableAutoSave(false);

	Match* match1 = new Match(j1, j2, &tourney);
	EXPECT_TRUE(tourney.AddMatch(match1));
	Match* matchdummy = new Match(j2, j3, &tourney);
	EXPECT_TRUE(tourney.AddMatch(matchdummy));

	tourney.Disqualify(*j1);

	Match* match2 = new Match(j1, j2, &tourney);
	EXPECT_TRUE(tourney.AddMatch(match2));

	EXPECT_TRUE(match1->HasConcluded());
	EXPECT_EQ(match1->GetResult().m_Winner, Winner::Blue);
	EXPECT_EQ(match1->GetResult().m_Score, Match::Score::Ippon);
	EXPECT_EQ(match1->GetResult().m_Time, 0);
	EXPECT_EQ(match1->GetLog().GetEvents().size(), 0);

	EXPECT_TRUE(match2->HasConcluded());
	EXPECT_EQ(match2->GetResult().m_Winner, Winner::Blue);
	EXPECT_EQ(match2->GetResult().m_Score, Match::Score::Ippon);
	EXPECT_EQ(match2->GetResult().m_Time, 0);
	EXPECT_EQ(match2->GetLog().GetEvents().size(), 0);
}



TEST(Tournament, AddAgeGroup)
{
	initialize();

	Judoboard::Application app;

	app.StartLocalMat(1);

	Judoboard::Mat* mat = (Judoboard::Mat*)app.GetLocalMat();

	srand(ZED::Core::CurrentTimestamp());

	auto tourney = new Judoboard::Tournament("Demo Tournament");
	tourney->EnableAutoSave(false);

	auto rule_set  = Judoboard::RuleSet("Demo", 180, 60, 20, 10);
	auto age_group = Judoboard::AgeGroup("U18", 0, 100, &rule_set);

	auto m1 = new Judoboard::RoundRobin(Weight(0), Weight(120));
	m1->SetMatID(1);
	m1->SetAgeGroup(&age_group);
	tourney->AddMatchTable(m1);

	EXPECT_TRUE(tourney->FindAgeGroup(age_group.GetUUID()));
	EXPECT_TRUE(tourney->FindRuleSet(rule_set.GetUUID()));

	app.GetDatabase().AddClub(new Judoboard::Club("Altenhagen"));
	app.GetDatabase().AddClub(new Judoboard::Club("Brackwede"));
	app.GetDatabase().AddClub(new Judoboard::Club("Senne"));

	for (int i = 0; i < 5; i++)
		tourney->AddParticipant(new Judoboard::Judoka(Test_CreateRandomJudoka(&app.GetDatabase())));

	EXPECT_EQ(tourney->GetSchedule().size(), 5*4/2);

	delete tourney;
}



TEST(Tournament, DoubleDisqualification)
{
	initialize();

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Match* match = new Match(&j1, &j2, &tourney);
	tourney.AddMatch(match);

	tourney.Disqualify(j1);
	tourney.Disqualify(j2);

	EXPECT_TRUE(match->HasConcluded());
	EXPECT_EQ(match->GetResult().m_Winner, Winner::Draw);
	EXPECT_EQ(match->GetResult().m_Score, Match::Score::Draw);
	EXPECT_EQ(match->GetResult().m_Time, 0);
	EXPECT_EQ(match->GetLog().GetEvents().size(), 0);
}



TEST(Tournament, DoubleDisqualification2)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Judoka j1("Firstname", "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);

		Tournament tourney("deleteMe");
		tourney.Reset();
		tourney.EnableAutoSave(false);

		Match* match1 = new Match(&j1, &j2, &tourney, 1);
		Match* match2 = new Match(&j1, &j2, &tourney, 1);

		tourney.AddMatch(match1);
		tourney.AddMatch(match2);

		Mat m(1);

		EXPECT_TRUE(m.StartMatch(match1));
		m.Hajime();
		m.AddIppon(Fighter::White);
		EXPECT_TRUE(m.EndMatch());

		tourney.Disqualify(j1);
		tourney.Disqualify(j2);

		EXPECT_TRUE(match1->HasConcluded());
		EXPECT_EQ(match1->GetResult().m_Winner, Winner::White);
		EXPECT_EQ(match1->GetResult().m_Score, Match::Score::Ippon);
		EXPECT_LE(match1->GetResult().m_Time, 50u);
		EXPECT_GE(match1->GetLog().GetEvents().size(), 3u);

		EXPECT_TRUE(match2->HasConcluded());
		EXPECT_EQ(match2->GetResult().m_Winner, Winner::Draw);
		EXPECT_EQ(match2->GetResult().m_Score, Match::Score::Draw);
		EXPECT_LE(match2->GetResult().m_Time, 50u);
		EXPECT_EQ(match2->GetLog().GetEvents().size(), 0u);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, ReviseMatch)
{
	initialize();

	Judoka* j1 = new Judoka("Firstname", "Lastname", 50, Gender::Male);
	Judoka* j2 = new Judoka("Firstname2", "Lastname2", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Mat mat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; ++f)
	{
		Match* match = new Match(j1, j2, &tourney, 1);
		tourney.AddMatch(match);

		EXPECT_TRUE(mat.StartMatch(match));

		mat.Hajime();
		ZED::Core::Pause(1000);
		mat.AddIppon(f);

		mat.EndMatch();
		EXPECT_TRUE(match->HasConcluded());
		EXPECT_EQ(match->GetResult().m_Winner, f);

		EXPECT_TRUE(tourney.ReviseMatch(*match, mat));

		EXPECT_TRUE(mat.AreFightersOnMat());
		EXPECT_EQ(mat.GetScoreboard(f).m_Ippon,  1);
		EXPECT_EQ(mat.GetScoreboard(!f).m_Ippon, 0);

		mat.RemoveIppon(f);
		mat.Hajime();
		ZED::Core::Pause(1000);
		mat.AddIppon(!f);

		mat.EndMatch();
		EXPECT_TRUE(match->HasConcluded());
		EXPECT_EQ(match->GetResult().m_Winner, !f);
		EXPECT_LE(std::abs((int)match->GetResult().m_Time - 2000), 50);
	}
}



TEST(Tournament, RevokeDisqualification)
{
	initialize();

	Judoka* j1 = new Judoka("Firstname", "Lastname", 50, Gender::Male);
	Judoka* j2 = new Judoka("Firstname2", "Lastname2", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Match* match = new Match(j1, j2, &tourney);
	tourney.AddMatch(match);

	tourney.Disqualify(*j1);
	tourney.RevokeDisqualification(*j1);

	EXPECT_FALSE(match->HasConcluded());
	EXPECT_FALSE(tourney.IsDisqualified(*j1));
}



TEST(Tournament, RevokeDoubleDisqualification)
{
	initialize();

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Match* match = new Match(&j1, &j2, &tourney);
	tourney.AddMatch(match);

	tourney.Disqualify(j1);
	tourney.Disqualify(j2);
	tourney.RevokeDisqualification(j1);

	EXPECT_TRUE(match->HasConcluded());
	EXPECT_EQ(match->GetResult().m_Winner, Winner::White);
	EXPECT_FALSE(tourney.IsDisqualified(j1));
	EXPECT_TRUE(tourney.IsDisqualified(j2));
}



TEST(Tournament, BestOf3_Matchtable)
{
	{
		initialize();

		Judoka j1("Firstname1", "Lastname1", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka j3("Firstname3", "Lastname3", 52, Gender::Male);
		Judoka j4("Firstname4", "Lastname4", 53, Gender::Male);

		ZED::Core::RemoveFile("tournaments/deleteMe.yml");

		Tournament tourney("deleteMe");
		tourney.Reset();
		tourney.EnableAutoSave(false);

		EXPECT_TRUE(tourney.AddParticipant(&j1));
		EXPECT_TRUE(tourney.AddParticipant(&j2));
		EXPECT_TRUE(tourney.AddParticipant(&j3));
		EXPECT_TRUE(tourney.AddParticipant(&j4));

		auto matchtable = new RoundRobin(Weight(50), Weight(60));
		matchtable->IsBestOfThree(true);
		tourney.AddMatchTable(matchtable);
		tourney.GenerateSchedule();

		EXPECT_EQ(tourney.GetMatchTables().size(), 1);

		EXPECT_EQ(matchtable->GetSchedule().size(), 6 * 3);
	}

	ZED::Core::RemoveFile("deleteMe.yml");
}



TEST(Tournament, ColorsForMatchTables)
{
	{
		initialize();
		Database d;
		d.Save("deleteMe.yml");
		d.Load("deleteMe.yml");

		EXPECT_TRUE(d.GetNumJudoka() == 0);

		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		d.AddJudoka(j1);
		d.AddJudoka(j2);
		d.AddJudoka(j3);
		d.AddJudoka(j4);

		ZED::Core::RemoveFile("tournaments/deleteMe.yml");

		Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
		tourney.Reset();
		tourney.EnableAutoSave(false);

		EXPECT_TRUE(tourney.AddParticipant(j1));
		EXPECT_TRUE(tourney.AddParticipant(j2));
		EXPECT_TRUE(tourney.AddParticipant(j3));
		EXPECT_TRUE(tourney.AddParticipant(j4));

		tourney.AddMatchTable(new RoundRobin(Weight(50), Weight(55)));
		tourney.AddMatchTable(new RoundRobin(Weight(60), Weight(65)));

		ASSERT_EQ(tourney.GetMatchTables().size(), 2);
		EXPECT_EQ(tourney.GetMatchTables()[0]->GetColor(), Color::Name::Blue);
		EXPECT_EQ(tourney.GetMatchTables()[1]->GetColor(), Color::Name::Red);
	}

	ZED::Core::RemoveFile("deleteMe.yml");
}



TEST(Tournament, ParticipantHasSameIDAsInDatabase)
{
	initialize();
	Database d;
	d.EnableAutoSave(false);

	EXPECT_TRUE(d.GetNumJudoka() == 0);

	Judoka* j1 = new Judoka("Firstname", "Lastname", 50, Gender::Male);

	d.AddJudoka(j1);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
	tourney.Reset();

	EXPECT_TRUE(tourney.AddParticipant(j1));
	ASSERT_TRUE(tourney.FindParticipant(j1->GetUUID()));
	EXPECT_EQ(j1->GetUUID(), tourney.FindParticipant(j1->GetUUID())->GetUUID());
	//tourney gets saved now

	tourney.Save();


	Tournament t("deleteMe");
	
	EXPECT_EQ(t.GetParticipants().size(), 1);
	ASSERT_TRUE(t.FindParticipant(j1->GetUUID()));
	EXPECT_EQ(j1->GetUUID(), t.FindParticipant(j1->GetUUID())->GetUUID());

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, Lottery)
{
	initialize();	

	for (int i = 0; i < 100; ++i)
	{
		ZED::Core::RemoveFile("tournaments/deleteMe.yml");
		Tournament tourney("deleteMe");

		Association* assoc = new Association(GetRandomName(), nullptr);
		Club* c1 = new Club(GetRandomName());
		c1->SetParent(assoc);
		Club* c2 = new Club(GetRandomName());
		c2->SetParent(assoc);

		Judoka* j1 = new Judoka(GetRandomName(), GetRandomName());
		j1->SetClub(c1);
		Judoka* j2 = new Judoka(GetRandomName(), GetRandomName());
		j2->SetClub(c2);

		tourney.SetOrganizer(assoc);

		EXPECT_TRUE(tourney.AddParticipant(j1));
		EXPECT_TRUE(tourney.AddParticipant(j2));

		auto lot1 = tourney.GetLotOfAssociation(*c1);
		auto lot2 = tourney.GetLotOfAssociation(*c2);
		EXPECT_GE(lot1, 0);
		EXPECT_GE(lot2, 0);
		EXPECT_LE(lot1, 1);
		EXPECT_LE(lot2, 1);
		EXPECT_NE(lot1, lot2);

		tourney.Save();

		Tournament tourney2("deleteMe");
		EXPECT_EQ(tourney.GetLotOfAssociation(*c1), tourney2.GetLotOfAssociation(*c1));
		EXPECT_EQ(tourney.GetLotOfAssociation(*c2), tourney2.GetLotOfAssociation(*c2));
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, CorrectLotInSingleElimination)
{
	initialize();	

	for (int i = 0; i < 100; ++i)
	{
		ZED::Core::RemoveFile("tournaments/deleteMe.yml");
		Tournament tourney("deleteMe");
		tourney.EnableAutoSave(false);

		Association* assoc = new Association(GetRandomName(), nullptr);
		Club* c1 = new Club(GetRandomName());
		c1->SetParent(assoc);
		Club* c2 = new Club(GetRandomName());
		c2->SetParent(assoc);

		Judoka* j1 = new Judoka(GetRandomName(), GetRandomName(), Weight(50));
		j1->SetClub(c1);
		Judoka* j2 = new Judoka(GetRandomName(), GetRandomName(), Weight(50));
		j2->SetClub(c1);
		Judoka* j3 = new Judoka(GetRandomName(), GetRandomName(), Weight(50));
		j3->SetClub(c1);

		Judoka* j4 = new Judoka(GetRandomName(), GetRandomName(), Weight(50));
		j4->SetClub(c2);
		Judoka* j5 = new Judoka(GetRandomName(), GetRandomName(), Weight(50));
		j5->SetClub(c2);
		Judoka* j6 = new Judoka(GetRandomName(), GetRandomName(), Weight(50));
		j6->SetClub(c2);

		tourney.SetOrganizer(assoc);

		EXPECT_TRUE(tourney.AddParticipant(j1));
		EXPECT_TRUE(tourney.AddParticipant(j4));
		EXPECT_TRUE(tourney.AddParticipant(j2));
		EXPECT_TRUE(tourney.AddParticipant(j5));
		EXPECT_TRUE(tourney.AddParticipant(j3));
		EXPECT_TRUE(tourney.AddParticipant(j6));

		tourney.AddMatchTable(new SingleElimination(Weight(10), Weight(200)));
		auto table = tourney.GetMatchTables()[0];

		auto lot1 = tourney.GetLotOfAssociation(*c1);
		auto lot2 = tourney.GetLotOfAssociation(*c2);
		
		if (lot1 == 0)
		{
			EXPECT_LE(table->GetStartPosition(j1), 2);
			EXPECT_LE(table->GetStartPosition(j2), 2);
			EXPECT_LE(table->GetStartPosition(j3), 2);

			EXPECT_GE(table->GetStartPosition(j4), 3);
			EXPECT_GE(table->GetStartPosition(j5), 3);
			EXPECT_GE(table->GetStartPosition(j6), 3);
		}

		else
		{
			EXPECT_LE(table->GetStartPosition(j4), 2);
			EXPECT_LE(table->GetStartPosition(j5), 2);
			EXPECT_LE(table->GetStartPosition(j6), 2);

			EXPECT_GE(table->GetStartPosition(j1), 3);
			EXPECT_GE(table->GetStartPosition(j2), 3);
			EXPECT_GE(table->GetStartPosition(j3), 3);
		}
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, LotteryTier)
{
	initialize();	

	for (int tier = 0; tier < 10; ++tier)
	{
		ZED::Core::RemoveFile("tournaments/deleteMe.yml");
		Tournament tourney("deleteMe");

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

		auto detmold = new Judoboard::Association("Detmold", nrw);

		auto biegue = new Judoboard::Association(u8"Bielefeld/G\u00fctersloh", detmold);

		Club* c1 = new Club("club1", biegue);
		Club* c2 = new Club("club2", biegue);

		Judoka* j1 = new Judoka(GetRandomName(), GetRandomName());
		j1->SetClub(c1);
		Judoka* j2 = new Judoka(GetRandomName(), GetRandomName());
		j2->SetClub(c2);

		tourney.SetOrganizer(de);
		tourney.SetLotteryTier(tier);
		EXPECT_EQ(tourney.GetLotteryTier(), tier);

		EXPECT_TRUE(tourney.AddParticipant(j1));
		EXPECT_TRUE(tourney.AddParticipant(j2));

		tourney.Save();

		Tournament tourney2("deleteMe");
		EXPECT_EQ(tourney2.GetLotteryTier(), tier);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, Lottery_Histogram)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	size_t c1_count = 0;
	size_t c2_count = 0;

	for (int i = 0; i < 100; ++i)
	{
		Tournament tourney("deleteMe");
		tourney.EnableAutoSave(false);

		Association* assoc = new Association(GetRandomName(), nullptr);
		Club* c1 = new Club(GetRandomName());
		c1->SetParent(assoc);
		Club* c2 = new Club(GetRandomName());
		c2->SetParent(assoc);

		Judoka* j1 = new Judoka(GetRandomName(), GetRandomName());
		j1->SetClub(c1);
		Judoka* j2 = new Judoka(GetRandomName(), GetRandomName());
		j2->SetClub(c2);

		tourney.SetOrganizer(assoc);

		EXPECT_TRUE(tourney.AddParticipant(j1));
		EXPECT_TRUE(tourney.AddParticipant(j2));

		auto lot1 = tourney.GetLotOfAssociation(*c1);
		auto lot2 = tourney.GetLotOfAssociation(*c2);

		c1_count += lot1;
		c2_count += lot2;
	}

	EXPECT_GE(c1_count, 35);
	EXPECT_LE(c1_count, 65);
	EXPECT_GE(c2_count, 35);
	EXPECT_LE(c2_count, 65);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, CanNotAddParticipantOfWrongAssociation)
{
	initialize();
	
	auto club1 = new Club("Club1");
	auto club2 = new Club("Club2");
	Judoka* j1 = new Judoka("Firstname", "Lastname", 50, Gender::Male);

	j1->SetClub(club1);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament tourney("deleteMe");
	tourney.Reset();

	EXPECT_FALSE(tourney.GetOrganizer());
	tourney.SetOrganizer(club2);
	ASSERT_TRUE(tourney.GetOrganizer());
	EXPECT_EQ(*tourney.GetOrganizer(), *club2);

	EXPECT_FALSE(tourney.AddParticipant(j1));
	tourney.SetOrganizer(club1);
	EXPECT_EQ(*tourney.GetOrganizer(), *club1);
	EXPECT_TRUE(tourney.AddParticipant(j1));

	tourney.Save();


	Tournament t("deleteMe");
	t.EnableAutoSave(false);
	EXPECT_EQ(t.GetParticipants().size(), 1);
	ASSERT_TRUE(t.FindParticipant(j1->GetUUID()));
	EXPECT_EQ(j1->GetUUID(), t.FindParticipant(j1->GetUUID())->GetUUID());
	ASSERT_TRUE(t.GetOrganizer());
	EXPECT_EQ(*t.GetOrganizer(), *club1);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, HasDefaultRuleSet)
{
	initialize();
	Database d;
	d.EnableAutoSave(false);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
	EXPECT_TRUE(tourney.AddParticipant(new Judoka("temp", "temp", 50)));
	tourney.Reset();

	tourney.Save();


	Tournament t("deleteMe");

	ASSERT_FALSE(t.GetDefaultRuleSet());

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, HasDefaultRuleSet2)
{
	initialize();
	Database d;
	d.EnableAutoSave(false);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	auto rules = new RuleSet("Default", 60, 30, 20, 10);
	d.AddRuleSet(rules);

	Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
	tourney.SetDefaultRuleSet(rules);
	EXPECT_TRUE(tourney.AddParticipant(new Judoka("temp", "temp", 50)));

	tourney.Save();


	Tournament t("deleteMe");

	ASSERT_TRUE(t.GetDefaultRuleSet());
	ASSERT_TRUE(d.FindRuleSetByName("Default"));
	EXPECT_EQ(t.GetDefaultRuleSet()->GetUUID(), d.FindRuleSetByName("Default")->GetUUID());

	EXPECT_EQ(t.GetDefaultRuleSet()->GetMatchTime(),         60);
	EXPECT_EQ(t.GetDefaultRuleSet()->GetGoldenScoreTime(),   30);
	EXPECT_EQ(t.GetDefaultRuleSet()->GetOsaeKomiTime(false), 20);
	EXPECT_EQ(t.GetDefaultRuleSet()->GetOsaeKomiTime(true),  10);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, FindSubMatchTable)
{
	initialize();

	{
		ZED::Core::RemoveFile("tournaments/deleteMe.yml");
		Tournament tourney("deleteMe");
		tourney.EnableAutoSave(false);

		for (int i = 0; i < 16; i++)
		{
			Judoka* j = new Judoka(GetRandomName(), GetRandomName(), 50, Gender::Male);
			tourney.AddParticipant(j);
		}

		Pool* pool = new Pool(Weight(10), Weight(100));
		tourney.AddMatchTable(pool);

		ASSERT_TRUE(tourney.FindMatchTable(pool->GetUUID()));
		ASSERT_TRUE(pool->GetPool(0));

		ASSERT_TRUE(tourney.FindMatchTable(pool->GetPool(0)->GetUUID()));
		EXPECT_TRUE(tourney.FindMatchTable(pool->GetPool(0)->GetUUID())->IsSubMatchTable());
		ASSERT_TRUE(tourney.FindMatchTable(pool->GetPool(1)->GetUUID()));
		EXPECT_TRUE(tourney.FindMatchTable(pool->GetPool(1)->GetUUID())->IsSubMatchTable());
		ASSERT_TRUE(tourney.FindMatchTable(pool->GetPool(2)->GetUUID()));
		EXPECT_TRUE(tourney.FindMatchTable(pool->GetPool(2)->GetUUID())->IsSubMatchTable());
		ASSERT_TRUE(tourney.FindMatchTable(pool->GetPool(3)->GetUUID()));
		EXPECT_TRUE(tourney.FindMatchTable(pool->GetPool(3)->GetUUID())->IsSubMatchTable());

		ASSERT_TRUE(tourney.FindMatchTable(pool->GetFinals().GetUUID()));
		EXPECT_TRUE(tourney.FindMatchTable(pool->GetFinals().GetUUID())->IsSubMatchTable());
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, RuleSetHasSameIDAsInDatabase)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);
		d.AddRuleSet(new RuleSet("Test", 10, 20, 30, 40));

		ZED::Core::RemoveFile("tournaments/deleteMe.yml");
		Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
		tourney.Reset();

		Judoka j1("Firstname",  "Lastname",  50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);

		Match* m = new Match(&j1, &j2, &tourney);
		m->SetRuleSet(d.FindRuleSetByName("Test"));
		tourney.AddMatch(m);

		tourney.Save();


		Tournament t("deleteMe");
		t.EnableAutoSave(false);

		ASSERT_TRUE(t.FindRuleSetByName("Test"));
		ASSERT_TRUE(d.FindRuleSetByName("Test"));
		EXPECT_EQ(t.FindRuleSetByName("Test")->GetUUID(), d.FindRuleSetByName("Test")->GetUUID());
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, ConcludedStatus)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);

		Tournament* tourney = new Tournament("deleteMe");
		tourney->EnableAutoSave(false);

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));

		tourney->AddMatchTable(new RoundRobin(Weight(50), Weight(55)));
		tourney->GetMatchTables()[0]->IsBestOfThree(true);
		tourney->GetMatchTables()[0]->SetMatID(1);
		tourney->GenerateSchedule();

		ASSERT_EQ(tourney->GetSchedule().size(), 3);

		Mat mat(1);

		mat.StartMatch(tourney->GetSchedule()[0]);
		mat.AddIppon(Fighter::White);
		mat.EndMatch();

		mat.StartMatch(tourney->GetSchedule()[1]);
		mat.AddIppon(Fighter::Blue);
		mat.EndMatch();

		EXPECT_EQ(tourney->GetSchedule()[2]->GetStatus(), Status::Skipped);
		EXPECT_EQ(tourney->GetStatus(), Status::Concluded);

		delete tourney;
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, TestData_Randori2022)
{
	initialize();

	{
		Tournament t("../test-data/randori-2022");

		EXPECT_EQ(t.GetDatabase().GetYear(), 2022);

		ASSERT_EQ(t.GetMatchTables().size(), 3);

		auto results = t.GetMatchTables()[0]->CalculateResults();
		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Theo");
		EXPECT_EQ(results[0].Wins, 2);
		EXPECT_EQ(results[0].Score, 14);

		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Richard");
		EXPECT_EQ(results[1].Wins, 1);
		EXPECT_EQ(results[1].Score, 10);

		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Joris");
		EXPECT_EQ(results[2].Wins, 0);
		EXPECT_EQ(results[2].Score, 0);

		results = t.GetMatchTables()[1]->CalculateResults();
		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Julius");
		EXPECT_EQ(results[0].Wins, 2);
		EXPECT_EQ(results[0].Score, 20);

		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Tom");
		EXPECT_EQ(results[1].Wins, 0);
		EXPECT_EQ(results[1].Score, 0);

		results = t.GetMatchTables()[2]->CalculateResults();
		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Maja");
		EXPECT_EQ(results[0].Wins, 2);
		EXPECT_EQ(results[0].Score, 17);

		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Clara");
		EXPECT_EQ(results[1].Wins, 1);
		EXPECT_EQ(results[1].Score, 1);

		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Leni");
		EXPECT_EQ(results[2].Wins, 0);
		EXPECT_EQ(results[2].Score, 0);
	}

	ZED::Core::RemoveFile("tournaments/randori-2022.yml");
}



TEST(Tournament, StressTest_DeletionMovingAdding)
{
	initialize();

	{
		auto tournament_name = GetRandomName();
		auto tourney = new Tournament(tournament_name, new RuleSet("Test", 3 * 60, 3 * 60, 20, 10));

		Judoka* j[100];

		for (int i = 0; i < 100; i++)
		{
			j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
			tourney->AddParticipant(j[i]);
		}

		MatchTable* m1 = new RoundRobin(Weight(0), Weight(60));
		MatchTable* m2 = new RoundRobin(Weight(60), Weight(70));
		MatchTable* m3 = new RoundRobin(Weight(70), Weight(80));
		MatchTable* m4 = new RoundRobin(Weight(80), Weight(90));
		MatchTable* m5 = new RoundRobin(Weight(90), Weight(100));

		tourney->AddMatchTable(m1);
		tourney->AddMatchTable(m2);
		tourney->AddMatchTable(m3);
		tourney->AddMatchTable(m4);
		tourney->AddMatchTable(m5);


		auto adder = [&]() {
			for (int i = 0; i < 1000; i++)
			{
				Match* new_match = new Match(j[rand() % 100], j[rand() % 100], nullptr);
				tourney->AddMatch(new_match);
			}
		};

		auto swapper = [&]() {
			for (int i = 0; i < 1000; i++)
			{
				auto schedule = tourney->GetSchedule();

				if (rand() % 2 == 0)
					tourney->MoveMatchUp(schedule[rand() % schedule.size()]->GetUUID());
				else
					tourney->MoveMatchDown(schedule[rand() % schedule.size()]->GetUUID());
			}
		};

		auto remover = [&]() {
			for (int i = 0; i < 1000; i++)
			{
				auto schedule = tourney->GetSchedule();
				if (schedule.size() > 0)
					tourney->RemoveMatch(schedule[rand() % schedule.size()]->GetUUID());
			}
		};

		for (int k = 0; k < 5; k++)
		{
			std::thread add[10];
			std::thread rem[10];
			std::thread swp[10];

			for (int i = 0; i < 10; i++)
				add[i] = std::thread(adder);
			for (int i = 0; i < 10; i++)
				rem[i] = std::thread(remover);
			for (int i = 0; i < 10; i++)
				swp[i] = std::thread(swapper);


			for (int i = 0; i < 10; i++)
				add[i].join();
			for (int i = 0; i < 10; i++)
				rem[i].join();
			for (int i = 0; i < 10; i++)
				swp[i].join();
		}
	}
}



TEST(Tournament, SwapMatches_Randori2022)
{
	initialize();

	{
		Tournament t("../test-data/randori-2022");

		EXPECT_EQ(t.GetDatabase().GetYear(), 2022);

		ASSERT_EQ(t.GetMatchTables().size(), 3);

		t.SwapAllFighters();

		auto results = t.GetMatchTables()[0]->CalculateResults();
		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Joris");
		EXPECT_EQ(results[0].Wins,   2);
		EXPECT_EQ(results[0].Score, 17);

		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Richard");
		EXPECT_EQ(results[1].Wins,  1);
		EXPECT_EQ(results[1].Score, 7);

		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Theo");
		EXPECT_EQ(results[2].Wins,  0);
		EXPECT_EQ(results[2].Score, 0);

		results = t.GetMatchTables()[1]->CalculateResults();
		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Tom");
		EXPECT_EQ(results[0].Wins,   2);
		EXPECT_EQ(results[0].Score, 20);

		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Julius");
		EXPECT_EQ(results[1].Wins,  0);
		EXPECT_EQ(results[1].Score, 0);

		results = t.GetMatchTables()[2]->CalculateResults();
		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leni");
		EXPECT_EQ(results[0].Wins,  2);
		EXPECT_EQ(results[0].Score, 8);

		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Clara");
		EXPECT_EQ(results[1].Wins,   1);
		EXPECT_EQ(results[1].Score, 10);

		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maja");
		EXPECT_EQ(results[2].Wins,  0);
		EXPECT_EQ(results[2].Score, 0);
	}

	ZED::Core::RemoveFile("tournaments/randori-2022.yml");
}



TEST(Tournament, SaveAndLoad)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_EQ(d.GetNumJudoka(), 0);

		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1->GetUUID(), j2->GetUUID());

		d.AddJudoka(j1);
		d.AddJudoka(j2);
		d.AddJudoka(j3);
		d.AddJudoka(j4);

		Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
		tourney->Reset();

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		tourney->MarkedAsWeighted(*j1);
		tourney->MarkedAsWeighted(*j2);

		tourney->AddMatchTable(new RoundRobin(Weight(50), Weight(55)));
		tourney->AddMatchTable(new RoundRobin(Weight(60), Weight(65)));
		tourney->AddMatchTable(new Pool(Weight(50), Weight(65)));
		tourney->AddMatchTable(new SingleElimination(Weight(50), Weight(65)));
		tourney->AddMatchTable(new DoubleElimination(Weight(50), Weight(65)));
		tourney->AddMatch(new Match(j1, j3, tourney, 1));
		tourney->AddMatch(new Match(j1, j4, tourney, 2));
		tourney->GenerateSchedule();

		tourney->Disqualify(*j1);

		tourney->IsReadonly(true);

		EXPECT_TRUE(tourney->IsReadonly());

		tourney->Save();


		Tournament t("deleteMe");
		t.EnableAutoSave(false);

		EXPECT_EQ(t.IsReadonly(), tourney->IsReadonly());

		EXPECT_EQ(t.GetName(), "deleteMe");
		EXPECT_EQ(t.GetParticipants().size(), 4);
		EXPECT_EQ(t.GetMatchTables().size(), tourney->GetMatchTables().size());
		EXPECT_EQ(t.GetSchedule().size(),    tourney->GetSchedule().size());

		EXPECT_TRUE( t.IsMarkedAsWeighted(*j1));
		EXPECT_TRUE( t.IsMarkedAsWeighted(*j2));
		EXPECT_FALSE(t.IsMarkedAsWeighted(*j3));
		EXPECT_FALSE(t.IsMarkedAsWeighted(*j4));

		EXPECT_TRUE( t.IsDisqualified(*j1));
		EXPECT_FALSE(t.IsDisqualified(*j2));
		EXPECT_FALSE(t.IsDisqualified(*j3));
		EXPECT_FALSE(t.IsDisqualified(*j4));

		delete tourney;
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, WeightclassesAreSorted)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	for (int i = 0; i < 100; ++i)
	{
		Tournament tourney("deleteMe");
		tourney.EnableAutoSave(false);

		for (int j = 0; j < 30; ++j)
		{
			auto w = rand() % 100;
			tourney.AddMatchTable(new RoundRobin(Weight(w), Weight(w + 10)));
		}
		
		auto tables = tourney.GetMatchTables();
		int current = 0;
		for (auto table : tables)
		{
			EXPECT_LE(current, (int)((Weightclass*)table->GetFilter())->GetMinWeight());

			current = (int)((Weightclass*)table->GetFilter())->GetMinWeight();
		}
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, SaveAndLoad_MatchTableConnection)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_EQ(d.GetNumJudoka(), 0);

		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1->GetUUID(), j2->GetUUID());

		d.AddJudoka(j1);
		d.AddJudoka(j2);
		d.AddJudoka(j3);
		d.AddJudoka(j4);

		Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
		tourney->Reset();

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		tourney->SetDescription("test description");

		tourney->AddMatchTable(new RoundRobin(Weight(50), Weight(155)));
		tourney->GenerateSchedule();

		tourney->Save();


		Tournament t("deleteMe");

		EXPECT_EQ(t.GetName(), "deleteMe");
		EXPECT_EQ(t.GetDescription(), tourney->GetDescription());
		EXPECT_EQ(t.GetParticipants().size(), 4);
		EXPECT_EQ(t.GetMatchTables().size(), 1);
		EXPECT_EQ(t.GetSchedule().size(), 6);

		for (auto match : t.GetSchedule())
		{
			ASSERT_TRUE(match->GetMatchTable());
			EXPECT_EQ(match->GetMatchTable()->GetUUID(), tourney->GetMatchTables()[0]->GetUUID());
		}
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, SaveAndLoad_AgeGroups)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		j1->SetBirthyear(2000);
		j2->SetBirthyear(2000);
		j3->SetBirthyear(2000);
		j4->SetBirthyear(2000);

		ZED::Core::RemoveFile("tournaments/deleteMe.yml");
		Tournament tourney("deleteMe");

		AgeGroup a("AgeGroup", 20, 1000, nullptr);

		tourney.AddAgeGroup(&a);
		EXPECT_TRUE(tourney.AddParticipant(j1));
		EXPECT_TRUE(tourney.AddParticipant(j2));
		EXPECT_TRUE(tourney.AddParticipant(j3));
		EXPECT_TRUE(tourney.AddParticipant(j4));
		tourney.Save();

		Tournament t("deleteMe");

		EXPECT_EQ(t.GetName(), "deleteMe");
		ASSERT_EQ(t.GetParticipants().size(), 4);

		for (auto j : t.GetParticipants())
		{
			auto age_group = t.GetAgeGroupOfJudoka(j);

			ASSERT_TRUE(age_group);
			EXPECT_EQ(age_group->GetUUID(), a.GetUUID());
			EXPECT_EQ(age_group->GetName(), a.GetName());
		}
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, SaveAndLoad_Clubs)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Club c("Club name");

		Judoka j1("Firstname",  "Lastname",  50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka j3("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka j4("Firstname4", "Lastname4", 61, Gender::Male);

		j1.SetClub(&c);
		j2.SetClub(&c);
		j3.SetClub(&c);
		j4.SetClub(&c);

		ZED::Core::RemoveFile("tournaments/deleteMe.yml");
		Tournament tourney("deleteMe");

		EXPECT_TRUE(tourney.AddParticipant(&j1));
		EXPECT_TRUE(tourney.AddParticipant(&j2));
		EXPECT_TRUE(tourney.AddParticipant(&j3));
		EXPECT_TRUE(tourney.AddParticipant(&j4));
		tourney.Save();

		Tournament t("deleteMe");

		EXPECT_EQ(t.GetName(), "deleteMe");
		ASSERT_EQ(t.GetParticipants().size(), 4);

		for (auto j : t.GetParticipants())
		{
			ASSERT_TRUE(j->GetClub());
			EXPECT_EQ(j->GetClub()->GetUUID(), c.GetUUID());
			EXPECT_EQ(j->GetClub()->GetName(), c.GetName());
		}
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, SaveAndLoad_AutoMatches)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_EQ(d.GetNumJudoka(), 0);

		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1->GetUUID(), j2->GetUUID());

		d.AddJudoka(j1);
		d.AddJudoka(j2);
		d.AddJudoka(j3);
		d.AddJudoka(j4);

		Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
		tourney->Reset();

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		tourney->AddMatchTable(new RoundRobin(Weight(50), Weight(55)));
		tourney->AddMatchTable(new RoundRobin(Weight(60), Weight(65)));
		tourney->AddMatch(new Match(j1, j3, tourney, 2));
		tourney->AddMatch(new Match(j1, j4, tourney, 2));
		tourney->GenerateSchedule();

		tourney->Save();


		Tournament t("deleteMe");
		
		EXPECT_EQ(t.GetParticipants().size(), 4);
		ASSERT_EQ(t.GetMatchTables().size(), 4);
		EXPECT_EQ(t.GetSchedule().size(), 4);

		ASSERT_EQ(t.GetMatchTables()[0]->GetSchedule().size(), 1);
		ASSERT_EQ(t.GetMatchTables()[1]->GetSchedule().size(), 1);

		delete tourney;
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}




TEST(Tournament, ChangeScheduleIndexAfterDeletion)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_EQ(d.GetNumJudoka(), 0);

		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1->GetUUID(), j2->GetUUID());

		d.AddJudoka(j1);
		d.AddJudoka(j2);
		d.AddJudoka(j3);
		d.AddJudoka(j4);

		Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
		tourney->Reset();
		tourney->EnableAutoSave(false);

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		auto w1 = new RoundRobin(Weight(50), Weight(55));
		auto w2 = new RoundRobin(Weight(60), Weight(65));
		auto w3 = new RoundRobin(Weight(70), Weight(80));
		tourney->AddMatchTable(w1);
		tourney->AddMatchTable(w2);
		tourney->AddMatchTable(w3);

		tourney->RemoveMatchTable(w2->GetUUID());		
		tourney->GenerateSchedule();

		EXPECT_EQ(w1->GetScheduleIndex(), 0);
		EXPECT_EQ(w3->GetScheduleIndex(), 1);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, ChangeTopmostScheduleIndexUp)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1->GetUUID(), j2->GetUUID());

		Tournament* tourney = new Tournament("deleteMe");
		tourney->Reset();
		tourney->Save();

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		auto w1 = new RoundRobin(Weight(50), Weight(55));
		auto w2 = new RoundRobin(Weight(60), Weight(65));
		auto w3 = new RoundRobin(Weight(70), Weight(80));
		tourney->AddMatchTable(w1);
		tourney->AddMatchTable(w2);
		tourney->AddMatchTable(w3);

		tourney->GenerateSchedule();

		EXPECT_EQ(w1->GetScheduleIndex(), 0);
		EXPECT_EQ(w2->GetScheduleIndex(), 1);
		EXPECT_EQ(w3->GetScheduleIndex(), 2);

		EXPECT_TRUE(tourney->MoveScheduleEntryUp(*w1));

		EXPECT_EQ(w1->GetScheduleIndex(), 0);
		EXPECT_EQ(w2->GetScheduleIndex(), 1);
		EXPECT_EQ(w3->GetScheduleIndex(), 2);

		EXPECT_TRUE(tourney->MoveScheduleEntryUp(*w2));

		EXPECT_EQ(w1->GetScheduleIndex(), 0);
		EXPECT_EQ(w2->GetScheduleIndex(), 0);
		EXPECT_EQ(w3->GetScheduleIndex(), 1);

		EXPECT_TRUE(tourney->MoveScheduleEntryUp(*w1));

		EXPECT_EQ(w1->GetScheduleIndex(), 0);
		EXPECT_EQ(w2->GetScheduleIndex(), 1);
		EXPECT_EQ(w3->GetScheduleIndex(), 2);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, ChangeScheduleIndexAfterChangingMat)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_EQ(d.GetNumJudoka(), 0);

		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1->GetUUID(), j2->GetUUID());

		d.AddJudoka(j1);
		d.AddJudoka(j2);
		d.AddJudoka(j3);
		d.AddJudoka(j4);

		Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
		tourney->Reset();
		tourney->Save();

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		auto w1 = new RoundRobin(Weight(50), Weight(55));
		auto w2 = new RoundRobin(Weight(60), Weight(65));
		auto w3 = new RoundRobin(Weight(70), Weight(80));
		tourney->AddMatchTable(w1);
		tourney->AddMatchTable(w2);
		tourney->AddMatchTable(w3);

		w2->SetMatID(2);
		tourney->GenerateSchedule();

		EXPECT_EQ(w1->GetScheduleIndex(), 0);
		EXPECT_EQ(w2->GetScheduleIndex(), 1);
		EXPECT_EQ(w3->GetScheduleIndex(), 2);

		tourney->RemoveMatchTable(w2->GetUUID());
		tourney->GenerateSchedule();

		EXPECT_EQ(w1->GetScheduleIndex(), 0);
		EXPECT_EQ(w3->GetScheduleIndex(), 1);

		w3->SetMatID(1);
		tourney->GenerateSchedule();

		EXPECT_EQ(w1->GetScheduleIndex(), 0);
		EXPECT_EQ(w3->GetScheduleIndex(), 1);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, NoReorderWhenAddingCustom)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_EQ(d.GetNumJudoka(), 0);

		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		d.AddJudoka(j1);
		d.AddJudoka(j2);
		d.AddJudoka(j3);
		d.AddJudoka(j4);

		Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
		tourney->Reset();
		tourney->Save();

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		auto w1 = new RoundRobin(Weight(50), Weight(55));
		auto w2 = new RoundRobin(Weight(60), Weight(65));
		auto w3 = new RoundRobin(Weight(70), Weight(80));
		tourney->AddMatchTable(w1);
		tourney->AddMatchTable(w2);
		tourney->AddMatchTable(w3);

		tourney->GenerateSchedule();

		auto match1 = tourney->GetSchedule()[0];
		auto match2 = tourney->GetSchedule()[1];

		EXPECT_TRUE(tourney->MoveMatchUp(*match2));
		EXPECT_EQ(*tourney->GetSchedule()[0], *match2);
		EXPECT_EQ(*tourney->GetSchedule()[1], *match1);

		EXPECT_TRUE(tourney->AddMatch(new Match(j1, j2, tourney)));

		EXPECT_EQ(*tourney->GetSchedule()[0], *match2);
		EXPECT_EQ(*tourney->GetSchedule()[1], *match1);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, SaveAndLoad_SingleElimination)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_EQ(d.GetNumJudoka(), 0);

		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1->GetUUID(), j2->GetUUID());

		d.AddJudoka(j1);
		d.AddJudoka(j2);
		d.AddJudoka(j3);
		d.AddJudoka(j4);

		Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
		tourney->Reset();

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		tourney->AddMatchTable(new SingleElimination(Weight(10), Weight(105)));
		tourney->GenerateSchedule();

		EXPECT_EQ(tourney->GetParticipants().size(), 4);
		ASSERT_EQ(tourney->GetMatchTables().size(), 1);
		ASSERT_EQ(tourney->GetSchedule().size(), 3);

		tourney->Save();

		Tournament t("deleteMe");

		EXPECT_EQ(t.GetParticipants().size(), 4);
		ASSERT_EQ(t.GetMatchTables().size(), 1);
		ASSERT_EQ(t.GetSchedule().size(), 3);

		EXPECT_TRUE(t.GetSchedule()[0]->GetMatchTable());
		EXPECT_TRUE(t.GetSchedule()[1]->GetMatchTable());
		EXPECT_TRUE(t.GetSchedule()[2]->GetMatchTable());

		ASSERT_EQ(t.GetSchedule()[2]->GetDependentMatches().size(), 2);
		ASSERT_TRUE(t.GetSchedule()[2]->GetDependentMatches()[0]);
		ASSERT_TRUE(t.GetSchedule()[2]->GetDependentMatches()[1]);

		EXPECT_EQ(t.GetSchedule()[2]->GetDependentMatches()[0]->GetUUID(), t.GetSchedule()[0]->GetUUID());
		EXPECT_EQ(t.GetSchedule()[2]->GetDependentMatches()[1]->GetUUID(), t.GetSchedule()[1]->GetUUID());

		delete tourney;
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, SaveAndLoad_DoubleElimination)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		Tournament* tourney = new Tournament("deleteMe");
		tourney->Reset();

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		tourney->AddMatchTable(new DoubleElimination(Weight(10), Weight(105)));
		tourney->GenerateSchedule();

		EXPECT_EQ(tourney->GetParticipants().size(), 4);
		ASSERT_EQ(tourney->GetMatchTables().size(), 1);
		ASSERT_EQ(tourney->GetSchedule().size(), 4);

		tourney->Save();

		Tournament t("deleteMe");

		EXPECT_EQ(t.GetParticipants().size(), 4);
		ASSERT_EQ(t.GetMatchTables().size(), 1);
		ASSERT_EQ(t.GetSchedule().size(), tourney->GetSchedule().size());

		EXPECT_TRUE(t.GetSchedule()[0]->GetMatchTable());
		EXPECT_TRUE(t.GetSchedule()[1]->GetMatchTable());
		EXPECT_TRUE(t.GetSchedule()[2]->GetMatchTable());
		EXPECT_TRUE(t.GetSchedule()[3]->GetMatchTable());

		delete tourney;
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, PruneUnusedClubs)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		Club* c1 = new Club("Club 1");
		Club* c2 = new Club("Club 1");

		Tournament* tourney = new Tournament("deleteMe");
		tourney->Reset();

		j1->SetClub(c1);
		j2->SetClub(c1);
		j3->SetClub(c1);
		j4->SetClub(c2);

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));
		EXPECT_TRUE(tourney->AddParticipant(j3));
		EXPECT_TRUE(tourney->AddParticipant(j4));

		tourney->Save();

		j4->SetClub(c1);

		EXPECT_EQ(tourney->GetDatabase().GetAllClubs().size(), 2);

		tourney->Save();

		EXPECT_EQ(tourney->GetDatabase().GetAllClubs().size(), 1);

		Tournament* tourney2 = new Tournament("deleteMe");

		EXPECT_EQ(tourney2->GetDatabase().GetAllClubs().size(), 1);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, AddMatchAfterConclusion)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_TRUE(d.GetNumJudoka() == 0);

		Judoka* j1 = new Judoka("Firstname", "Lastname", 50, Gender::Male);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

		d.AddJudoka(j1);
		d.AddJudoka(j2);
		d.AddJudoka(j3);
		d.AddJudoka(j4);

		Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
		tourney.Reset();
		tourney.EnableAutoSave(false);

		EXPECT_TRUE(tourney.AddParticipant(j1));
		EXPECT_TRUE(tourney.AddParticipant(j2));
		EXPECT_TRUE(tourney.AddParticipant(j3));
		EXPECT_TRUE(tourney.AddParticipant(j4));

		//auto match1 = new Match(j1, j3, &tourney, 1);
		auto match1 = new Match(j1, j3, nullptr);
		auto match2 = new Match(j1, j4, &tourney, 1);

		EXPECT_FALSE(match1->GetTournament());
		EXPECT_TRUE(tourney.AddMatch(match1));
		EXPECT_TRUE(match1->GetTournament());
		EXPECT_EQ(match1->GetMatID(), 1);

		auto mat = new Mat(1);
		mat->StartMatch(match1);

		mat->AddIppon(Fighter::White);
		mat->EndMatch();

		EXPECT_TRUE(tourney.AddMatch(match2));

		tourney.IsReadonly(true);

		EXPECT_FALSE(tourney.AddMatch(match2));
		delete mat;
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, AddMatchAfterConclusionForTemporaryTournaments)
{
	initialize();

	Database d;
	d.EnableAutoSave(false);

	EXPECT_TRUE(d.GetNumJudoka() == 0);

	Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
	Judoka* j2 = new Judoka("Firstname2", "Lastname2", 51, Gender::Male);
	Judoka* j3 = new Judoka("Firstname3", "Lastname3", 60, Gender::Male);
	Judoka* j4 = new Judoka("Firstname4", "Lastname4", 61, Gender::Male);

	d.AddJudoka(j1);
	d.AddJudoka(j2);
	d.AddJudoka(j3);
	d.AddJudoka(j4);

	Tournament tourney("", d.FindRuleSetByName("Default"));//Temporary tournament
	tourney.Reset();
	tourney.EnableAutoSave(false);

	EXPECT_TRUE(tourney.AddParticipant(j1));
	EXPECT_TRUE(tourney.AddParticipant(j2));
	EXPECT_TRUE(tourney.AddParticipant(j3));
	EXPECT_TRUE(tourney.AddParticipant(j4));

	auto match1 = new Match(j1, j3, &tourney, 1);
	auto match2 = new Match(j1, j4, &tourney, 1);
	auto match3 = new Match(j1, j4, &tourney, 1);

	EXPECT_TRUE(tourney.AddMatch(match1));

	auto mat = new Mat(1);
	mat->StartMatch(match1);

	mat->AddIppon(Fighter::White);
	mat->EndMatch();

	EXPECT_TRUE(tourney.AddMatch(match2));

	tourney.IsReadonly(true);//Temp tournament can not be read only

	EXPECT_TRUE(tourney.AddMatch(match3));

	delete mat;
}



TEST(Tournament, AddParticpantsWhenAddingMatchTable)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	Tournament t("deleteMe");
	t.EnableAutoSave(false);

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	auto w = new RoundRobin(Weight(10), Weight(100));

	EXPECT_TRUE(w->AddParticipant(&j1));
	EXPECT_TRUE(w->AddParticipant(&j2));

	EXPECT_EQ(w->GetSchedule().size(), 1);

	t.AddMatchTable(w);

	EXPECT_EQ(t.GetParticipants().size(), 2);
	EXPECT_EQ(t.GetSchedule().size(),     1);
}