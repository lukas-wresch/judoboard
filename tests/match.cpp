#include "tests.h"



TEST(Match, ExportImport)
{
	initialize();
	
	for (int i = 0; i < 1000; i++)
	{
		Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		int matid = rand();
		RuleSet rule_set("test", rand(), rand(), rand(), rand());
		Tournament tourney;

		Match* match = new Match(&j1, &j2, &tourney, matid);
		match->SetRuleSet(&rule_set);
		tourney.AddMatch(match);//Also copies the rule set inside the tournament's database

		YAML::Emitter yaml;
		*match >> yaml;

		Match match2(YAML::Load(yaml.c_str()), nullptr, &tourney);

		ASSERT_EQ(match->GetFighter(Fighter::White)->GetUUID(), match2.GetFighter(Fighter::White)->GetUUID());
		ASSERT_EQ(match->GetFighter(Fighter::Blue )->GetUUID(), match2.GetFighter(Fighter::Blue )->GetUUID());

		ASSERT_EQ(match->GetRuleSet().GetUUID(), match2.GetRuleSet().GetUUID());

		YAML::Emitter yaml1, yaml2;
		match->ToString(yaml1);
		match2.ToString(yaml2);
		ASSERT_EQ((std::string)yaml1.c_str(), (std::string)yaml2.c_str());

		ASSERT_EQ(match->GetUUID(),      match2.GetUUID());
		ASSERT_EQ(match->HasConcluded(), match2.HasConcluded());
		ASSERT_EQ(match->IsRunning(),    match2.IsRunning());
	}
}



TEST(Match, ExportImport_CopyConstructor)
{
	initialize();

	for (int i = 0; i < 1000; i++)
	{
		Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		int matid = rand();
		RuleSet rule_set("test", rand(), rand(), rand(), rand());
		Tournament tourney;

		Match* match = new Match(&j1, &j2, &tourney, matid);
		match->SetRuleSet(&rule_set);
		tourney.AddMatch(match);//Also copies the rule set inside the tournament's database

		Match match2(*match);

		ASSERT_EQ(match->GetFighter(Fighter::White)->GetUUID(), match2.GetFighter(Fighter::White)->GetUUID());
		ASSERT_EQ(match->GetFighter(Fighter::Blue )->GetUUID(), match2.GetFighter(Fighter::Blue )->GetUUID());

		ASSERT_EQ(match->GetRuleSet().GetUUID(), match2.GetRuleSet().GetUUID());

		YAML::Emitter yaml1, yaml2;
		match->ToString(yaml1);
		match2.ToString(yaml2);
		ASSERT_EQ((std::string)yaml1.c_str(), (std::string)yaml2.c_str());

		ASSERT_EQ(match->GetUUID(),      match2.GetUUID());
		ASSERT_EQ(match->HasConcluded(), match2.HasConcluded());
		ASSERT_EQ(match->IsRunning(),    match2.IsRunning());
	}
}



TEST(Match, BestOf3)
{
	initialize();

	Mat mat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));

		Tournament tourney;

		Match m1(&j1, &j2, &tourney, 1);
		Match m2(&j2, &j1, &tourney, 1);
		Match m3(&j1, &j2, &tourney, 1);

		m3.SetBestOfThree(&m1, &m2);

		EXPECT_FALSE(m1.HasDependentMatches());
		EXPECT_FALSE(m2.HasDependentMatches());
		EXPECT_TRUE(m3.HasDependentMatches());

		EXPECT_TRUE(m3.HasUnresolvedDependency());
		EXPECT_EQ(m3.GetStatus(), Status::Optional);

		mat.StartMatch(&m1);
		mat.AddIppon(f);
		mat.EndMatch();

		mat.StartMatch(&m2);
		mat.AddIppon(f);
		mat.EndMatch();

		EXPECT_FALSE(m3.HasUnresolvedDependency());
		EXPECT_EQ(m3.GetStatus(), Status::Scheduled);
	}
}



TEST(Match, BestOf3_2)
{
	initialize();

	Mat mat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));

		Tournament tourney;
		tourney.EnableAutoSave(false);

		Match m1(&j1, &j2, &tourney, 1);
		Match m2(&j2, &j1, &tourney, 1);
		Match m3(&j1, &j2, &tourney, 1);

		m3.SetBestOfThree(&m1, &m2);

		EXPECT_FALSE(m1.HasDependentMatches());
		EXPECT_FALSE(m2.HasDependentMatches());
		EXPECT_TRUE(m3.HasDependentMatches());

		EXPECT_TRUE(m3.HasUnresolvedDependency());
		EXPECT_EQ(m3.GetStatus(), Status::Optional);

		mat.StartMatch(&m1);
		mat.AddIppon(f);
		mat.EndMatch();

		mat.StartMatch(&m2);
		mat.AddIppon(!f);
		mat.EndMatch();

		EXPECT_FALSE(m3.HasUnresolvedDependency());
		EXPECT_EQ(m3.GetStatus(), Status::Skipped);
	}
}



TEST(Match, BestOf3_3)
{
	initialize();

	Mat mat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));

		Tournament tourney;
		tourney.EnableAutoSave(false);

		Match m1(&j1, &j2, &tourney, 1);
		Match m2(&j1, &j2, &tourney, 1);
		Match m3(&j1, &j2, &tourney, 1);

		m3.SetBestOfThree(&m1, &m2);

		EXPECT_FALSE(m1.HasDependentMatches());
		EXPECT_FALSE(m2.HasDependentMatches());
		EXPECT_TRUE(m3.HasDependentMatches());

		EXPECT_TRUE(m3.HasUnresolvedDependency());
		EXPECT_EQ(m3.GetStatus(), Status::Optional);

		mat.StartMatch(&m1);
		mat.AddIppon(f);
		mat.EndMatch();

		mat.StartMatch(&m2);
		mat.AddIppon(f);
		mat.EndMatch();

		EXPECT_FALSE(m3.HasUnresolvedDependency());
		EXPECT_EQ(m3.GetStatus(), Status::Skipped);
	}
}



TEST(Match, BestOf3ExportImport)
{
	initialize();

	Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
	Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	Mat mat(1);

	{
		Tournament tourney("deleteMe");

		Match* m1 = new Match(&j1, &j2, &tourney, 1);
		Match* m2 = new Match(&j2, &j1, &tourney, 1);
		Match* m3 = new Match(&j1, &j2, &tourney, 1);

		m3->SetBestOfThree(m1, m2);

		auto dep = m3->GetDependentMatches();
		ASSERT_EQ(dep.size(), 2);
		EXPECT_EQ(dep[0], m1);
		EXPECT_EQ(dep[1], m2);

		tourney.AddMatch(m1);
		tourney.AddMatch(m2);
		tourney.AddMatch(m3);

		tourney.Save();
		tourney.EnableAutoSave(false);
	}

	{
		Tournament tourney2("deleteMe");
		tourney2.EnableAutoSave(false);

		ASSERT_EQ(tourney2.GetSchedule().size(), 3);

		auto m1 = tourney2.GetSchedule()[0];
		auto m2 = tourney2.GetSchedule()[1];
		auto m3 = tourney2.GetSchedule()[2];

		EXPECT_FALSE(m1->HasDependentMatches());
		EXPECT_FALSE(m2->HasDependentMatches());
		EXPECT_TRUE(m3->HasDependentMatches());

		auto dep = m3->GetDependentMatches();
		ASSERT_EQ(dep.size(), 2);
		EXPECT_EQ(dep[0], m1);
		EXPECT_EQ(dep[1], m2);

		EXPECT_TRUE(m3->HasUnresolvedDependency());
		EXPECT_EQ(m3->GetStatus(), Status::Optional);

		mat.StartMatch(m1);
		mat.AddIppon(Fighter::White);
		mat.EndMatch();

		mat.StartMatch(m2);
		mat.AddIppon(Fighter::White);
		mat.EndMatch();

		EXPECT_FALSE(m3->HasUnresolvedDependency());
		EXPECT_EQ(m3->GetStatus(), Status::Scheduled);

		tourney2.EnableAutoSave(false);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}