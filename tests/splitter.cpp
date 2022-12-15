#include "tests.h"



TEST(Splitter, Split2JudokaBy2)
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



TEST(Splitter, Split4JudokaBy2)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(Weight(10), Weight(100));

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));
	EXPECT_TRUE(w.AddParticipant(&j3));
	EXPECT_TRUE(w.AddParticipant(&j4));

	Splitter s1(w, 2, 0);
	Splitter s2(w, 2, 1);

	ASSERT_EQ(s1.GetParticipants().size(), 2);
	ASSERT_EQ(s2.GetParticipants().size(), 2);

	EXPECT_EQ(*s1.GetJudokaByStartPosition(0).GetJudoka(), j1);
	EXPECT_EQ(*s1.GetJudokaByStartPosition(1).GetJudoka(), j3);
	EXPECT_EQ(*s2.GetJudokaByStartPosition(0).GetJudoka(), j2);
	EXPECT_EQ(*s2.GetJudokaByStartPosition(1).GetJudoka(), j4);
}



TEST(Splitter, Split4JudokaBy4)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(Weight(10), Weight(100));

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));
	EXPECT_TRUE(w.AddParticipant(&j3));
	EXPECT_TRUE(w.AddParticipant(&j4));

	Splitter s1(w, 4, 0);
	Splitter s2(w, 4, 1);
	Splitter s3(w, 4, 2);
	Splitter s4(w, 4, 3);

	ASSERT_EQ(s1.GetParticipants().size(), 1);
	ASSERT_EQ(s2.GetParticipants().size(), 1);
	ASSERT_EQ(s3.GetParticipants().size(), 1);
	ASSERT_EQ(s4.GetParticipants().size(), 1);

	EXPECT_EQ(*s1.GetJudokaByStartPosition(0).GetJudoka(), j1);
	EXPECT_EQ(*s2.GetJudokaByStartPosition(0).GetJudoka(), j2);
	EXPECT_EQ(*s3.GetJudokaByStartPosition(0).GetJudoka(), j3);
	EXPECT_EQ(*s4.GetJudokaByStartPosition(0).GetJudoka(), j4);
}