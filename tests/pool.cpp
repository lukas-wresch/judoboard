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

	auto results = w.CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(*results[0].Judoka, j4);
	EXPECT_EQ(*results[1].Judoka, j3);
}



TEST(Pool, Count4_Pool2Top1)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 51, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 52, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 53, Gender::Male);

	Pool w(Weight(10), Weight(100));
	w.SetMatID(1);
	w.SetTakeTop(1);

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

	EXPECT_EQ(w.GetSchedule().size(), 1 + 1 + 1);

	auto schedule = w.GetSchedule();
	auto finals = schedule[w.GetSchedule().size() - 1];

	ASSERT_TRUE(finals->GetDependentMatchTableOf(Fighter::White));
	EXPECT_EQ(finals->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*finals->GetDependentMatchTableOf(Fighter::White), *w.GetPool(0));
	EXPECT_EQ(finals->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank1);
	EXPECT_EQ(*finals->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(1));

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
	EXPECT_EQ(*results[0].Judoka, j4);
	EXPECT_EQ(*results[1].Judoka, j3);
}



TEST(Pool, Count4_3rdPlace)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 51, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 52, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 53, Gender::Male);

	Pool w(Weight(10), Weight(100));
	w.SetMatID(1);
	w.IsThirdPlaceMatch(true);

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

	EXPECT_EQ(w.GetSchedule().size(), 1 + 1 + 3 + 1);

	auto schedule = w.GetSchedule();
	auto semi1 = schedule[w.GetSchedule().size() - 4];
	auto semi2 = schedule[w.GetSchedule().size() - 3];

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

	auto results = w.CalculateResults();

	ASSERT_EQ(results.GetSize(), 4);
	EXPECT_EQ(*results[0].Judoka, j4);
	EXPECT_EQ(*results[1].Judoka, j3);
	EXPECT_EQ(*results[2].Judoka, j2);
	EXPECT_EQ(*results[3].Judoka, j1);
}



TEST(Pool, Count6_Pool2)
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



TEST(Pool, Count8_Pool4)
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
	EXPECT_NE(*quater1->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(0));
	EXPECT_NE(*quater1->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(2));

	EXPECT_EQ(quater2->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*quater2->GetDependentMatchTableOf(Fighter::White), *w.GetPool(2));
	EXPECT_EQ(quater2->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank2);
	EXPECT_NE(*quater2->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(0));
	EXPECT_NE(*quater2->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(2));

	EXPECT_EQ(quater3->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*quater3->GetDependentMatchTableOf(Fighter::White), *w.GetPool(1));
	EXPECT_EQ(quater3->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank2);
	EXPECT_NE(*quater3->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(1));
	EXPECT_NE(*quater3->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(3));

	EXPECT_EQ(quater4->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*quater4->GetDependentMatchTableOf(Fighter::White), *w.GetPool(3));
	EXPECT_EQ(quater4->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank2);
	EXPECT_NE(*quater4->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(1));
	EXPECT_NE(*quater4->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(3));

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
	EXPECT_EQ(*results[0].Judoka, j8);
	EXPECT_EQ(*results[1].Judoka, j7);
}



TEST(Pool, Count8_Pool4Top1)
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
	w.SetTakeTop(1);

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

	EXPECT_EQ(w.GetSchedule().size(), 1*4 + 2+1);

	auto schedule = w.GetSchedule();
	auto semi1   = schedule[w.GetSchedule().size() - 3];
	auto semi2   = schedule[w.GetSchedule().size() - 2];

	EXPECT_EQ(semi1->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*semi1->GetDependentMatchTableOf(Fighter::White), *w.GetPool(0));
	EXPECT_EQ(semi1->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank1);
	EXPECT_EQ(*semi1->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(2));

	EXPECT_EQ(semi2->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*semi2->GetDependentMatchTableOf(Fighter::White), *w.GetPool(1));
	EXPECT_EQ(semi2->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank1);
	EXPECT_EQ(*semi2->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(3));

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
	EXPECT_EQ(*results[0].Judoka, j8);
	EXPECT_EQ(*results[1].Judoka, j7);
}



TEST(Pool, Count8_Pool2Top3_5Place)
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

	w.SetTakeTop(2);
	w.IsThirdPlaceMatch(true);
	w.IsFifthPlaceMatch(true);

	ASSERT_EQ(w.GetPoolCount(), 2);

	ASSERT_TRUE(w.GetPool(0));
	ASSERT_TRUE(w.GetPool(1));
	ASSERT_FALSE(w.GetPool(2));
	ASSERT_FALSE(w.GetPool(3));

	ASSERT_EQ(w.GetPool(0)->GetParticipants().size(), 4);
	ASSERT_EQ(w.GetPool(1)->GetParticipants().size(), 4);

	EXPECT_EQ(w.GetSchedule().size(), 6*2 + 2+1 + 1 + 1);

	EXPECT_EQ(w.GetFinals()->GetSchedule().size(), 2+1 + 1 + 1);

	auto schedule = w.GetSchedule();
	/*auto quater1 = schedule[w.GetSchedule().size() - 9];
	auto quater2 = schedule[w.GetSchedule().size() - 8];
	auto quater3 = schedule[w.GetSchedule().size() - 7];
	auto quater4 = schedule[w.GetSchedule().size() - 6];*/
	auto semi1   = schedule[w.GetSchedule().size() - 5];
	auto semi2   = schedule[w.GetSchedule().size() - 4];
	auto fifth   = schedule[w.GetSchedule().size() - 3];
	auto third   = schedule[w.GetSchedule().size() - 2];
	auto finals  = schedule[w.GetSchedule().size() - 1];

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

	ASSERT_EQ(results.GetSize(), 6);
	EXPECT_EQ(*results[0].Judoka, j8);
	EXPECT_EQ(*results[1].Judoka, j7);
	EXPECT_EQ(*results[2].Judoka, j6);
	EXPECT_EQ(*results[3].Judoka, j5);
	EXPECT_EQ(*results[4].Judoka, j4);
	EXPECT_EQ(*results[5].Judoka, j3);
}



TEST(Pool, Count8_Pool2Top3)
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

	w.SetTakeTop(3);

	ASSERT_EQ(w.GetPoolCount(), 2);

	ASSERT_TRUE(w.GetPool(0));
	ASSERT_TRUE(w.GetPool(1));
	ASSERT_FALSE(w.GetPool(2));
	ASSERT_FALSE(w.GetPool(3));

	ASSERT_EQ(w.GetPool(0)->GetParticipants().size(), 4);
	ASSERT_EQ(w.GetPool(1)->GetParticipants().size(), 4);

	EXPECT_EQ(w.GetSchedule().size(), 6*2 + 4+2+1);

	EXPECT_EQ(w.GetFinals()->GetSchedule().size(), 4+2+1);

	auto schedule = w.GetSchedule();
	auto quater1 = schedule[w.GetSchedule().size() - 7];
	auto quater2 = schedule[w.GetSchedule().size() - 6];
	auto quater3 = schedule[w.GetSchedule().size() - 5];
	auto quater4 = schedule[w.GetSchedule().size() - 4];
	auto semi1   = schedule[w.GetSchedule().size() - 3];
	auto semi2   = schedule[w.GetSchedule().size() - 2];

	ASSERT_TRUE(quater1->GetDependentMatchTableOf(Fighter::White));
	EXPECT_EQ(quater1->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank1);
	EXPECT_EQ(*quater1->GetDependentMatchTableOf(Fighter::White), *w.GetPool(0));
	EXPECT_TRUE(quater1->IsEmptyMatch());

	ASSERT_TRUE(quater2->GetDependentMatchTableOf(Fighter::White));
	ASSERT_TRUE(quater2->GetDependentMatchTableOf(Fighter::Blue));
	EXPECT_EQ(quater2->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank2);
	EXPECT_EQ(*quater2->GetDependentMatchTableOf(Fighter::White), *w.GetPool(0));
	EXPECT_EQ(quater2->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank3);
	EXPECT_EQ(*quater2->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(1));

	ASSERT_TRUE(quater3->GetDependentMatchTableOf(Fighter::White));
	EXPECT_EQ(quater3->GetDependencyTypeOf(Fighter::White),  DependencyType::TakeRank1);
	EXPECT_EQ(*quater3->GetDependentMatchTableOf(Fighter::White),  *w.GetPool(1));
	EXPECT_TRUE(quater3->IsEmptyMatch());

	ASSERT_TRUE(quater4->GetDependentMatchTableOf(Fighter::White));
	ASSERT_TRUE(quater4->GetDependentMatchTableOf(Fighter::Blue));
	EXPECT_EQ(quater4->GetDependencyTypeOf(Fighter::White), DependencyType::TakeRank2);
	EXPECT_EQ(*quater4->GetDependentMatchTableOf(Fighter::White), *w.GetPool(1));
	EXPECT_EQ(quater4->GetDependencyTypeOf(Fighter::Blue),  DependencyType::TakeRank3);
	EXPECT_EQ(*quater4->GetDependentMatchTableOf(Fighter::Blue),  *w.GetPool(0));

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
	EXPECT_EQ(*results[0].Judoka, j8);
	EXPECT_EQ(*results[1].Judoka, j7);
}



TEST(Pool, PoolsOnDifferentMats)
{
	initialize();

	Pool w(Weight(10), Weight(100));
	w.SetMatID(1);

	for (int i = 0; i < 16; i++)
	{
		Judoka* j = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i, Gender::Male);
		EXPECT_TRUE(w.AddParticipant(j));
	}

	ASSERT_TRUE(w.GetPool(0));
	ASSERT_TRUE(w.GetPool(1));
	ASSERT_TRUE(w.GetPool(2));
	ASSERT_TRUE(w.GetPool(3));
	
	w.GetPool(0)->SetMatID(1);
	w.GetPool(1)->SetMatID(2);
	w.GetPool(2)->SetMatID(3);
	w.GetPool(3)->SetMatID(4);
	w.GetFinals()->SetMatID(5);
	
	w.GenerateSchedule();

	Mat m(1);
	int count[5] = {0, 0, 0, 0, 0};

	for (auto match : w.GetSchedule())
	{
		ASSERT_TRUE(!match->IsEmptyMatch());

		//Check if match is on the right mat
		auto mat_id = match->GetMatID();

		count[mat_id-1]++;

		if (1 <= mat_id && mat_id <= 4)
			EXPECT_TRUE(w.GetPool(mat_id-1)->FindMatch(*match));
		else
			EXPECT_TRUE(w.GetFinals()->FindMatch(*match));

		m.SetMatID(match->GetMatID());
		EXPECT_TRUE(m.StartMatch(match));

		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);

		EXPECT_TRUE(m.EndMatch());
	}

	EXPECT_EQ(count[0], 6);
	EXPECT_EQ(count[1], 6);
	EXPECT_EQ(count[2], 6);
	EXPECT_EQ(count[3], 6);
	EXPECT_EQ(count[4], 7);
}



TEST(Pool, PoolsOnDifferentMats_ExportImport)
{
	initialize();

	Tournament tourney;
	Tournament tourney2;
	auto w = std::make_shared<Pool>(Weight(10), Weight(100));
	w->SetMatID(1);
	tourney.AddMatchTable(w);

	for (int i = 0; i < 16; i++)
	{
		Judoka* j = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i, Gender::Male);
		EXPECT_TRUE(w->AddParticipant(j));
		EXPECT_TRUE(tourney2.AddParticipant(j));
	}

	ASSERT_TRUE(w->GetPool(0));
	ASSERT_TRUE(w->GetPool(1));
	ASSERT_TRUE(w->GetPool(2));
	ASSERT_TRUE(w->GetPool(3));

	w->GetPool(0)->SetMatID(1);
	w->GetPool(1)->SetMatID(2);
	w->GetPool(2)->SetMatID(3);
	w->GetPool(3)->SetMatID(4);
	w->GetFinals()->SetMatID(5);

	w->GenerateSchedule();

	YAML::Emitter yaml;
	*w >> yaml;

	Pool w2(nullptr, &tourney2);
	w2.LoadYaml(YAML::Load(yaml.c_str()));

	ASSERT_EQ(w2.GetSchedule().size(), w->GetSchedule().size());

	EXPECT_EQ(w2.GetPool(0)->GetMatID(), 1);
	EXPECT_EQ(w2.GetPool(1)->GetMatID(), 2);
	EXPECT_EQ(w2.GetPool(2)->GetMatID(), 3);
	EXPECT_EQ(w2.GetPool(3)->GetMatID(), 4);
	EXPECT_EQ(w2.GetFinals()->GetMatID(), 5);

	//Check UUIDs
	EXPECT_EQ(*w2.GetSchedule()[0], *w->GetSchedule()[0]);
	EXPECT_EQ(*w2.GetPool(0), *w->GetPool(0));
	EXPECT_EQ(*w2.GetPool(1), *w->GetPool(1));
	EXPECT_EQ(*w2.GetPool(2), *w->GetPool(2));
	EXPECT_EQ(*w2.GetPool(3), *w->GetPool(3));
	EXPECT_EQ(*w2.GetFinals(), *w->GetFinals());

	EXPECT_EQ(w2.GetFinals()->GetParticipants(), w->GetFinals()->GetParticipants());
	EXPECT_EQ(w2.GetFinals()->GetFilter()->GetParticipants().size(), w->GetFinals()->GetFilter()->GetParticipants().size());
	EXPECT_EQ(w2.ToHTML(), w->ToHTML());

	Mat m(1);
	int count[5] = {0, 0, 0, 0, 0};

	for (auto match : w->GetSchedule())
	{
		ASSERT_TRUE(!match->IsEmptyMatch());

		m.SetMatID(match->GetMatID());
		EXPECT_TRUE(m.StartMatch(match));

		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);

		EXPECT_TRUE(m.EndMatch());
	}


	for (auto match : w2.GetSchedule())
	{
		if (match->IsEmptyMatch())
			int t = 546;
		ASSERT_TRUE(!match->IsEmptyMatch());

		//Check if match is on the right mat
		auto mat_id = match->GetMatID();

		count[mat_id-1]++;

		if (1 <= mat_id && mat_id <= 4)
			EXPECT_TRUE(w2.GetPool(mat_id-1)->FindMatch(*match));
		else
			EXPECT_TRUE(w2.GetFinals()->FindMatch(*match));

		m.SetMatID(match->GetMatID());
		EXPECT_TRUE(m.StartMatch(match));

		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);

		EXPECT_TRUE(m.EndMatch());
	}

	EXPECT_EQ(count[0], 6);
	EXPECT_EQ(count[1], 6);
	EXPECT_EQ(count[2], 6);
	EXPECT_EQ(count[3], 6);
	EXPECT_EQ(count[4], 7);
}