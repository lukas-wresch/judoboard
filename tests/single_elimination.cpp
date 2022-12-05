#include "tests.h"



TEST(SingleElimination, ExportImport)
{
	initialize();

	SingleElimination* group = new SingleElimination(0, 200);

	group->SetScheduleIndex(rand());

	group->IsThirdPlaceMatch(true);
	group->IsFifthPlaceMatch(true);

	YAML::Emitter yaml;
	yaml << YAML::BeginMap;
	*group >> yaml;
	yaml << YAML::EndMap;

	SingleElimination group2(YAML::Load(yaml.c_str()));

	EXPECT_EQ(group2.IsThirdPlaceMatch(), group->IsThirdPlaceMatch());
	EXPECT_EQ(group2.IsFifthPlaceMatch(), group->IsFifthPlaceMatch());


	{
		group->IsThirdPlaceMatch(false);
		group->IsFifthPlaceMatch(false);

		YAML::Emitter yaml;
		yaml << YAML::BeginMap;
		*group >> yaml;
		yaml << YAML::EndMap;

		SingleElimination group2(YAML::Load(yaml.c_str()));

		EXPECT_EQ(group2.IsThirdPlaceMatch(), group->IsThirdPlaceMatch());
		EXPECT_EQ(group2.IsFifthPlaceMatch(), group->IsFifthPlaceMatch());

		YAML::Emitter yaml2, yaml3;
		group->ToString(yaml2);
		group2.ToString(yaml3);
		EXPECT_EQ((std::string)yaml2.c_str(), (std::string)yaml3.c_str());
	}
}



TEST(SingleElimination, ExportImport_StartPositions)
{
	initialize();

	for (int i = 0; i < 100; i++)
	{
		Tournament* t = new Tournament("Tournament Name");
		t->EnableAutoSave(false);

		SingleElimination* group = new SingleElimination(0, 200);
		t->AddMatchTable(group);

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


		group->SetStartPosition(j1, rand() % 8);
		group->SetStartPosition(j2, rand() % 8);
		group->SetStartPosition(j3, rand() % 8);
		group->SetStartPosition(j4, rand() % 8);
		group->SetStartPosition(j5, rand() % 8);

		ASSERT_EQ(group->GetParticipants().size(), 5);


		{
			YAML::Emitter yaml;
			yaml << YAML::BeginMap;
			*group >> yaml;
			yaml << YAML::EndMap;

			SingleElimination group2(YAML::Load(yaml.c_str()), t);

			EXPECT_EQ(group2.GetStartPosition(j1), group->GetStartPosition(j1));
			EXPECT_EQ(group2.GetStartPosition(j2), group->GetStartPosition(j2));
			EXPECT_EQ(group2.GetStartPosition(j3), group->GetStartPosition(j3));
			EXPECT_EQ(group2.GetStartPosition(j4), group->GetStartPosition(j4));
			EXPECT_EQ(group2.GetStartPosition(j5), group->GetStartPosition(j5));
		}

		delete t;
	}
}



TEST(SingleElimination, Count1)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);
	
	SingleElimination* group = new SingleElimination(0, 200);

	t->AddMatchTable(group);
	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + rand() % 50);
	t->AddParticipant(j1);

	ASSERT_EQ(group->GetParticipants().size(), 1);
	ASSERT_EQ(group->GetSchedule().size(), 0);

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 1);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j1->GetUUID());

	delete t;
}



TEST(SingleElimination, Count2)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	ASSERT_EQ(group->GetParticipants().size(), 2);
	ASSERT_EQ(group->GetSchedule().size(), 1);

	Mat m(1);

	EXPECT_TRUE(m.StartMatch(group->GetSchedule()[0]));
	if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
		m.AddIppon(Fighter::White);
	else
		m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j2->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j1->GetUUID());

	delete t;
}



TEST(SingleElimination, Count3)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j3);

	ASSERT_EQ(group->GetParticipants().size(), 3);
	ASSERT_EQ(group->GetSchedule().size(), 3);

	EXPECT_EQ(group->GetSchedule()[0]->GetFighter(Fighter::White)->GetUUID(), j1->GetUUID());
	EXPECT_EQ(group->GetSchedule()[0]->GetFighter(Fighter::Blue )->GetUUID(), j3->GetUUID());
	EXPECT_EQ(group->GetSchedule()[1]->GetFighter(Fighter::White)->GetUUID(), j2->GetUUID());
	EXPECT_FALSE(group->GetSchedule()[1]->GetFighter(Fighter::Blue));
	EXPECT_FALSE(group->GetSchedule()[2]->GetFighter(Fighter::White));
	EXPECT_TRUE( group->GetSchedule()[2]->GetFighter(Fighter::Blue));

	//Check dependencies
	ASSERT_TRUE(group->GetSchedule()[2]->GetDependentMatchOf(Fighter::White));
	ASSERT_TRUE(group->GetSchedule()[2]->GetDependentMatchOf(Fighter::Blue));

	EXPECT_EQ(*group->GetSchedule()[2]->GetDependentMatchOf(Fighter::White), *group->GetSchedule()[0]);
	EXPECT_EQ(*group->GetSchedule()[2]->GetDependentMatchOf(Fighter::Blue),  *group->GetSchedule()[1]);

	Mat m(1);

	for (auto match : group->GetSchedule())
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

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j3->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());

	delete t;
}



TEST(SingleElimination, Count3_BO3)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	group->IsBestOfThree(true);
	t->AddMatchTable(group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j3);

	ASSERT_EQ(group->GetParticipants().size(), 3);
	ASSERT_EQ(group->GetSchedule().size(), 3*3);

	Mat m(1);

	//Check dependency tree
	EXPECT_EQ(*group->GetSchedule()[2]->GetDependentMatchOf(Fighter::White), *group->GetSchedule()[0]);
	EXPECT_EQ(*group->GetSchedule()[2]->GetDependentMatchOf(Fighter::Blue),  *group->GetSchedule()[1]);

	ASSERT_TRUE(group->GetSchedule()[6]->GetDependentMatchOf(Fighter::White));
	ASSERT_TRUE(group->GetSchedule()[6]->GetDependentMatchOf(Fighter::Blue));
	ASSERT_TRUE(group->GetSchedule()[7]->GetDependentMatchOf(Fighter::White));
	ASSERT_TRUE(group->GetSchedule()[7]->GetDependentMatchOf(Fighter::Blue));
	ASSERT_TRUE(group->GetSchedule()[8]->GetDependentMatchOf(Fighter::White));
	ASSERT_TRUE(group->GetSchedule()[8]->GetDependentMatchOf(Fighter::Blue));

	EXPECT_EQ(*group->GetSchedule()[6]->GetDependentMatchOf(Fighter::White), *group->GetSchedule()[2]);
	EXPECT_EQ(*group->GetSchedule()[6]->GetDependentMatchOf(Fighter::Blue),  *group->GetSchedule()[5]);
	EXPECT_EQ(*group->GetSchedule()[7]->GetDependentMatchOf(Fighter::White), *group->GetSchedule()[5]);
	EXPECT_EQ(*group->GetSchedule()[7]->GetDependentMatchOf(Fighter::Blue),  *group->GetSchedule()[2]);
	EXPECT_EQ(*group->GetSchedule()[8]->GetDependentMatchOf(Fighter::White), *group->GetSchedule()[6]);
	EXPECT_EQ(*group->GetSchedule()[8]->GetDependentMatchOf(Fighter::Blue),  *group->GetSchedule()[7]);

	EXPECT_TRUE(m.StartMatch(group->GetSchedule()[0]));
	m.AddIppon(Fighter::White);//j1
	EXPECT_TRUE(m.EndMatch());

	EXPECT_TRUE(m.StartMatch(group->GetSchedule()[1]));
	m.AddIppon(Fighter::White);//j2
	EXPECT_TRUE(m.EndMatch());

	EXPECT_TRUE(m.StartMatch(group->GetSchedule()[2]));
	m.AddIppon(Fighter::Blue);//j2
	EXPECT_TRUE(m.EndMatch());

	EXPECT_FALSE(m.StartMatch(group->GetSchedule()[3]));//Half empty
	EXPECT_FALSE(m.StartMatch(group->GetSchedule()[4]));//Half empty
	EXPECT_FALSE(m.StartMatch(group->GetSchedule()[5]));//Half empty

	EXPECT_TRUE(m.StartMatch(group->GetSchedule()[6]));
	m.AddIppon(Fighter::White);//j2
	EXPECT_TRUE(m.EndMatch());

	EXPECT_TRUE(m.StartMatch(group->GetSchedule()[7]));
	m.AddIppon(Fighter::Blue);//j2
	EXPECT_TRUE(m.EndMatch());

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	ASSERT_TRUE(results[0].Judoka);
	ASSERT_TRUE(results[1].Judoka);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j3->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());

	delete t;
}



TEST(SingleElimination, Count4)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j3);

	auto j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 80);
	t->AddParticipant(j4);

	ASSERT_EQ(group->GetParticipants().size(), 4);
	ASSERT_EQ(group->GetSchedule().size(),     3);

	Mat m(1);

	for (auto match : group->GetSchedule())
	{
		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j4->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());

	delete t;
}



TEST(SingleElimination, Count5)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

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

	ASSERT_EQ(group->GetParticipants().size(), 5);
	ASSERT_EQ(group->GetSchedule().size(),     7);

	Mat m(1);

	for (auto match : group->GetSchedule())
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

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j5->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count8)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	Judoka* j[8];
	bool has_match[8];

	for (int i = 0; i < 8; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group->GetParticipants().size(), 8);

	for (auto match : group->GetSchedule())
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

	for (auto match : group->GetSchedule())
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

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[7]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count8_Dont_allow_illegal_start_pos)
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

	group.SetStartPosition(j[7], 8);
	for (int i = 0; i < 7; ++i)
		group.RemoveParticipant(j[i]);

	EXPECT_EQ(group.GetStartPosition(j[7]), 0);

	for (int i = 0; i < 8; ++i)
		group.AddParticipant(j[i]);

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

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[7]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count8_BO3)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	Judoka* j[8];
	bool has_match[8];

	for (int i = 0; i < 8; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group->GetParticipants().size(), 8);
	ASSERT_EQ(group->GetSchedule().size(), 7);

	group->IsBestOfThree(true);

	ASSERT_EQ(group->GetParticipants().size(), 8);
	ASSERT_EQ(group->GetSchedule().size(), 7*3);

	for (auto match : group->GetSchedule())
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

	for (auto match : group->GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;
		if (match->HasConcluded())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[7]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count10)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	Judoka* j[10];
	bool has_match[10];

	for (int i = 0; i < 10; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group->GetParticipants().size(), 10);

	for (auto match : group->GetSchedule())
	{
		for (int i = 0; i < 10; ++i)
		{
			if (match->GetFighter(Fighter::White) && match->GetFighter(Fighter::White)->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
			if (match->GetFighter(Fighter::Blue ) && match->GetFighter(Fighter::Blue )->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
		}
	}

	for (int i = 0; i < 10; ++i)
		EXPECT_TRUE(has_match[i]);

	Mat m(1);

	for (auto match : group->GetSchedule())
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

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[9]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count14)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	Judoka* j[14];
	bool has_match[14];

	for (int i = 0; i < 14; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group->GetParticipants().size(), 14);

	for (auto match : group->GetSchedule())
	{
		for (int i = 0; i < 14; ++i)
		{
			if (match->GetFighter(Fighter::White) && match->GetFighter(Fighter::White)->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
			if (match->GetFighter(Fighter::Blue ) && match->GetFighter(Fighter::Blue )->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
		}
	}

	for (int i = 0; i < 14; ++i)
		EXPECT_TRUE(has_match[i]);

	Mat m(1);

	for (auto match : group->GetSchedule())
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

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[13]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());

	delete t;
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

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < 16; ++i)
		group->SetStartPosition(j[i], i);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[0]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[8]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[4]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[10]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[14]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[15]));

	ASSERT_EQ(group->GetParticipants().size(), 16);

	for (auto match : group->GetSchedule())
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

	for (auto match : group->GetSchedule())
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

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[15]->GetUUID());
	//EXPECT_EQ(results[1].Judoka->GetUUID(), j2->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j1->GetUUID());

	delete t;
}



TEST(SingleElimination, Count32)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];

	for (int i = 1; i <= 32; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < 32; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[17]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[25]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[21]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[29]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[19]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[27]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[23]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[15]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[31]));
	EXPECT_TRUE(group->GetMatch(8)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(8)->Contains(*j[18]));
	EXPECT_TRUE(group->GetMatch(9)->Contains(*j[10]));
	EXPECT_TRUE(group->GetMatch(9)->Contains(*j[26]));
	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[22]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[14]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[30]));
	EXPECT_TRUE(group->GetMatch(12)->Contains(*j[4]));
	EXPECT_TRUE(group->GetMatch(12)->Contains(*j[20]));
	EXPECT_TRUE(group->GetMatch(13)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(13)->Contains(*j[28]));
	EXPECT_TRUE(group->GetMatch(14)->Contains(*j[8]));
	EXPECT_TRUE(group->GetMatch(14)->Contains(*j[24]));
	EXPECT_TRUE(group->GetMatch(15)->Contains(*j[16]));
	EXPECT_TRUE(group->GetMatch(15)->Contains(*j[32]));

	ASSERT_EQ(group->GetParticipants().size(), 32);
	ASSERT_EQ(group->GetSchedule().size(),     31);

	delete t;
}



TEST(SingleElimination, Count64)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[65];
	bool has_match[65];

	for (int i = 1; i <= 64; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < 64; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[33]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[17]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[49]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[41]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[25]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[57]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[37]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[21]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[53]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[45]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[29]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[61]));
	EXPECT_TRUE(group->GetMatch(8)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(8)->Contains(*j[35]));
	EXPECT_TRUE(group->GetMatch(9)->Contains(*j[19]));
	EXPECT_TRUE(group->GetMatch(9)->Contains(*j[51]));
	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[43]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[27]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[59]));
	EXPECT_TRUE(group->GetMatch(12)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(12)->Contains(*j[39]));
	EXPECT_TRUE(group->GetMatch(13)->Contains(*j[23]));
	EXPECT_TRUE(group->GetMatch(13)->Contains(*j[55]));
	EXPECT_TRUE(group->GetMatch(14)->Contains(*j[15]));
	EXPECT_TRUE(group->GetMatch(14)->Contains(*j[47]));
	EXPECT_TRUE(group->GetMatch(15)->Contains(*j[31]));
	EXPECT_TRUE(group->GetMatch(15)->Contains(*j[63]));
	EXPECT_TRUE(group->GetMatch(16)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(16)->Contains(*j[34]));
	EXPECT_TRUE(group->GetMatch(17)->Contains(*j[18]));
	EXPECT_TRUE(group->GetMatch(17)->Contains(*j[50]));
	EXPECT_TRUE(group->GetMatch(18)->Contains(*j[10]));
	EXPECT_TRUE(group->GetMatch(18)->Contains(*j[42]));
	EXPECT_TRUE(group->GetMatch(19)->Contains(*j[26]));
	EXPECT_TRUE(group->GetMatch(19)->Contains(*j[58]));
	EXPECT_TRUE(group->GetMatch(20)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(20)->Contains(*j[38]));
	EXPECT_TRUE(group->GetMatch(21)->Contains(*j[22]));
	EXPECT_TRUE(group->GetMatch(21)->Contains(*j[54]));
	EXPECT_TRUE(group->GetMatch(22)->Contains(*j[14]));
	EXPECT_TRUE(group->GetMatch(22)->Contains(*j[46]));
	EXPECT_TRUE(group->GetMatch(23)->Contains(*j[30]));
	EXPECT_TRUE(group->GetMatch(23)->Contains(*j[62]));
	EXPECT_TRUE(group->GetMatch(24)->Contains(*j[4]));
	EXPECT_TRUE(group->GetMatch(24)->Contains(*j[36]));
	EXPECT_TRUE(group->GetMatch(25)->Contains(*j[20]));
	EXPECT_TRUE(group->GetMatch(25)->Contains(*j[52]));
	EXPECT_TRUE(group->GetMatch(26)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(26)->Contains(*j[44]));
	EXPECT_TRUE(group->GetMatch(27)->Contains(*j[28]));
	EXPECT_TRUE(group->GetMatch(27)->Contains(*j[60]));
	EXPECT_TRUE(group->GetMatch(28)->Contains(*j[8]));
	EXPECT_TRUE(group->GetMatch(28)->Contains(*j[40]));
	EXPECT_TRUE(group->GetMatch(29)->Contains(*j[24]));
	EXPECT_TRUE(group->GetMatch(29)->Contains(*j[56]));
	EXPECT_TRUE(group->GetMatch(30)->Contains(*j[16]));
	EXPECT_TRUE(group->GetMatch(30)->Contains(*j[48]));
	EXPECT_TRUE(group->GetMatch(31)->Contains(*j[32]));
	EXPECT_TRUE(group->GetMatch(31)->Contains(*j[64]));
		
	ASSERT_EQ(group->GetParticipants().size(), 64);
	ASSERT_EQ(group->GetSchedule().size(),     63);
}



TEST(SingleElimination, Count4_ExportImport)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

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
	*group >> yaml;
	yaml << YAML::EndMap;

	SingleElimination group2(YAML::Load(yaml.c_str()), t);

	EXPECT_EQ(group->GetMatID(), group2.GetMatID());
	EXPECT_EQ(group->GetParticipants().size(), group2.GetParticipants().size());
	ASSERT_EQ(group->GetSchedule().size(),     group2.GetSchedule().size());

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

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

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
	*group >> yaml;
	yaml << YAML::EndMap;

	SingleElimination group2(YAML::Load(yaml.c_str()), t);

	EXPECT_EQ(group->GetMatID(), group2.GetMatID());
	EXPECT_EQ(group->GetParticipants().size(), group2.GetParticipants().size());
	ASSERT_EQ(group->GetSchedule().size(),     group2.GetSchedule().size());

	for (int i = 0; i < group->GetSchedule().size(); ++i)
	{
		EXPECT_EQ(group->GetSchedule()[i]->GetUUID(), group2.GetSchedule()[i]->GetUUID());

		if (group->GetSchedule()[i]->GetFighter(Fighter::White))
		{
			ASSERT_TRUE(group2.GetSchedule()[i]->GetFighter(Fighter::White));
			EXPECT_EQ(group->GetSchedule()[i]->GetFighter(Fighter::White)->GetName(NameStyle::GivenName), group2.GetSchedule()[i]->GetFighter(Fighter::White)->GetName(NameStyle::GivenName));
		}

		if (group->GetSchedule()[i]->GetFighter(Fighter::Blue))
		{
			ASSERT_TRUE(group2.GetSchedule()[i]->GetFighter(Fighter::Blue));
			EXPECT_EQ(group->GetSchedule()[i]->GetFighter(Fighter::Blue)->GetName(NameStyle::GivenName), group2.GetSchedule()[i]->GetFighter(Fighter::Blue)->GetName(NameStyle::GivenName));
		}
	}

	EXPECT_EQ(group->ToHTML().length(), group2.ToHTML().length());
	EXPECT_EQ(group->ToHTML(), group2.ToHTML());
}



TEST(SingleElimination, Count4_ThirdPlace)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	group->IsThirdPlaceMatch(true);
	t->AddMatchTable(group);

	auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
	t->AddParticipant(j1);

	auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
	t->AddParticipant(j2);

	auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
	t->AddParticipant(j3);

	auto j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 80);
	t->AddParticipant(j4);

	EXPECT_EQ(group->GetParticipants().size(), 4);
	EXPECT_EQ(group->GetSchedule().size(),     4);

	Mat m(1);

	for (auto match : group->GetSchedule())
	{
		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	//j1 vs j3, j2 vs j4

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 4);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j4->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j3->GetUUID());
	EXPECT_EQ(results[2].Judoka->GetUUID(), j2->GetUUID());
	EXPECT_EQ(results[3].Judoka->GetUUID(), j1->GetUUID());
}



TEST(SingleElimination, Count8_3rd_5th)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	group->IsThirdPlaceMatch(true);
	group->IsFifthPlaceMatch(true);
	t->AddMatchTable(group);

	Judoka* j[9];
	bool has_match[9];

	for (int i = 1; i <= 8; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group->GetParticipants().size(), 8);

	for (auto match : group->GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		for (int i = 1; i <= 8; ++i)
		{
			if (match->GetFighter(Fighter::White)->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
			if (match->GetFighter(Fighter::Blue )->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
		}
	}

	for (int i = 1; i <= 8; ++i)
		EXPECT_TRUE(has_match[i]);

	Mat m(1);

	for (auto match : group->GetSchedule())
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

	auto results = group->CalculateResults();

	//j1 vs j5, j3 vs j7, j2 vs j6, j4 vs j8
	//j5 vs j7, j6 vs j8
	//j7 vs j8

	ASSERT_EQ(results.GetSize(), 6);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[8]->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j[7]->GetUUID());
	EXPECT_EQ(results[2].Judoka->GetUUID(), j[6]->GetUUID());
	EXPECT_EQ(results[3].Judoka->GetUUID(), j[5]->GetUUID());
	EXPECT_EQ(results[4].Judoka->GetUUID(), j[4]->GetUUID());
	EXPECT_EQ(results[5].Judoka->GetUUID(), j[3]->GetUUID());
}



TEST(SingleElimination, Count8_5th)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	group->IsThirdPlaceMatch(false);
	group->IsFifthPlaceMatch(true);
	t->AddMatchTable(group);

	Judoka* j[9];
	bool has_match[9];

	for (int i = 1; i <= 8; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group->GetParticipants().size(), 8);

	for (auto match : group->GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		for (int i = 1; i <= 8; ++i)
		{
			if (match->GetFighter(Fighter::White)->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
			if (match->GetFighter(Fighter::Blue )->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
		}
	}

	for (int i = 1; i <= 8; ++i)
		EXPECT_TRUE(has_match[i]);

	Mat m(1);

	for (auto match : group->GetSchedule())
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

	auto results = group->CalculateResults();

	//j1 vs j5, j3 vs j7, j2 vs j6, j4 vs j8
	//j5 vs j7, j6 vs j8
	//j7 vs j8

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[8]->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j[7]->GetUUID());
}



TEST(SingleElimination, Render_3rd_5th)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	Judoka* j[8];
	bool has_match[8];

	for (int i = 0; i < 8; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group->GetParticipants().size(), 8);

	auto none = group->ToHTML();

	group->IsFifthPlaceMatch(true);

	auto with_5th = group->ToHTML();

	group->IsThirdPlaceMatch(true);

	auto with_3rd_5th = group->ToHTML();

	group->IsFifthPlaceMatch(false);

	auto with_3rd = group->ToHTML();

	EXPECT_NE(none, with_3rd);
	EXPECT_NE(none, with_5th);
	EXPECT_NE(none, with_3rd_5th);

	EXPECT_NE(with_3rd, with_5th);
	EXPECT_NE(with_3rd, with_3rd_5th);

	EXPECT_NE(with_5th, with_3rd_5th);

	delete t;
}



TEST(SingleElimination, Count8_3rd_5th_ExportImport)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	group->IsThirdPlaceMatch(true);
	group->IsFifthPlaceMatch(true);

	YAML::Emitter yaml;
	yaml << YAML::BeginMap;
	*group >> yaml;
	yaml << YAML::EndMap;

	SingleElimination group2(YAML::Load(yaml.c_str()), t);

	t->AddMatchTable(&group2);

	Judoka* j[9];
	bool has_match[9];

	for (int i = 1; i <= 8; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		t->AddParticipant(j[i]);
		has_match[i] = false;
	}

	ASSERT_EQ(group2.GetParticipants().size(), 8);

	for (auto match : group2.GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		for (int i = 1; i <= 8; ++i)
		{
			if (match->GetFighter(Fighter::White)->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
			if (match->GetFighter(Fighter::Blue )->GetUUID() == j[i]->GetUUID())
				has_match[i] = true;
		}
	}

	for (int i = 1; i <= 8; ++i)
		EXPECT_TRUE(has_match[i]);

	Mat m(1);

	for (auto match : group2.GetSchedule())
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

	auto results = group2.CalculateResults();

	//j1 vs j5, j3 vs j7, j2 vs j6, j4 vs j8
	//j5 vs j7, j6 vs j8
	//j7 vs j8

	ASSERT_EQ(results.GetSize(), 6);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[8]->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j[7]->GetUUID());
	EXPECT_EQ(results[2].Judoka->GetUUID(), j[6]->GetUUID());
	EXPECT_EQ(results[3].Judoka->GetUUID(), j[5]->GetUUID());
	EXPECT_EQ(results[4].Judoka->GetUUID(), j[4]->GetUUID());
	EXPECT_EQ(results[5].Judoka->GetUUID(), j[3]->GetUUID());
}