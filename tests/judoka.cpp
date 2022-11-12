#include "tests.h"



TEST(Judoka, Basic)
{
	initialize();
	Judoka j("Firstname", "Lastname", 50, Gender::Male);

	EXPECT_EQ(j.GetGender(), Gender::Male);
	EXPECT_EQ(j.GetWeight(), Weight(50));
	EXPECT_EQ(j.GetNumber(), "");
	EXPECT_EQ(j.GetName(NameStyle::GivenName), std::string("Firstname Lastname"));

	j.SetWeight(80);
	EXPECT_EQ(j.GetWeight(), Weight(80));

	j.SetGender(Gender::Female);
	EXPECT_EQ(j.GetGender(), Gender::Female);

	j.SetFirstname("New");
	j.SetLastname("Name");
	EXPECT_EQ(j.GetName(NameStyle::GivenName), std::string("New Name"));

	j.SetNumber("ABC123");
	EXPECT_EQ(j.GetNumber(), "ABC123");
}



TEST(Judoka, ImportExport)
{
	for (int i = 0; i < 1000; i++)
	{
		Judoka j(GetRandomName(), GetRandomName(), rand()%200, (Gender)(rand()%2));
		j.SetNumber("ABC123");

		YAML::Emitter yaml;
		j >> yaml;

		Judoka j2(YAML::Load(yaml.c_str()), nullptr);

		EXPECT_EQ(j.GetName(NameStyle::GivenName), j2.GetName(NameStyle::GivenName));
		EXPECT_EQ(j.GetGender(), j2.GetGender());
		EXPECT_EQ(j.GetWeight(), j2.GetWeight());
		EXPECT_EQ(j.GetAge(),  j2.GetAge());
		EXPECT_EQ(j.GetUUID(), j2.GetUUID());
		EXPECT_EQ(j.GetLengthOfBreak(), j2.GetLengthOfBreak());
		EXPECT_EQ(j.GetNumber(), j2.GetNumber());
	}
}



TEST(Judoka, ImportExport_RuleSet)
{
	for (int i = 0; i < 1000; i++)
	{
		RuleSet r(GetRandomName(), rand() % 200, rand() % 200, rand() % 200, rand() % 200, rand() % 2, rand() % 2, rand() % 2, rand() % 200);

		YAML::Emitter yaml;
		r >> yaml;

		RuleSet r2(YAML::Load(yaml.c_str()));

		EXPECT_EQ(r.GetName(), r2.GetName());
		EXPECT_EQ(r.IsYukoEnabled(), r2.IsYukoEnabled());
		EXPECT_EQ(r.IsKokaEnabled(), r2.IsKokaEnabled());
		EXPECT_EQ(r.IsGoldenScoreEnabled(), r2.IsGoldenScoreEnabled());
		EXPECT_EQ(r.GetUUID(), r2.GetUUID());
		EXPECT_EQ(r.IsDrawAllowed(), r2.IsDrawAllowed());
		EXPECT_EQ(r.GetOsaeKomiTime(true),  r2.GetOsaeKomiTime(true));
		EXPECT_EQ(r.GetOsaeKomiTime(false), r2.GetOsaeKomiTime(false));
		EXPECT_EQ(r.GetMatchTime(), r2.GetMatchTime());
		EXPECT_EQ(r.GetGoldenScoreTime(), r2.GetGoldenScoreTime());
		EXPECT_EQ(r.GetBreakTime(),  r2.GetBreakTime());
		EXPECT_EQ(r.GetDescription(), r2.GetDescription());
	}
}



TEST(Judoka, Weight)
{
	initialize();

	Weight w(123);
	EXPECT_EQ((uint32_t)w, 123 * 1000);

	Weight w2("12,3");
	EXPECT_EQ((uint32_t)w2, 12 * 1000 + 300);
	EXPECT_EQ(w2.ToString(), "12,3 ");

	Weight w3("12.3");
	EXPECT_EQ((uint32_t)w3, 12 * 1000 + 300);
	EXPECT_EQ(w3.ToString(), "12,3 ");

	Weight w4("12");
	EXPECT_EQ((uint32_t)w4, 12 * 1000);
	EXPECT_EQ(w4.ToString(), "12");

	YAML::Emitter yaml;
	w2 >> yaml;
	w2 = Weight(YAML::Load(yaml.c_str()));
	EXPECT_EQ((uint32_t)w2, 12 * 1000 + 300);
	EXPECT_EQ(w2.ToString(), "12,3 ");
}



TEST(Judoka, WeightWithGrams)
{
	initialize();

	Weight w(123);
	EXPECT_EQ((uint32_t)w, 123 * 1000);

	Weight w2("12,03");
	EXPECT_EQ((uint32_t)w2, 12 * 1000 + 30);
	EXPECT_EQ(w2.ToString(), "12,0 ");

	Weight w3("12.003");
	EXPECT_EQ((uint32_t)w3, 12 * 1000 + 3);
	EXPECT_EQ(w3.ToString(), "12,0 ");

	Weight w4("12");
	EXPECT_EQ((uint32_t)w4, 12 * 1000);
	EXPECT_EQ(w4.ToString(), "12");

	YAML::Emitter yaml;
	w2 >> yaml;
	w2 = Weight(YAML::Load(yaml.c_str()));
	EXPECT_EQ((uint32_t)w2, 12 * 1000 + 30);
	EXPECT_EQ(w2.ToString(), "12,0 ");

	YAML::Emitter yaml2;
	w3 >> yaml2;
	w3 = Weight(YAML::Load(yaml2.c_str()));
	EXPECT_EQ((uint32_t)w3, 12 * 1000 + 3);
	EXPECT_EQ(w3.ToString(), "12,0 ");
}