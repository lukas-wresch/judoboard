#include "tests.h"



TEST(LoserBracket, Count14)
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

	LoserBracket* group = new LoserBracket(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < 14; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(), 14);

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