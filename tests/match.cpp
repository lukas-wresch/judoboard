#include "tests.h"



TEST(Match, ExportImport)
{
	initialize();
	
	for (int i = 0; i < 1000 * 5; i++)
	{
		Judoka j1(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		Judoka j2(GetRandomName(), GetRandomName(), rand() % 200, (Gender)(rand() % 2));
		int matid = rand();
		RuleSet rule_set("test", rand(), rand(), rand(), rand());
		Tournament tourney;

		Match match(&tourney, &j1, &j2, matid);
		match.SetRuleSet(&rule_set);
		tourney.AddMatch(&match);//Also copies the rule set inside the tournment's database

		ZED::CSV csv;
		match >> csv;

		Match match2(csv, &tourney);

		ASSERT_EQ(match.GetFighter(Fighter::White)->GetUUID(), match2.GetFighter(Fighter::White)->GetUUID());
		ASSERT_EQ(match.GetFighter(Fighter::Blue )->GetUUID(), match2.GetFighter(Fighter::Blue )->GetUUID());

		ASSERT_EQ(match.GetRuleSet().GetUUID(), match2.GetRuleSet().GetUUID());
		//ASSERT_EQ((std::string)match.AllToString(), (std::string)match2.AllToString());//Will fail due to ID mismatch
		ASSERT_EQ(match.GetColor(), match2.GetColor());
		ASSERT_EQ(match.GetUUID(), match2.GetUUID());
		ASSERT_EQ(match.HasConcluded(), match2.HasConcluded());
		ASSERT_EQ(match.IsRunning(), match2.IsRunning());
	}
}