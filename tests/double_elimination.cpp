#include "tests.h"



TEST(LoserBracket, Count14)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 14;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	LoserBracket* group = new LoserBracket(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(), count);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[4]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[8]));

	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[10]));

	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[14]));

	ASSERT_EQ(group->GetParticipants().size(), 14);
	ASSERT_EQ(group->GetSchedule().size(),     13);

	delete t;
}



TEST(DoubleElimination, Count8)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 8;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(), count);

	auto loser_schedule = group->GetLoserBracket().GetSchedule();

	//EXPECT_TRUE(loser_schedule[0]->Contains(*j[1]));

	ASSERT_EQ(loser_schedule.size(),  4 + 4 + 2 + 2 + 1);

	delete t;
}



TEST(DoubleElimination, ExportImport)
{
	initialize();

	for (size_t count = 1; count <= 32; ++count)
	{
		Judoka* j[33];
		bool has_match[33];

		DoubleElimination group(0, 200);

		for (int i = 1; i <= count; ++i)
		{
			j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
			has_match[i] = false;
			group.AddParticipant(j[i]);
		}

		group.SetMatID(1);

		for (int i = 1; i <= count; ++i)
			group.SetStartPosition(j[i], i-1);

		EXPECT_EQ(group.GetMaxStartPositions(), count);

		auto loser_schedule = group.GetLoserBracket().GetSchedule();

		YAML::Emitter yaml;
		group >> yaml;

		DoubleElimination group2(YAML::Load(yaml.c_str()));

		EXPECT_EQ(group.ToHTML(), group2.ToHTML());

		for (int i = 1; i <= count; ++i)
			delete j[i];
	}
}