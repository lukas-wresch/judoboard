#include "tests.h"



TEST(AgeGroup, ExportImport)
{
	initialize();
	
	for (int i = 0; i < 1000 * 5; i++)
	{
		Database d;
		auto r = new RuleSet(GetRandomName(), rand(), rand(), rand(), rand());
		d.AddRuleSet(r);
		AgeGroup a(GetRandomName(), rand(), rand(), r, d);

		YAML::Emitter yaml;
		a >> yaml;

		AgeGroup a2(YAML::Load(yaml.c_str()), d);

		EXPECT_EQ(a.GetUUID(), a2.GetUUID());
		EXPECT_EQ(a.GetName(), a2.GetName());
		ASSERT_TRUE(a.GetRuleSet());
		ASSERT_TRUE(a2.GetRuleSet());
		EXPECT_EQ(a.GetRuleSet()->GetName(),      a2.GetRuleSet()->GetName());
		EXPECT_EQ(a.GetRuleSet()->GetMatchTime(), a2.GetRuleSet()->GetMatchTime());
		EXPECT_EQ(a.GetRuleSet()->GetGoldenScoreTime(),   a2.GetRuleSet()->GetGoldenScoreTime());
		EXPECT_EQ(a.GetRuleSet()->GetOsaeKomiTime(false), a2.GetRuleSet()->GetOsaeKomiTime(false));
		EXPECT_EQ(a.GetRuleSet()->GetOsaeKomiTime(true),  a2.GetRuleSet()->GetOsaeKomiTime(true));
	}
}