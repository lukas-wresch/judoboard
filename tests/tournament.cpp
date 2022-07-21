#include "tests.h"



TEST(Tournament, DontDuplicateParticipants)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_TRUE(d.GetNumJudoka() == 0);

		Judoka j1("Firstname", "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 60, Gender::Female);

		d.AddJudoka(&j1);
		d.AddJudoka(&j2);

		Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
		tourney.Reset();
		tourney.EnableAutoSave(false);

		EXPECT_TRUE(tourney.AddParticipant(&j1));
		EXPECT_TRUE(tourney.GetParticipants().size() == 1);
		EXPECT_FALSE(tourney.AddParticipant(&j1));
		EXPECT_TRUE(tourney.GetParticipants().size() == 1);

		EXPECT_TRUE(tourney.AddParticipant(&j2));
		EXPECT_TRUE(tourney.GetParticipants().size() == 2);
		EXPECT_FALSE(tourney.AddParticipant(&j2));
		EXPECT_TRUE(tourney.GetParticipants().size() == 2);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe");
}




TEST(Tournament, MatchAgainstOneself)
{
	initialize();
	Database d;
	d.EnableAutoSave(false);


	Judoka j1("Firstname", "Lastname", 50, Gender::Male);

	d.AddJudoka(&j1);

	Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
	tourney->Reset();
	tourney->EnableAutoSave(false);

	EXPECT_FALSE(tourney->AddMatch(Match(tourney, &j1, &j1)));
}



TEST(Tournament, Disqualification)
{
	initialize();

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Match match(&tourney, &j1, &j2);
	tourney.AddMatch(&match);

	tourney.Disqualify(j1);

	EXPECT_TRUE(match.HasConcluded());
	EXPECT_TRUE(match.GetMatchResult().m_Winner == Winner::Blue);
	EXPECT_TRUE(match.GetMatchResult().m_Score == Match::Score::Ippon);
	EXPECT_TRUE(match.GetMatchResult().m_Time == 0);
	EXPECT_TRUE(match.GetLog().GetEvents().size() == 0);

	EXPECT_TRUE(tourney.IsDisqualified(j1));
	EXPECT_FALSE(tourney.IsDisqualified(j2));
}



TEST(Tournament, AddMatchAfterDisqualification)
{
	initialize();

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);
	Judoka j3("Firstname3", "Lastname3", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Match match1(&tourney, &j1, &j2);
	tourney.AddMatch(&match1);
	Match matchdummy(&tourney, &j2, &j3);
	tourney.AddMatch(&matchdummy);

	tourney.Disqualify(j1);

	Match match2(&tourney, &j1, &j2);
	tourney.AddMatch(&match2);

	EXPECT_TRUE(match1.HasConcluded());
	EXPECT_EQ(match1.GetMatchResult().m_Winner, Winner::Blue);
	EXPECT_EQ(match1.GetMatchResult().m_Score, Match::Score::Ippon);
	EXPECT_EQ(match1.GetMatchResult().m_Time, 0);
	EXPECT_EQ(match1.GetLog().GetEvents().size(), 0);

	EXPECT_TRUE(match2.HasConcluded());
	EXPECT_EQ(match2.GetMatchResult().m_Winner, Winner::Blue);
	EXPECT_EQ(match2.GetMatchResult().m_Score, Match::Score::Ippon);
	EXPECT_EQ(match2.GetMatchResult().m_Time, 0);
	EXPECT_EQ(match2.GetLog().GetEvents().size(), 0);
}



TEST(Tournament, DoubleDisqualification)
{
	initialize();

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Match match(&tourney, &j1, &j2);
	tourney.AddMatch(&match);

	tourney.Disqualify(j1);
	tourney.Disqualify(j2);

	EXPECT_TRUE(match.HasConcluded());
	EXPECT_TRUE(match.GetMatchResult().m_Winner == Winner::Draw);
	EXPECT_TRUE(match.GetMatchResult().m_Score == Match::Score::Draw);
	EXPECT_TRUE(match.GetMatchResult().m_Time == 0);
	EXPECT_TRUE(match.GetLog().GetEvents().size() == 0);
}



TEST(Tournament, DoubleDisqualification2)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe");

	{
		Judoka j1("Firstname", "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);

		Tournament tourney("deleteMe");
		tourney.Reset();
		tourney.EnableAutoSave(false);

		Match match1(&tourney, &j1, &j2, 1);
		Match match2(&tourney, &j1, &j2, 1);

		tourney.AddMatch(&match1);
		tourney.AddMatch(&match2);

		Mat m(1);

		EXPECT_TRUE(m.StartMatch(&match1));
		m.Hajime();
		m.AddIppon(Fighter::White);
		EXPECT_TRUE(m.EndMatch());

		tourney.Disqualify(j1);
		tourney.Disqualify(j2);

		EXPECT_TRUE(match1.HasConcluded());
		EXPECT_EQ(match1.GetMatchResult().m_Winner, Winner::White);
		EXPECT_EQ(match1.GetMatchResult().m_Score, Match::Score::Ippon);
		EXPECT_LE(match1.GetMatchResult().m_Time, 50u);
		EXPECT_GE(match1.GetLog().GetEvents().size(), 3u);

		EXPECT_TRUE(match2.HasConcluded());
		EXPECT_EQ(match2.GetMatchResult().m_Winner, Winner::Draw);
		EXPECT_EQ(match2.GetMatchResult().m_Score, Match::Score::Draw);
		EXPECT_LE(match2.GetMatchResult().m_Time, 50u);
		EXPECT_EQ(match2.GetLog().GetEvents().size(), 0u);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe");
}



TEST(Tournament, RevokeDisqualification)
{
	initialize();

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Match match(&tourney, &j1, &j2);
	tourney.AddMatch(&match);

	tourney.Disqualify(j1);
	tourney.RevokeDisqualification(j1);

	EXPECT_FALSE(match.HasConcluded());
	EXPECT_FALSE(tourney.IsDisqualified(j1));
}



TEST(Tournament, RevokeDoubleDisqualification)
{
	initialize();

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname2", "Lastname2", 50, Gender::Male);

	Tournament tourney("deleteMe");
	tourney.Reset();
	tourney.EnableAutoSave(false);

	Match match(&tourney, &j1, &j2);
	tourney.AddMatch(&match);

	tourney.Disqualify(j1);
	tourney.Disqualify(j2);
	tourney.RevokeDisqualification(j1);

	EXPECT_TRUE(match.HasConcluded());
	EXPECT_EQ(match.GetMatchResult().m_Winner, Winner::White);
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

		auto matchtable = new Weightclass(&tourney, 50, 60);
		matchtable->IsBestOfThree(true);
		tourney.AddMatchTable(matchtable);

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

		Judoka j1("Firstname", "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka j3("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka j4("Firstname4", "Lastname4", 61, Gender::Male);

		d.AddJudoka(&j1);
		d.AddJudoka(&j2);
		d.AddJudoka(&j3);
		d.AddJudoka(&j4);

		ZED::Core::RemoveFile("tournaments/deleteMe.yml");

		Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
		tourney.Reset();
		tourney.EnableAutoSave(false);

		EXPECT_TRUE(tourney.AddParticipant(&j1));
		EXPECT_TRUE(tourney.AddParticipant(&j2));
		EXPECT_TRUE(tourney.AddParticipant(&j3));
		EXPECT_TRUE(tourney.AddParticipant(&j4));

		tourney.AddMatchTable(new Weightclass(&tourney, 50, 55));
		tourney.AddMatchTable(new Weightclass(&tourney, 60, 65));

		EXPECT_TRUE(tourney.GetMatchTables().size() == 2);
		EXPECT_TRUE(tourney.GetMatchTables()[0]->GetColor() == Color::Name::Blue);
		EXPECT_TRUE(tourney.GetMatchTables()[1]->GetColor() == Color::Name::Red);
	}

	ZED::Core::RemoveFile("deleteMe.yml");
}



TEST(Tournament, ParticipantHasSameIDAsInDatabase)
{
	initialize();
	Database d;
	d.EnableAutoSave(false);

	EXPECT_TRUE(d.GetNumJudoka() == 0);

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);

	d.AddJudoka(&j1);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
	tourney.Reset();

	EXPECT_TRUE(tourney.AddParticipant(&j1));
	ASSERT_TRUE(tourney.FindParticipant(j1.GetUUID()));
	EXPECT_EQ(j1.GetUUID(), tourney.FindParticipant(j1.GetUUID())->GetUUID());
	//tourney gets saved now

	tourney.EnableAutoSave(false);


	Tournament t("deleteMe");
	t.EnableAutoSave(false);
	EXPECT_EQ(t.GetParticipants().size(), 1);
	ASSERT_TRUE(t.FindParticipant(j1.GetUUID()));
	EXPECT_EQ(j1.GetUUID(), t.FindParticipant(j1.GetUUID())->GetUUID());

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

	tourney.EnableAutoSave(false);


	Tournament t("deleteMe");
	//t.ConnectToDatabase(d);
	t.EnableAutoSave(false);

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
	tourney.Reset();

	tourney.EnableAutoSave(false);


	Tournament t("deleteMe");
	t.EnableAutoSave(false);

	ASSERT_TRUE(t.GetDefaultRuleSet());
	ASSERT_TRUE(d.FindRuleSetByName("Default"));
	EXPECT_EQ(t.GetDefaultRuleSet()->GetUUID(), d.FindRuleSetByName("Default")->GetUUID());

	EXPECT_EQ(t.GetDefaultRuleSet()->GetMatchTime(),         60);
	EXPECT_EQ(t.GetDefaultRuleSet()->GetGoldenScoreTime(),   30);
	EXPECT_EQ(t.GetDefaultRuleSet()->GetOsaeKomiTime(false), 20);
	EXPECT_EQ(t.GetDefaultRuleSet()->GetOsaeKomiTime(true),  10);

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

		ZED::Core::RemoveFile("tournaments/deleteMe");
		Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
		tourney.Reset();

		Judoka j1("Firstname", "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);

		Match m(&tourney, &j1, &j2);
		m.SetRuleSet(d.FindRuleSetByName("Test"));
		tourney.AddMatch(&m);

		tourney.EnableAutoSave(false);


		Tournament t("deleteMe");
		//t.ConnectToDatabase(d);
		t.EnableAutoSave(false);

		ASSERT_TRUE(t.FindRuleSetByName("Test"));
		ASSERT_TRUE(d.FindRuleSetByName("Test"));
		//EXPECT_EQ(t.FindRuleSetByName("Test")->GetID(), d.FindRuleSetByName("Test")->GetID());
		EXPECT_EQ(t.FindRuleSetByName("Test")->GetUUID(), d.FindRuleSetByName("Test")->GetUUID());
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Tournament, SaveAndLoad)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_EQ(d.GetNumJudoka(), 0);

		Judoka j1("Firstname",  "Lastname",  50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka j3("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka j4("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1.GetUUID(), j2.GetUUID());

		d.AddJudoka(&j1);
		d.AddJudoka(&j2);
		d.AddJudoka(&j3);
		d.AddJudoka(&j4);

		Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
		tourney->Reset();

		EXPECT_TRUE(tourney->AddParticipant(&j1));
		EXPECT_TRUE(tourney->AddParticipant(&j2));
		EXPECT_TRUE(tourney->AddParticipant(&j3));
		EXPECT_TRUE(tourney->AddParticipant(&j4));

		tourney->AddMatchTable(new Weightclass(tourney, 50, 55));
		tourney->AddMatchTable(new Weightclass(tourney, 60, 65));
		tourney->AddMatch(Match(tourney, &j1, &j3, 1));
		tourney->AddMatch(Match(tourney, &j1, &j4, 2));

		tourney->Disqualify(j1);

		tourney->EnableAutoSave(false);


		Tournament t("deleteMe");
		t.EnableAutoSave(false);

		EXPECT_EQ(t.GetName(), "deleteMe");
		EXPECT_EQ(t.GetParticipants().size(), 4);
		EXPECT_EQ(t.GetMatchTables().size(), 2);
		EXPECT_EQ(t.GetSchedule().size(), 4);

		EXPECT_TRUE(t.IsDisqualified(j1));
		EXPECT_FALSE(t.IsDisqualified(j2));
		EXPECT_FALSE(t.IsDisqualified(j3));
		EXPECT_FALSE(t.IsDisqualified(j4));
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

		Judoka j1("Firstname", "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka j3("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka j4("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1.GetUUID(), j2.GetUUID());

		d.AddJudoka(&j1);
		d.AddJudoka(&j2);
		d.AddJudoka(&j3);
		d.AddJudoka(&j4);

		Tournament* tourney = new Tournament("deleteMe", d.FindRuleSetByName("Default"));
		tourney->Reset();

		EXPECT_TRUE(tourney->AddParticipant(&j1));
		EXPECT_TRUE(tourney->AddParticipant(&j2));
		EXPECT_TRUE(tourney->AddParticipant(&j3));
		EXPECT_TRUE(tourney->AddParticipant(&j4));

		tourney->AddMatchTable(new Weightclass(tourney, 50, 55));
		tourney->AddMatchTable(new Weightclass(tourney, 60, 65));
		tourney->AddMatch(Match(tourney, &j1, &j3, 2));
		tourney->AddMatch(Match(tourney, &j1, &j4, 2));

		tourney->EnableAutoSave(false);


		Tournament t("deleteMe");
		t.EnableAutoSave(false);
		EXPECT_EQ(t.GetParticipants().size(), 4);
		ASSERT_EQ(t.GetMatchTables().size(), 2);
		EXPECT_EQ(t.GetSchedule().size(), 4);

		EXPECT_TRUE(t.GetMatchTables()[0]->GetSchedule()[0]->IsAutoGenerated());
		EXPECT_TRUE(t.GetMatchTables()[1]->GetSchedule()[0]->IsAutoGenerated());

		for (auto match : t.GetSchedule())
		{
			if (match->GetMatID() == 2)
				EXPECT_FALSE(match->IsAutoGenerated());
			else
				EXPECT_TRUE(match->IsAutoGenerated());
		}
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

		Judoka j1("Firstname", "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka j3("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka j4("Firstname4", "Lastname4", 61, Gender::Male);

		d.AddJudoka(&j1);
		d.AddJudoka(&j2);
		d.AddJudoka(&j3);
		d.AddJudoka(&j4);

		Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
		tourney.Reset();
		tourney.EnableAutoSave(false);

		EXPECT_TRUE(tourney.AddParticipant(&j1));
		EXPECT_TRUE(tourney.AddParticipant(&j2));
		EXPECT_TRUE(tourney.AddParticipant(&j3));
		EXPECT_TRUE(tourney.AddParticipant(&j4));

		auto match1 = Match(&tourney, &j1, &j3, 1);
		auto match2 = Match(&tourney, &j1, &j4, 1);

		EXPECT_TRUE(tourney.AddMatch(&match1));

		auto mat = new Mat(1);
		mat->StartMatch(&match1);

		mat->AddIppon(Fighter::White);
		mat->EndMatch();

		EXPECT_FALSE(tourney.AddMatch(&match2));
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

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);
	Judoka j3("Firstname3", "Lastname3", 60, Gender::Male);
	Judoka j4("Firstname4", "Lastname4", 61, Gender::Male);

	d.AddJudoka(&j1);
	d.AddJudoka(&j2);
	d.AddJudoka(&j3);
	d.AddJudoka(&j4);

	Tournament tourney("", d.FindRuleSetByName("Default"));//Temporary tournament
	tourney.Reset();
	tourney.EnableAutoSave(false);

	EXPECT_TRUE(tourney.AddParticipant(&j1));
	EXPECT_TRUE(tourney.AddParticipant(&j2));
	EXPECT_TRUE(tourney.AddParticipant(&j3));
	EXPECT_TRUE(tourney.AddParticipant(&j4));

	auto match1 = Match(&tourney, &j1, &j3, 1);
	auto match2 = Match(&tourney, &j1, &j4, 1);

	EXPECT_TRUE(tourney.AddMatch(&match1));

	auto mat = new Mat(1);
	mat->StartMatch(&match1);

	mat->AddIppon(Fighter::White);
	mat->EndMatch();

	EXPECT_TRUE(tourney.AddMatch(&match2));
	delete mat;
}