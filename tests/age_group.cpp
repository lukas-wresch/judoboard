#include "tests.h"



TEST(AgeGroup, MatchTableTakeRuleSet)
{
	initialize();

	for (int i = 0; i < 1000; i++)
	{
		Tournament t("deleteMe");
		t.EnableAutoSave(false);

		auto r = new RuleSet(GetRandomName(), rand(), rand(), rand(), rand());
		AgeGroup a(GetRandomName(), rand(), rand(), r);

		Match* match = new Match(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), &t, 1);
		t.AddMatch(match);

		ASSERT_EQ(t.GetMatchTables().size(), 1);

		t.GetMatchTables()[0]->SetAgeGroup(&a);

		ASSERT_TRUE(t.GetMatchTables()[0]->GetAgeGroup());
		EXPECT_EQ(*t.GetMatchTables()[0]->GetAgeGroup(), a);
		EXPECT_EQ(t.GetMatchTables()[0]->GetRuleSet(),  *r);
	}
}



TEST(AgeGroup, ExportImport)
{
	initialize();
	
	for (int i = 0; i < 1000 * 5; i++)
	{
		Database d;
		auto r = new RuleSet(GetRandomName(), rand(), rand(), rand(), rand());
		d.AddRuleSet(r);
		AgeGroup a(GetRandomName(), rand(), rand(), r);

		YAML::Emitter yaml;
		a >> yaml;

		AgeGroup a2(YAML::Load(yaml.c_str()), d);

		EXPECT_EQ(a.GetUUID(), a2.GetUUID());
		EXPECT_EQ(a.GetName(), a2.GetName());
		EXPECT_EQ(a.GetMinAge(), a2.GetMinAge());
		EXPECT_EQ(a.GetMaxAge(), a2.GetMaxAge());
		ASSERT_TRUE(a.GetRuleSet());
		ASSERT_TRUE(a2.GetRuleSet());
		EXPECT_EQ(a.GetRuleSet()->GetName(),      a2.GetRuleSet()->GetName());
		EXPECT_EQ(a.GetRuleSet()->GetMatchTime(), a2.GetRuleSet()->GetMatchTime());
		EXPECT_EQ(a.GetRuleSet()->GetGoldenScoreTime(),   a2.GetRuleSet()->GetGoldenScoreTime());
		EXPECT_EQ(a.GetRuleSet()->GetOsaeKomiTime(false), a2.GetRuleSet()->GetOsaeKomiTime(false));
		EXPECT_EQ(a.GetRuleSet()->GetOsaeKomiTime(true),  a2.GetRuleSet()->GetOsaeKomiTime(true));
	}
}



TEST(AgeGroup, Default2021)
{
	initialize();

	{
		Database d;
		d.SetYear(2021);

		Judoka j00(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j00.SetBirthyear(2000);
		Judoka j01(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j01.SetBirthyear(2001);
		Judoka j02(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j02.SetBirthyear(2002);
		Judoka j03(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j03.SetBirthyear(2003);
		Judoka j04(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j04.SetBirthyear(2004);
		Judoka j05(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j05.SetBirthyear(2005);
		Judoka j06(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j06.SetBirthyear(2006);
		Judoka j07(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j07.SetBirthyear(2007);
		Judoka j08(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j08.SetBirthyear(2008);
		Judoka j09(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j09.SetBirthyear(2009);
		Judoka j10(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j10.SetBirthyear(2010);
		Judoka j11(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j11.SetBirthyear(2011);
		Judoka j12(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j12.SetBirthyear(2012);
		Judoka j13(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j13.SetBirthyear(2013);
		Judoka j14(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j14.SetBirthyear(2014);
		Judoka j15(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j15.SetBirthyear(2015);

		auto u11 = d.FindAgeGroupByName("U11");
		auto u13 = d.FindAgeGroupByName("U13");
		auto u15 = d.FindAgeGroupByName("U15");
		auto u18 = d.FindAgeGroupByName("U18");
		auto u21 = d.FindAgeGroupByName("U21");
		auto sen = d.FindAgeGroupByName("Seniors");

		ASSERT_TRUE(u11);
		ASSERT_TRUE(u13);
		ASSERT_TRUE(u15);
		ASSERT_TRUE(u18);
		ASSERT_TRUE(u21);
		ASSERT_TRUE(sen);

		EXPECT_FALSE(u11->IsElgiable(j10, d));
		EXPECT_TRUE( u11->IsElgiable(j11, d));
		EXPECT_TRUE( u11->IsElgiable(j12, d));
		EXPECT_TRUE( u11->IsElgiable(j13, d));
		EXPECT_FALSE(u11->IsElgiable(j14, d));

		EXPECT_FALSE(u13->IsElgiable(j08, d));
		EXPECT_TRUE( u13->IsElgiable(j09, d));
		EXPECT_TRUE( u13->IsElgiable(j10, d));
		EXPECT_TRUE( u13->IsElgiable(j11, d));
		EXPECT_FALSE(u13->IsElgiable(j12, d));

		EXPECT_FALSE(u15->IsElgiable(j06, d));
		EXPECT_TRUE( u15->IsElgiable(j07, d));
		EXPECT_TRUE( u15->IsElgiable(j08, d));
		EXPECT_TRUE( u15->IsElgiable(j09, d));
		EXPECT_FALSE(u15->IsElgiable(j10, d));

		EXPECT_FALSE(u18->IsElgiable(j03, d));
		EXPECT_TRUE( u18->IsElgiable(j04, d));
		EXPECT_TRUE( u18->IsElgiable(j05, d));
		EXPECT_TRUE( u18->IsElgiable(j06, d));
		EXPECT_FALSE(u18->IsElgiable(j07, d));

		EXPECT_FALSE(u21->IsElgiable(j00, d));
		EXPECT_TRUE( u21->IsElgiable(j01, d));
		EXPECT_TRUE( u21->IsElgiable(j02, d));
		EXPECT_TRUE( u21->IsElgiable(j03, d));
		EXPECT_TRUE( u21->IsElgiable(j04, d));
		EXPECT_FALSE(u21->IsElgiable(j05, d));

		EXPECT_FALSE(sen->IsElgiable(j05, d));
		EXPECT_TRUE( sen->IsElgiable(j04, d));
		EXPECT_TRUE( sen->IsElgiable(j03, d));
		EXPECT_TRUE( sen->IsElgiable(j02, d));
		EXPECT_TRUE( sen->IsElgiable(j01, d));
	}
}



TEST(AgeGroup, Default2022)
{
	initialize();

	{
		Database d;
		d.SetYear(2022);

		Judoka j01(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j01.SetBirthyear(2001);
		Judoka j02(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j02.SetBirthyear(2002);
		Judoka j03(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j03.SetBirthyear(2003);
		Judoka j04(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j04.SetBirthyear(2004);
		Judoka j05(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j05.SetBirthyear(2005);
		Judoka j06(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j06.SetBirthyear(2006);
		Judoka j07(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j07.SetBirthyear(2007);
		Judoka j08(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j08.SetBirthyear(2008);
		Judoka j09(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j09.SetBirthyear(2009);
		Judoka j10(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j10.SetBirthyear(2010);
		Judoka j11(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j11.SetBirthyear(2011);
		Judoka j12(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j12.SetBirthyear(2012);
		Judoka j13(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j13.SetBirthyear(2013);
		Judoka j14(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j14.SetBirthyear(2014);
		Judoka j15(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
		j15.SetBirthyear(2015);

		auto u11 = d.FindAgeGroupByName("U11");
		auto u13 = d.FindAgeGroupByName("U13");
		auto u15 = d.FindAgeGroupByName("U15");
		auto u18 = d.FindAgeGroupByName("U18");
		auto u21 = d.FindAgeGroupByName("U21");
		auto sen = d.FindAgeGroupByName("Seniors");

		ASSERT_TRUE(u11);
		ASSERT_TRUE(u13);
		ASSERT_TRUE(u15);
		ASSERT_TRUE(u18);
		ASSERT_TRUE(u21);
		ASSERT_TRUE(sen);

		EXPECT_FALSE(u11->IsElgiable(j11, d));
		EXPECT_TRUE( u11->IsElgiable(j12, d));
		EXPECT_TRUE( u11->IsElgiable(j13, d));
		EXPECT_TRUE( u11->IsElgiable(j14, d));
		EXPECT_FALSE(u11->IsElgiable(j15, d));

		EXPECT_FALSE(u13->IsElgiable(j09, d));
		EXPECT_TRUE( u13->IsElgiable(j10, d));
		EXPECT_TRUE( u13->IsElgiable(j11, d));
		EXPECT_TRUE( u13->IsElgiable(j12, d));
		EXPECT_FALSE(u13->IsElgiable(j13, d));

		EXPECT_FALSE(u15->IsElgiable(j07, d));
		EXPECT_TRUE( u15->IsElgiable(j08, d));
		EXPECT_TRUE( u15->IsElgiable(j09, d));
		EXPECT_TRUE( u15->IsElgiable(j10, d));
		EXPECT_FALSE(u15->IsElgiable(j11, d));

		EXPECT_FALSE(u18->IsElgiable(j04, d));
		EXPECT_TRUE( u18->IsElgiable(j05, d));
		EXPECT_TRUE( u18->IsElgiable(j06, d));
		EXPECT_TRUE( u18->IsElgiable(j07, d));
		EXPECT_FALSE(u18->IsElgiable(j08, d));

		EXPECT_FALSE(u21->IsElgiable(j01, d));
		EXPECT_TRUE( u21->IsElgiable(j02, d));
		EXPECT_TRUE( u21->IsElgiable(j03, d));
		EXPECT_TRUE( u21->IsElgiable(j04, d));
		EXPECT_TRUE( u21->IsElgiable(j05, d));
		EXPECT_FALSE(u21->IsElgiable(j06, d));

		EXPECT_FALSE(sen->IsElgiable(j06, d));
		EXPECT_TRUE( sen->IsElgiable(j05, d));
		EXPECT_TRUE( sen->IsElgiable(j04, d));
		EXPECT_TRUE( sen->IsElgiable(j03, d));
		EXPECT_TRUE( sen->IsElgiable(j02, d));
	}
}



TEST(AgeGroup, Default2022HasCorrectMatchTime)
{
	initialize();

	{
		Database d;
		d.SetYear(2022);

		auto u11 = d.FindAgeGroupByName("U11");
		auto u13 = d.FindAgeGroupByName("U13");
		auto u15 = d.FindAgeGroupByName("U15");
		auto u18 = d.FindAgeGroupByName("U18");
		auto u21 = d.FindAgeGroupByName("U21");
		auto sen = d.FindAgeGroupByName("Seniors");

		ASSERT_TRUE(u11);
		ASSERT_TRUE(u13);
		ASSERT_TRUE(u15);
		ASSERT_TRUE(u18);
		ASSERT_TRUE(u21);
		ASSERT_TRUE(sen);

		ASSERT_TRUE(u11->GetRuleSet());
		ASSERT_TRUE(u13->GetRuleSet());
		ASSERT_TRUE(u15->GetRuleSet());
		ASSERT_TRUE(u18->GetRuleSet());
		ASSERT_TRUE(u21->GetRuleSet());
		ASSERT_TRUE(sen->GetRuleSet());

		EXPECT_EQ(u11->GetRuleSet()->GetMatchTime(), 2*60);
		EXPECT_EQ(u13->GetRuleSet()->GetMatchTime(), 3*60);
		EXPECT_EQ(u15->GetRuleSet()->GetMatchTime(), 3*60);
		EXPECT_EQ(u18->GetRuleSet()->GetMatchTime(), 4*60);
		EXPECT_EQ(u21->GetRuleSet()->GetMatchTime(), 4*60);
		EXPECT_EQ(sen->GetRuleSet()->GetMatchTime(), 4*60);
	}
}