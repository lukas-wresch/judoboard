#include "tests.h"



TEST(Match, NoDoubleEvents)
{
	initialize();

	Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
	Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));

	Match* match = new Match(&j1, &j2, nullptr, 1);
	Mat mat(1);

	EXPECT_TRUE(mat.StartMatch(match));

	mat.Hajime();
	mat.Hajime();

	mat.Mate();
	mat.Mate();

	auto log = match->GetLog();
	ASSERT_EQ(log.GetNumEvent(), 3);//Start, hajime, mate
	EXPECT_EQ(log.GetEvents()[0].m_Event, MatchLog::NeutralEvent::StartMatch);
	EXPECT_EQ(log.GetEvents()[1].m_Event, MatchLog::NeutralEvent::Hajime);
	EXPECT_EQ(log.GetEvents()[2].m_Event, MatchLog::NeutralEvent::Mate);

	mat.AddIppon(Fighter::White);
	mat.EndMatch();

	delete match;
}



TEST(Match, SwapEvents)
{
	initialize();

	Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
	Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));

	Match* match = new Match(&j1, &j2, nullptr, 1);
	Mat mat(1);

	EXPECT_EQ(*match->GetFighter(Fighter::White), j1);
	EXPECT_EQ(*match->GetFighter(Fighter::Blue),  j2);

	EXPECT_TRUE(mat.StartMatch(match));

	mat.Hajime();

	mat.AddIppon(Fighter::White);
	mat.RemoveIppon(Fighter::White);
	mat.AddIppon(Fighter::Blue);

	mat.Mate();
	mat.EndMatch();

	auto& log = match->GetLog();
	ASSERT_EQ(log.GetNumEvent(), 7);
	EXPECT_EQ(log.GetEvents()[0].m_Event, MatchLog::NeutralEvent::StartMatch);
	EXPECT_EQ(log.GetEvents()[1].m_Event, MatchLog::NeutralEvent::Hajime);
	EXPECT_EQ(log.GetEvents()[2].m_BiasedEvent, MatchLog::BiasedEvent::AddIppon);
	EXPECT_EQ(log.GetEvents()[2].m_Group, MatchLog::EventGroup::White);
	EXPECT_EQ(log.GetEvents()[3].m_Event, MatchLog::NeutralEvent::Mate);
	EXPECT_EQ(log.GetEvents()[4].m_BiasedEvent, MatchLog::BiasedEvent::RemoveIppon);
	EXPECT_EQ(log.GetEvents()[4].m_Group, MatchLog::EventGroup::White);
	EXPECT_EQ(log.GetEvents()[5].m_BiasedEvent, MatchLog::BiasedEvent::AddIppon);
	EXPECT_EQ(log.GetEvents()[5].m_Group, MatchLog::EventGroup::Blue);
	EXPECT_EQ(log.GetEvents()[6].m_Event, MatchLog::NeutralEvent::EndMatch);

	match->SwapFighters();
	EXPECT_EQ(*match->GetFighter(Fighter::White), j2);
	EXPECT_EQ(*match->GetFighter(Fighter::Blue),  j1);

	ASSERT_EQ(log.GetNumEvent(), 7);
	EXPECT_EQ(log.GetEvents()[0].m_Event, MatchLog::NeutralEvent::StartMatch);
	EXPECT_EQ(log.GetEvents()[1].m_Event, MatchLog::NeutralEvent::Hajime);
	EXPECT_EQ(log.GetEvents()[2].m_BiasedEvent, MatchLog::BiasedEvent::AddIppon);
	EXPECT_EQ(log.GetEvents()[2].m_Group, MatchLog::EventGroup::Blue);
	EXPECT_EQ(log.GetEvents()[3].m_Event, MatchLog::NeutralEvent::Mate);
	EXPECT_EQ(log.GetEvents()[4].m_BiasedEvent, MatchLog::BiasedEvent::RemoveIppon);
	EXPECT_EQ(log.GetEvents()[4].m_Group, MatchLog::EventGroup::Blue);
	EXPECT_EQ(log.GetEvents()[5].m_BiasedEvent, MatchLog::BiasedEvent::AddIppon);
	EXPECT_EQ(log.GetEvents()[5].m_Group, MatchLog::EventGroup::White);
	EXPECT_EQ(log.GetEvents()[6].m_Event, MatchLog::NeutralEvent::EndMatch);

	delete match;
}



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

		Match* match = nullptr;
		if (rand()%2 == 0)
			match = new Match(&j1, &j2, &tourney, matid);
		else
			match = new Match(DependentJudoka(&j1), DependentJudoka(&j2), &tourney, matid);

		EXPECT_TRUE(match->GetTournament());
		EXPECT_EQ(match->GetMatID(), matid);
		match->SetRuleSet(&rule_set);
		tourney.AddMatch(match);//Also copies the rule set inside the tournament's database

		YAML::Emitter yaml;
		*match >> yaml;

		Match match2(YAML::Load(yaml.c_str()), nullptr, &tourney);

		ASSERT_EQ(match->GetFighter(Fighter::White)->GetUUID(), match2.GetFighter(Fighter::White)->GetUUID());
		ASSERT_EQ(match->GetFighter(Fighter::Blue )->GetUUID(), match2.GetFighter(Fighter::Blue )->GetUUID());

		ASSERT_EQ(match->GetRuleSet().GetUUID(), match2.GetRuleSet().GetUUID());

		YAML::Emitter yaml1, yaml2;
		*match >> yaml1;
		match2 >> yaml2;
		ASSERT_EQ((std::string)yaml1.c_str(), (std::string)yaml2.c_str());

		ASSERT_EQ(match->GetUUID(),      match2.GetUUID());
		ASSERT_EQ(match->HasConcluded(), match2.HasConcluded());
		ASSERT_EQ(match->IsRunning(),    match2.IsRunning());
	}


	Match match(nullptr, nullptr, nullptr, 0);

	match.SetTag(Match::Tag::Finals());
	EXPECT_EQ((int)match.GetTag().value, 1);

	match.SetTag(Match::Tag::Semi());
	EXPECT_EQ((int)match.GetTag().value, 2);

	match.SetTag(Match::Tag::Third());
	EXPECT_EQ((int)match.GetTag().value, 4);

	match.SetTag(Match::Tag::Fifth());
	EXPECT_EQ((int)match.GetTag().value, 8);
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
		*match >> yaml1;
		match2 >> yaml2;
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

		ASSERT_EQ(tourney.GetMatchTables().size(), 1);
		auto& custom = tourney.GetMatchTables()[0];
		custom->AddMatch(m2);
		custom->AddMatch(m3);

		//tourney.AddMatch(m2);
		//tourney.AddMatch(m3);
		tourney.GenerateSchedule();

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



TEST(Match, BreakTime)
{
	initialize();

	Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
	Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
	Judoka j3(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
	Judoka j4(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));

	Match m1(&j1, &j2, nullptr, 1);
	Match m2(&j2, &j1, nullptr, 1);
	Match m3(&j3, &j4, nullptr, 1);
	Match m4(&j1, nullptr, nullptr, 1);

	m1.StartMatch();
	m1.EndMatch();

	EXPECT_EQ(m2.GetCurrentBreaktime(), 0);
	ZED::Core::Pause(10 * 1000);
	EXPECT_EQ(m2.GetCurrentBreaktime(), 10);
	ZED::Core::Pause(10 * 1000);
	EXPECT_EQ(m2.GetCurrentBreaktime(), 20);
	EXPECT_EQ(m4.GetCurrentBreaktime(), 20);

	EXPECT_TRUE(m3.GetCurrentBreaktime() > 20);
}



TEST(Match, SkipBreakTime)
{
	initialize();

	Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
	Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
	Judoka j3(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
	Judoka j4(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));

	RuleSet* rules = new RuleSet("test", 30, 30, 20, 10, false, false, false, 60);
	Match m1(&j1, &j2, nullptr, 1);
	Match m2(&j2, &j1, nullptr, 1);

	m1.SetRuleSet(rules);
	m2.SetRuleSet(rules);

	m1.StartMatch();
	m1.EndMatch();

	EXPECT_TRUE(j1.NeedsBreak());
	EXPECT_TRUE(j2.NeedsBreak());

	j1.SkipBreak();
	j2.SkipBreak();

	EXPECT_FALSE(j1.NeedsBreak());
	EXPECT_FALSE(j2.NeedsBreak());
}



TEST(Match, MatchTimeOnlyForWinner)
{
	initialize();

	Mat mat(1);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Judoka* j1 = new Judoka(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		Judoka* j2 = new Judoka(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));

		Tournament tourney;

		RoundRobin* r = new RoundRobin(Weight(0), Weight(0));
		r->AddParticipant(j1);
		r->AddParticipant(j2);
		r->SetMatID(1);

		tourney.AddMatchTable(r);

		ASSERT_EQ(r->GetSchedule().size(), 1);

		EXPECT_TRUE(mat.StartMatch(r->GetSchedule()[0]));
		EXPECT_EQ(r->GetSchedule()[0]->GetLog().GetNumEvent(), 1);

		EXPECT_TRUE(mat.StopMatch());
		EXPECT_EQ(r->GetSchedule()[0]->GetLog().GetNumEvent(), 2);

		EXPECT_TRUE(mat.StartMatch(r->GetSchedule()[0]));
		EXPECT_EQ(r->GetSchedule()[0]->GetLog().GetNumEvent(), 3);

		mat.Hajime();

		EXPECT_FALSE(mat.StopMatch());

		ZED::Core::Pause(3005);

		mat.AddIppon(f);
		mat.EndMatch();

		
		EXPECT_EQ(r->GetStatus(), Status::Concluded);
		auto results = r->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);

		EXPECT_EQ(results[0].Wins, 1);
		EXPECT_EQ(results[0].Score, 10);
		EXPECT_EQ(results[0].Time / 1000, 3);

		EXPECT_EQ(results[1].Wins,  0);
		EXPECT_EQ(results[1].Score, 0);
		EXPECT_EQ(results[1].Time,  0);//Not counted for loser
	}
}