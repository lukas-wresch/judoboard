#include "tests.h"



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