#include "tests.h"



TEST(Judoka, Basic)
{
	initialize();
	Judoka j("Firstname", "Lastname", 50, Gender::Male);

	EXPECT_EQ(j.GetGender(), Gender::Male);
	EXPECT_EQ(j.GetWeight(), Weight(50));

	EXPECT_EQ(j.GetName(), std::string("Firstname Lastname"));

	j.SetWeight(80);
	EXPECT_EQ(j.GetWeight(), Weight(80));

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

		YAML::Emitter yaml;
		j >> yaml;

		Judoka j2(YAML::Load(yaml.c_str()));

		EXPECT_EQ(j.GetName(), j2.GetName());
		EXPECT_EQ(j.GetGender(), j2.GetGender());
		EXPECT_EQ(j.GetWeight(), j2.GetWeight());
		EXPECT_EQ(j.GetAge(), j2.GetAge());
		EXPECT_EQ(j.GetUUID(), j2.GetUUID());
		EXPECT_EQ(j.GetLengthOfBreak(), j2.GetLengthOfBreak());
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



TEST(Judoka, Weight)
{
	initialize();

	Weight w(123);
	EXPECT_EQ(w, 123 * 1000);

	Weight w2("12,3");
	EXPECT_EQ(w2, 12 * 1000 + 300);
	EXPECT_EQ(w2.ToString(), "12,3 ");

	Weight w3("12.3");
	EXPECT_EQ(w3, 12 * 1000 + 300);
	EXPECT_EQ(w3.ToString(), "12,3 ");

	Weight w4("12");
	EXPECT_EQ(w4, 12 * 1000);
	EXPECT_EQ(w4.ToString(), "12");

	YAML::Emitter yaml;
	w2 >> yaml;
	w2 = Weight(YAML::Load(yaml.c_str()));
	EXPECT_EQ(w2, 12 * 1000 + 300);
	EXPECT_EQ(w2.ToString(), "12,3 ");
}



TEST(Judoka, WeightWithGrams)
{
	initialize();

	Weight w(123);
	EXPECT_EQ(w, 123 * 1000);

	Weight w2("12,03");
	EXPECT_EQ(w2, 12 * 1000 + 30);
	EXPECT_EQ(w2.ToString(), "12,0 ");

	Weight w3("12.003");
	EXPECT_EQ(w3, 12 * 1000 + 3);
	EXPECT_EQ(w3.ToString(), "12,0 ");

	Weight w4("12");
	EXPECT_EQ(w4, 12 * 1000);
	EXPECT_EQ(w4.ToString(), "12");

	YAML::Emitter yaml;
	w2 >> yaml;
	w2 = Weight(YAML::Load(yaml.c_str()));
	EXPECT_EQ(w2, 12 * 1000 + 30);
	EXPECT_EQ(w2.ToString(), "12,0 ");

	YAML::Emitter yaml2;
	w3 >> yaml2;
	w3 = Weight(YAML::Load(yaml2.c_str()));
	EXPECT_EQ(w3, 12 * 1000 + 3);
	EXPECT_EQ(w3.ToString(), "12,0 ");
}