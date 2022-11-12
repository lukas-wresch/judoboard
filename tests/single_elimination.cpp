#include "tests.h"



TEST(SingleElimination, ExportImport)
{
	initialize();

	SingleElimination group(0, 200);

	group.IsThirdPlaceMatch(true);
	group.IsFifthPlaceMatch(true);

	YAML::Emitter yaml;
	yaml << YAML::BeginMap;
	group >> yaml;
	yaml << YAML::EndMap;

	SingleElimination group2(YAML::Load(yaml.c_str()));

	EXPECT_EQ(group2.IsThirdPlaceMatch(), group.IsThirdPlaceMatch());
	EXPECT_EQ(group2.IsFifthPlaceMatch(), group.IsFifthPlaceMatch());


	{
		group.IsThirdPlaceMatch(false);
		group.IsFifthPlaceMatch(false);

		YAML::Emitter yaml;
		yaml << YAML::BeginMap;
		group >> yaml;
		yaml << YAML::EndMap;

		SingleElimination group2(YAML::Load(yaml.c_str()));

		EXPECT_EQ(group2.IsThirdPlaceMatch(), group.IsThirdPlaceMatch());
		EXPECT_EQ(group2.IsFifthPlaceMatch(), group.IsFifthPlaceMatch());
	}
}



TEST(SingleElimination, ExportImport_StartingPositions)
{
	initialize();

	for (int i = 0; i < 100; i++)
	{
		Tournament* t = new Tournament("Tournament Name");
		t->EnableAutoSave(false);

		SingleElimination group(0, 200);
		t->AddMatchTable(&group);

		auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
		t->AddParticipant(j1);

		auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
		t->AddParticipant(j2);

		auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
		t->AddParticipant(j3);

		auto j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 80);
		t->AddParticipant(j4);

		auto j5 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 90);
		t->AddParticipant(j5);


		group.SetStartingPosition(j1, rand() % 8);
		group.SetStartingPosition(j2, rand() % 8);
		group.SetStartingPosition(j3, rand() % 8);
		group.SetStartingPosition(j4, rand() % 8);
		group.SetStartingPosition(j5, rand() % 8);

		ASSERT_EQ(group.GetParticipants().size(), 5);


		{
			YAML::Emitter yaml;
			yaml << YAML::BeginMap;
			group >> yaml;
			yaml << YAML::EndMap;

			SingleElimination group2(YAML::Load(yaml.c_str()), t);

			EXPECT_EQ(group2.GetStartingPosition(j1), group.GetStartingPosition(j1));
			EXPECT_EQ(group2.GetStartingPosition(j2), group.GetStartingPosition(j2));
			EXPECT_EQ(group2.GetStartingPosition(j3), group.GetStartingPosition(j3));
			EXPECT_EQ(group2.GetStartingPosition(j4), group.GetStartingPosition(j4));
			EXPECT_EQ(group2.GetStartingPosition(j5), group.GetStartingPosition(j5));
		}
	}
}



TEST(SingleElimination, Count1)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);
	
	SingleElimination group(0, 200);

	t->AddMatchTable(&group);
	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
	t->AddParticipant(j1);

	ASSERT_EQ(group.GetParticipants().size(), 1);
	ASSERT_EQ(group.GetSchedule().size(), 0);

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count2)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	ASSERT_EQ(group.GetParticipants().size(), 2);
	ASSERT_EQ(group.GetSchedule().size(), 1);

	Mat m(1);

	EXPECT_TRUE(m.StartMatch(group.GetSchedule()[0]));
	if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
		m.AddIppon(Fighter::White);
	else
		m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j2->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count3)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j3);

	ASSERT_EQ(group.GetParticipants().size(), 3);
	ASSERT_EQ(group.GetSchedule().size(), 3);

	EXPECT_EQ(group.GetSchedule()[0]->GetFighter(Fighter::White)->GetUUID(), j1->GetUUID());
	EXPECT_EQ(group.GetSchedule()[0]->GetFighter(Fighter::Blue )->GetUUID(), j2->GetUUID());
	EXPECT_EQ(group.GetSchedule()[1]->GetFighter(Fighter::White)->GetUUID(), j3->GetUUID());
	EXPECT_FALSE(group.GetSchedule()[1]->GetFighter(Fighter::Blue));
	EXPECT_FALSE(group.GetSchedule()[2]->GetFighter(Fighter::White));
	EXPECT_TRUE( group.GetSchedule()[2]->GetFighter(Fighter::Blue));

	//Check dependencies
	ASSERT_TRUE(group.GetSchedule()[2]->GetDependentMatchOf(Fighter::White));
	ASSERT_TRUE(group.GetSchedule()[2]->GetDependentMatchOf(Fighter::Blue));

	EXPECT_EQ(*group.GetSchedule()[2]->GetDependentMatchOf(Fighter::White), *group.GetSchedule()[0]);
	EXPECT_EQ(*group.GetSchedule()[2]->GetDependentMatchOf(Fighter::Blue),  *group.GetSchedule()[1]);

	Mat m(1);

	for (auto match : group.GetSchedule())
	{
		if (match->IsEmptyMatch())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j3->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count3_BO3)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	group.IsBestOfThree(true);
	t->AddMatchTable(&group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j3);

	ASSERT_EQ(group.GetParticipants().size(), 3);
	ASSERT_EQ(group.GetSchedule().size(), 3*3);

	Mat m(1);

	//Check dependency tree
	EXPECT_EQ(*group.GetSchedule()[2]->GetDependentMatchOf(Fighter::White), *group.GetSchedule()[0]);
	EXPECT_EQ(*group.GetSchedule()[2]->GetDependentMatchOf(Fighter::Blue),  *group.GetSchedule()[1]);

	ASSERT_TRUE(group.GetSchedule()[6]->GetDependentMatchOf(Fighter::White));
	ASSERT_TRUE(group.GetSchedule()[6]->GetDependentMatchOf(Fighter::Blue));
	ASSERT_TRUE(group.GetSchedule()[7]->GetDependentMatchOf(Fighter::White));
	ASSERT_TRUE(group.GetSchedule()[7]->GetDependentMatchOf(Fighter::Blue));
	ASSERT_TRUE(group.GetSchedule()[8]->GetDependentMatchOf(Fighter::White));
	ASSERT_TRUE(group.GetSchedule()[8]->GetDependentMatchOf(Fighter::Blue));

	EXPECT_EQ(*group.GetSchedule()[6]->GetDependentMatchOf(Fighter::White), *group.GetSchedule()[2]);
	EXPECT_EQ(*group.GetSchedule()[6]->GetDependentMatchOf(Fighter::Blue),  *group.GetSchedule()[5]);
	EXPECT_EQ(*group.GetSchedule()[7]->GetDependentMatchOf(Fighter::White), *group.GetSchedule()[5]);
	EXPECT_EQ(*group.GetSchedule()[7]->GetDependentMatchOf(Fighter::Blue),  *group.GetSchedule()[2]);
	EXPECT_EQ(*group.GetSchedule()[8]->GetDependentMatchOf(Fighter::White), *group.GetSchedule()[6]);
	EXPECT_EQ(*group.GetSchedule()[8]->GetDependentMatchOf(Fighter::Blue),  *group.GetSchedule()[7]);

	EXPECT_TRUE(m.StartMatch(group.GetSchedule()[0]));
	m.AddIppon(Fighter::White);//j1
	EXPECT_TRUE(m.EndMatch());

	EXPECT_TRUE(m.StartMatch(group.GetSchedule()[1]));
	m.AddIppon(Fighter::White);//j2
	EXPECT_TRUE(m.EndMatch());

	EXPECT_TRUE(m.StartMatch(group.GetSchedule()[2]));
	m.AddIppon(Fighter::Blue);//j2
	EXPECT_TRUE(m.EndMatch());

	EXPECT_FALSE(m.StartMatch(group.GetSchedule()[3]));//Half empty
	EXPECT_FALSE(m.StartMatch(group.GetSchedule()[4]));//Half empty
	EXPECT_FALSE(m.StartMatch(group.GetSchedule()[5]));//Half empty

	EXPECT_TRUE(m.StartMatch(group.GetSchedule()[6]));
	m.AddIppon(Fighter::White);//j2
	EXPECT_TRUE(m.EndMatch());

	EXPECT_TRUE(m.StartMatch(group.GetSchedule()[7]));
	m.AddIppon(Fighter::Blue);//j2
	EXPECT_TRUE(m.EndMatch());

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 2);
	ASSERT_TRUE(results[0].Judoka);
	ASSERT_TRUE(results[1].Judoka);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j2->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j3->GetUUID());
}



TEST(SingleElimination, Count4)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j3);

	auto j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 80);
	t->AddParticipant(j4);

	ASSERT_EQ(group.GetParticipants().size(), 4);
	ASSERT_EQ(group.GetSchedule().size(),     3);

	Mat m(1);

	for (auto match : group.GetSchedule())
	{
		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j4->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count5)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j3);

	auto j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 80);
	t->AddParticipant(j4);

	auto j5 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 90);
	t->AddParticipant(j5);

	ASSERT_EQ(group.GetParticipants().size(), 5);
	ASSERT_EQ(group.GetSchedule().size(),     7);

	Mat m(1);

	for (auto match : group.GetSchedule())
	{
		if (match->IsEmptyMatch())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j5->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count8)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	Judoka* j[8];
	bool has_match[8];

	for (int i = 0; i < 8; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group.GetParticipants().size(), 8);

	for (auto match : group.GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		for (int i = 0; i < 8; ++i)
		{
			if (match->GetFighter(Fighter::White)->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
			if (match->GetFighter(Fighter::Blue )->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
		}
	}

	for (int i = 0; i < 8; ++i)
		EXPECT_TRUE(has_match[i]);

	Mat m(1);

	for (auto match : group.GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[7]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count10)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	Judoka* j[10];
	bool has_match[10];

	for (int i = 0; i < 10; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group.GetParticipants().size(), 10);

	for (auto match : group.GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		for (int i = 0; i < 10; ++i)
		{
			if (match->GetFighter(Fighter::White)->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
			if (match->GetFighter(Fighter::Blue )->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
		}
	}

	for (int i = 0; i < 10; ++i)
		EXPECT_TRUE(has_match[i]);

	Mat m(1);

	for (auto match : group.GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[9]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count14)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	Judoka* j[14];
	bool has_match[14];

	for (int i = 0; i < 14; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group.GetParticipants().size(), 14);

	for (auto match : group.GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		for (int i = 0; i < 14; ++i)
		{
			if (match->GetFighter(Fighter::White)->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
			if (match->GetFighter(Fighter::Blue )->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
		}
	}

	for (int i = 0; i < 14; ++i)
		EXPECT_TRUE(has_match[i]);

	Mat m(1);

	for (auto match : group.GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[13]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count16)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[16];
	bool has_match[16];

	for (int i = 0; i < 16; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	ASSERT_EQ(group.GetParticipants().size(), 16);

	for (auto match : group.GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		for (int i = 0; i < 16; ++i)
		{
			if (match->GetFighter(Fighter::White)->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
			if (match->GetFighter(Fighter::Blue )->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
		}
	}

	for (int i = 0; i < 16; ++i)
		EXPECT_TRUE(has_match[i]);

	Mat m(1);

	for (auto match : group.GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group.CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[15]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count4_ExportImport)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j3);

	auto j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 80);
	t->AddParticipant(j4);

	YAML::Emitter yaml;
	yaml << YAML::BeginMap;
	group >> yaml;
	yaml << YAML::EndMap;

	SingleElimination group2(YAML::Load(yaml.c_str()), t);

	EXPECT_EQ(group.GetMatID(), group2.GetMatID());
	EXPECT_EQ(group.GetParticipants().size(), group2.GetParticipants().size());
	ASSERT_EQ(group.GetSchedule().size(),     group2.GetSchedule().size());

	ASSERT_EQ(group2.GetSchedule()[2]->GetDependentMatches().size(), 2);
	ASSERT_TRUE(group2.GetSchedule()[2]->GetDependentMatches()[0]);
	ASSERT_TRUE(group2.GetSchedule()[2]->GetDependentMatches()[1]);

	EXPECT_EQ(group2.GetSchedule()[2]->GetDependentMatches()[0]->GetUUID(), group2.GetSchedule()[0]->GetUUID());
	EXPECT_EQ(group2.GetSchedule()[2]->GetDependentMatches()[1]->GetUUID(), group2.GetSchedule()[1]->GetUUID());
}



TEST(SingleElimination, Count5_ExportImport)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination group(0, 200);
	group.SetMatID(1);
	t->AddMatchTable(&group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j3);

	auto j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 80);
	t->AddParticipant(j4);

	auto j5 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 90);
	t->AddParticipant(j5);

	YAML::Emitter yaml;
	yaml << YAML::BeginMap;
	group >> yaml;
	yaml << YAML::EndMap;

	SingleElimination group2(YAML::Load(yaml.c_str()), t);

	EXPECT_EQ(group.GetMatID(), group2.GetMatID());
	EXPECT_EQ(group.GetParticipants().size(), group2.GetParticipants().size());
	ASSERT_EQ(group.GetSchedule().size(),     group2.GetSchedule().size());

	for (int i = 0; i < group.GetSchedule().size(); ++i)
	{
		EXPECT_EQ(group.GetSchedule()[i]->GetUUID(), group2.GetSchedule()[i]->GetUUID());

		if (group.GetSchedule()[i]->GetFighter(Fighter::White))
		{
			ASSERT_TRUE(group2.GetSchedule()[i]->GetFighter(Fighter::White));
			EXPECT_EQ(group.GetSchedule()[i]->GetFighter(Fighter::White)->GetName(NameStyle::GivenName), group2.GetSchedule()[i]->GetFighter(Fighter::White)->GetName(NameStyle::GivenName));
		}

		if (group.GetSchedule()[i]->GetFighter(Fighter::Blue))
		{
			ASSERT_TRUE(group2.GetSchedule()[i]->GetFighter(Fighter::Blue));
			EXPECT_EQ(group.GetSchedule()[i]->GetFighter(Fighter::Blue)->GetName(NameStyle::GivenName), group2.GetSchedule()[i]->GetFighter(Fighter::Blue)->GetName(NameStyle::GivenName));
		}
	}

	EXPECT_EQ(group.ToHTML().length(), group2.ToHTML().length());
	EXPECT_EQ(group.ToHTML(), group2.ToHTML());
}