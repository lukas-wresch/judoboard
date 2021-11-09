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
		EXPECT_TRUE(match1.GetMatchResult().m_Winner == Winner::White);
		EXPECT_TRUE(match1.GetMatchResult().m_Score == Match::Score::Ippon);
		EXPECT_TRUE(match1.GetMatchResult().m_Time == 0);
		EXPECT_TRUE(match1.GetLog().GetEvents().size() >= 3);

		EXPECT_TRUE(match2.HasConcluded());
		EXPECT_TRUE(match2.GetMatchResult().m_Winner == Winner::Draw);
		EXPECT_TRUE(match2.GetMatchResult().m_Score == Match::Score::Draw);
		EXPECT_TRUE(match2.GetMatchResult().m_Time == 0);
		EXPECT_TRUE(match2.GetLog().GetEvents().size() == 0);
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
}



TEST(Tournament, ColorsForMatchTables)
{
	{
		initialize();
		Database d;
		d.Save("deleteMe.csv");
		d.Load("deleteMe.csv");

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

		tourney.AddMatchTable(new Weightclass(&tourney, 50, 55));
		tourney.AddMatchTable(new Weightclass(&tourney, 60, 65));

		EXPECT_TRUE(tourney.GetMatchTables().size() == 2);
		EXPECT_TRUE(tourney.GetMatchTables()[0]->GetColor() == Color::Name::Blue);
		EXPECT_TRUE(tourney.GetMatchTables()[1]->GetColor() == Color::Name::Red);
	}

	ZED::Core::RemoveFile("deleteMe.csv");
}



TEST(Tournament, ParticipantHasSameIDAsInDatabase)
{
	initialize();
	Database d;
	d.EnableAutoSave(false);

	EXPECT_TRUE(d.GetNumJudoka() == 0);

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);

	d.AddJudoka(&j1);

	ZED::Core::RemoveFile("tournaments\\deleteMe");
	Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
	tourney.Reset();

	EXPECT_TRUE(tourney.AddParticipant(&j1));
	//tourney gets saved now

	tourney.EnableAutoSave(false);


	Tournament t("deleteMe");
	t.ConnectToDatabase(d);
	t.EnableAutoSave(false);
	EXPECT_TRUE(t.GetParticipants().size() == 1);
	EXPECT_TRUE(t.FindParticipant(j1.GetUUID()));
	EXPECT_TRUE(t.FindParticipant(j1.GetID()));

	ZED::Core::RemoveFile("tournaments/deleteMe");
}



TEST(Tournament, HasDefaultRuleSet)
{
	initialize();
	Database d;
	d.EnableAutoSave(false);

	ZED::Core::RemoveFile("tournaments\\deleteMe");
	Tournament tourney("deleteMe", d.FindRuleSetByName("Default"));
	EXPECT_TRUE(tourney.AddParticipant(new Judoka("temp", "temp", 50)));
	tourney.Reset();

	tourney.EnableAutoSave(false);


	Tournament t("deleteMe");
	t.ConnectToDatabase(d);
	t.EnableAutoSave(false);

	EXPECT_TRUE(t.GetDefaultRuleSet());
	EXPECT_TRUE(d.FindRuleSetByName("Default"));
	EXPECT_TRUE(t.GetDefaultRuleSet()->GetID()   == d.FindRuleSetByName("Default")->GetID());
	EXPECT_TRUE(t.GetDefaultRuleSet()->GetUUID() == d.FindRuleSetByName("Default")->GetUUID());

	ZED::Core::RemoveFile("tournaments/deleteMe");
}


TEST(Tournament, RuleSetHasSameIDAsInDatabase)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe");

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
		t.ConnectToDatabase(d);
		t.EnableAutoSave(false);

		ASSERT_TRUE(t.FindRuleSetByName("Test"));
		ASSERT_TRUE(d.FindRuleSetByName("Test"));
		EXPECT_TRUE(t.FindRuleSetByName("Test")->GetID()   == d.FindRuleSetByName("Test")->GetID());
		EXPECT_TRUE(t.FindRuleSetByName("Test")->GetUUID() == d.FindRuleSetByName("Test")->GetUUID());
	}

	ZED::Core::RemoveFile("tournaments/deleteMe");
}



TEST(Tournament, SaveAndLoad)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe");

	{
		Database d;
		d.EnableAutoSave(false);

		EXPECT_TRUE(d.GetNumJudoka() == 0);

		Judoka j1("Firstname",  "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 51, Gender::Male);
		Judoka j3("Firstname3", "Lastname3", 60, Gender::Male);
		Judoka j4("Firstname4", "Lastname4", 61, Gender::Male);

		EXPECT_NE(j1.GetID(),   j2.GetID());
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

		tourney->EnableAutoSave(false);


		Tournament t("deleteMe");
		t.EnableAutoSave(false);
		EXPECT_TRUE(t.GetParticipants().size() == 4);
		EXPECT_TRUE(t.GetMatchTables().size() == 2);
		EXPECT_TRUE(t.GetSchedule().size() == 4);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe");
}



TEST(Tournament, AddMatchAfterConclusion)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe");

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

	ZED::Core::RemoveFile("tournaments/deleteMe");
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