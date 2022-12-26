#include "tests.h"



TEST(Pool, Count4)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 51, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 52, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 53, Gender::Male);

	Pool w(Weight(10), Weight(100));
	w.SetMatID(1);

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));
	EXPECT_TRUE(w.AddParticipant(&j3));
	EXPECT_TRUE(w.AddParticipant(&j4));

	w.SetPoolCount(2);

	ASSERT_EQ(w.GetPoolCount(), 2);

	ASSERT_TRUE(w.GetPool(0));
	ASSERT_TRUE(w.GetPool(1));
	ASSERT_FALSE(w.GetPool(2));

	ASSERT_EQ(w.GetPool(0)->GetParticipants().size(), 2);
	ASSERT_EQ(w.GetPool(1)->GetParticipants().size(), 2);

	EXPECT_EQ(*w.GetPool(0)->GetJudokaByStartPosition(0), j1);
	EXPECT_EQ(*w.GetPool(0)->GetJudokaByStartPosition(1), j3);
	EXPECT_EQ(*w.GetPool(1)->GetJudokaByStartPosition(0), j2);
	EXPECT_EQ(*w.GetPool(1)->GetJudokaByStartPosition(1), j4);

	EXPECT_EQ(w.GetSchedule().size(), 1 + 1 + 3);

	auto schedule = w.GetSchedule();
	auto semi1 = schedule[w.GetSchedule().size() - 3];
	auto semi2 = schedule[w.GetSchedule().size() - 2];

	EXPECT_EQ(semi1->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*semi1->GetDependentMatchTableOf(Fighter::White), *w.GetPool(0));
	EXPECT_EQ(semi1->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank2);
	EXPECT_EQ(*semi1->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(1));

	EXPECT_EQ(semi2->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*semi2->GetDependentMatchTableOf(Fighter::White), *w.GetPool(1));
	EXPECT_EQ(semi2->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank2);
	EXPECT_EQ(*semi2->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(0));

	Mat m(1);

	for (auto match : w.GetSchedule())
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
}



TEST(Pool, Count6)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 51, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 52, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 53, Gender::Male);
	Judoka j5(GetFakeFirstname(), GetFakeLastname(), 54, Gender::Male);
	Judoka j6(GetFakeFirstname(), GetFakeLastname(), 55, Gender::Male);

	Pool w(Weight(10), Weight(100));
	w.SetMatID(1);

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));
	EXPECT_TRUE(w.AddParticipant(&j3));
	EXPECT_TRUE(w.AddParticipant(&j4));
	EXPECT_TRUE(w.AddParticipant(&j5));
	EXPECT_TRUE(w.AddParticipant(&j6));

	w.SetPoolCount(2);

	ASSERT_EQ(w.GetPoolCount(), 2);

	ASSERT_TRUE(w.GetPool(0));
	ASSERT_TRUE(w.GetPool(1));
	ASSERT_FALSE(w.GetPool(2));

	ASSERT_EQ(w.GetPool(0)->GetParticipants().size(), 3);
	ASSERT_EQ(w.GetPool(1)->GetParticipants().size(), 3);

	EXPECT_EQ(*w.GetPool(0)->GetJudokaByStartPosition(0), j1);
	EXPECT_EQ(*w.GetPool(0)->GetJudokaByStartPosition(1), j3);
	EXPECT_EQ(*w.GetPool(0)->GetJudokaByStartPosition(2), j5);
	EXPECT_EQ(*w.GetPool(1)->GetJudokaByStartPosition(0), j2);
	EXPECT_EQ(*w.GetPool(1)->GetJudokaByStartPosition(1), j4);
	EXPECT_EQ(*w.GetPool(1)->GetJudokaByStartPosition(2), j6);

	EXPECT_EQ(w.GetSchedule().size(), 3 + 3 + 3);

	Mat m(1);

	for (auto match : w.GetSchedule())
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

	auto results = w.CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(*results[0].Judoka, j6);
	EXPECT_EQ(*results[1].Judoka, j5);
}



TEST(Pool, Count8)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 51, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 52, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 53, Gender::Male);
	Judoka j5(GetFakeFirstname(), GetFakeLastname(), 54, Gender::Male);
	Judoka j6(GetFakeFirstname(), GetFakeLastname(), 55, Gender::Male);
	Judoka j7(GetFakeFirstname(), GetFakeLastname(), 56, Gender::Male);
	Judoka j8(GetFakeFirstname(), GetFakeLastname(), 57, Gender::Male);

	Pool w(Weight(10), Weight(100));
	w.SetMatID(1);

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));
	EXPECT_TRUE(w.AddParticipant(&j3));
	EXPECT_TRUE(w.AddParticipant(&j4));
	EXPECT_TRUE(w.AddParticipant(&j5));
	EXPECT_TRUE(w.AddParticipant(&j6));
	EXPECT_TRUE(w.AddParticipant(&j7));
	EXPECT_TRUE(w.AddParticipant(&j8));

	w.SetPoolCount(4);

	ASSERT_EQ(w.GetPoolCount(), 4);

	ASSERT_TRUE(w.GetPool(0));
	ASSERT_TRUE(w.GetPool(1));
	ASSERT_TRUE(w.GetPool(2));
	ASSERT_TRUE(w.GetPool(3));

	ASSERT_EQ(w.GetPool(0)->GetParticipants().size(), 2);
	ASSERT_EQ(w.GetPool(1)->GetParticipants().size(), 2);

	EXPECT_EQ(*w.GetPool(0)->GetJudokaByStartPosition(0), j1);
	EXPECT_EQ(*w.GetPool(0)->GetJudokaByStartPosition(1), j5);
	EXPECT_EQ(*w.GetPool(1)->GetJudokaByStartPosition(0), j2);
	EXPECT_EQ(*w.GetPool(1)->GetJudokaByStartPosition(1), j6);
	EXPECT_EQ(*w.GetPool(2)->GetJudokaByStartPosition(0), j3);
	EXPECT_EQ(*w.GetPool(2)->GetJudokaByStartPosition(1), j7);
	EXPECT_EQ(*w.GetPool(3)->GetJudokaByStartPosition(0), j4);
	EXPECT_EQ(*w.GetPool(3)->GetJudokaByStartPosition(1), j8);

	EXPECT_EQ(w.GetSchedule().size(), 1*4 + 4+2+1);

	auto schedule = w.GetSchedule();
	auto quater1 = schedule[w.GetSchedule().size() - 7];
	auto quater2 = schedule[w.GetSchedule().size() - 6];
	auto quater3 = schedule[w.GetSchedule().size() - 5];
	auto quater4 = schedule[w.GetSchedule().size() - 4];
	auto semi1   = schedule[w.GetSchedule().size() - 3];
	auto semi2   = schedule[w.GetSchedule().size() - 2];

	EXPECT_EQ(quater1->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*quater1->GetDependentMatchTableOf(Fighter::White), *w.GetPool(0));
	EXPECT_EQ(quater1->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank2);
	EXPECT_EQ(*quater1->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(1));

	EXPECT_EQ(quater2->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank2);
	EXPECT_EQ(*quater2->GetDependentMatchTableOf(Fighter::White), *w.GetPool(0));
	EXPECT_EQ(quater2->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank1);
	EXPECT_EQ(*quater2->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(1));

	Mat m(1);

	for (auto match : w.GetSchedule())
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
}