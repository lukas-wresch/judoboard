#include "tests.h"



TEST(Judoka, Basic)
{
	initialize();
	Judoka j("Firstname", "Lastname", 50, Gender::Male);

	EXPECT_TRUE(j.GetGender() == Gender::Male);
	EXPECT_TRUE(j.GetWeight() == 50);

	EXPECT_EQ(j.GetName(), std::string("Firstname Lastname"));

	j.SetWeight(80);
	EXPECT_TRUE(j.GetWeight() == 80);

	j.SetGender(Gender::Female);
	EXPECT_TRUE(j.GetGender() == Gender::Female);

	j.SetFirstname("New");
	j.SetLastname("Name");
	EXPECT_EQ(j.GetName(), std::string("New Name"));
}


TEST(Judoka, ImportExport)
{
	for (int i = 0; i < 1000 * 10; i++)
	{
		Judoka j(GetRandomName(), GetRandomName(), rand()%200, (Gender)(rand()%2));

		ZED::CSV csv;
		j >> csv;

		Judoka j2(csv);

		EXPECT_TRUE(j.GetName() == j2.GetName());
		EXPECT_TRUE(j.GetGender() == j2.GetGender());
		EXPECT_TRUE(j.GetWeight() == j2.GetWeight());
		EXPECT_TRUE(j.GetAge() == j2.GetAge());
		EXPECT_TRUE(j.GetUUID() == j2.GetUUID());
		EXPECT_TRUE(j.GetLengthOfBreak() == j2.GetLengthOfBreak());
	}
}


TEST(Judoka, ImportExport_RuleSet)
{
	for (int i = 0; i < 1000 * 10; i++)
	{
		RuleSet r(GetRandomName(), rand() % 200, rand() % 200, rand() % 200, rand() % 200, rand() % 2, rand() % 2, rand() % 2, rand() % 200);

		ZED::CSV csv;
		r >> csv;

		RuleSet r2(csv);

		EXPECT_TRUE(r.GetName() == r2.GetName());
		EXPECT_TRUE(r.IsYukoEnabled() == r2.IsYukoEnabled());
		EXPECT_TRUE(r.IsKokaEnabled() == r2.IsKokaEnabled());
		EXPECT_TRUE(r.IsGoldenScoreEnabled() == r2.IsGoldenScoreEnabled());
		EXPECT_TRUE(r.GetUUID() == r2.GetUUID());
		EXPECT_TRUE(r.IsDrawAllowed() == r2.IsDrawAllowed());
		EXPECT_TRUE(r.GetOsaeKomiTime(true)  == r2.GetOsaeKomiTime(true));
		EXPECT_TRUE(r.GetOsaeKomiTime(false) == r2.GetOsaeKomiTime(false));
		EXPECT_TRUE(r.GetMatchTime() == r2.GetMatchTime());
		EXPECT_TRUE(r.GetGoldenScoreTime() == r2.GetGoldenScoreTime());
		EXPECT_TRUE(r.GetBreakTime() ==   r2.GetBreakTime());
		EXPECT_TRUE(r.GetDescription() == r2.GetDescription());
	}
}