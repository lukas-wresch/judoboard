#include "tests.h"



TEST(Splitter, Split2Judoka)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(Weight(10), Weight(100));

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));

	Splitter s1(w, 2, 0);
	Splitter s2(w, 2, 1);

	ASSERT_EQ(s1.GetParticipants().size(), 1);
	ASSERT_EQ(s2.GetParticipants().size(), 1);

	EXPECT_EQ(*s1.GetJudokaByStartPosition(0).GetJudoka(), j1);
	EXPECT_EQ(*s2.GetJudokaByStartPosition(0).GetJudoka(), j2);
}