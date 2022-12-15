#include "tests.h"



TEST(Pool, Count6)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j5(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j6(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Pool w(Weight(10), Weight(100));

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
}