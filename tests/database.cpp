#include "tests.h"



TEST(Database, JudokaTest)
{
	initialize();
	ZED::Core::RemoveFile("temp.csv");
	ZED::Core::RemoveFile("temp2.csv");

	{
		Database d;

		EXPECT_TRUE(d.GetNumJudoka() == 0);

		Judoka j1("Firstname", "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 60, Gender::Female);

		d.AddJudoka(&j1);
		d.AddJudoka(&j2);

		EXPECT_TRUE(d.FindJudoka(j1.GetID())->GetWeight() == 50);
		EXPECT_TRUE(d.FindJudoka(j2.GetID())->GetWeight() == 60);

		EXPECT_TRUE(d.Save("temp.csv"));
		EXPECT_TRUE(d.Save("temp2.csv"));

		ASSERT_TRUE(d.DeleteJudoka(j1.GetID()));
		EXPECT_FALSE(d.DeleteJudoka(j1.GetID()));

		ASSERT_TRUE(d.FindJudoka(j1.GetID()) == nullptr);
		EXPECT_TRUE(d.FindJudoka(j2.GetID())->GetWeight() == 60);

		EXPECT_TRUE(d.DeleteJudoka(j2.GetID()));
		EXPECT_FALSE(d.DeleteJudoka(j2.GetID()));

		EXPECT_TRUE(d.FindJudoka(j1.GetID()) == nullptr);
		EXPECT_TRUE(d.FindJudoka(j2.GetID()) == nullptr);

		EXPECT_TRUE(d.Load("temp.csv"));

		ASSERT_TRUE(d.FindJudoka(j1.GetUUID()));
		EXPECT_TRUE(d.FindJudoka(j1.GetUUID())->GetWeight() == 50);
		ASSERT_TRUE(d.FindJudoka(j2.GetUUID()));
		EXPECT_TRUE(d.FindJudoka(j2.GetUUID())->GetWeight() == 60);
	}

	ZED::Core::RemoveFile("temp.csv");
	ZED::Core::RemoveFile("temp2.csv");
}




TEST(Database, SaveAndLoad)
{
	initialize();

	{
		Database d;

		EXPECT_TRUE(d.GetNumJudoka() == 0);
		EXPECT_TRUE(d.GetRuleSets().size() == 1);

		Judoka j1("Firstname", "Lastname", 50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 60, Gender::Female);

		d.AddJudoka(&j1);
		d.AddJudoka(&j2);

		d.AddRuleSet(new RuleSet("Test", 60, 30, 20, 10, true, true, true, 1));

		EXPECT_TRUE(d.FindJudoka(j1.GetID())->GetWeight() == 50);
		EXPECT_TRUE(d.FindJudoka(j2.GetID())->GetWeight() == 60);

		EXPECT_TRUE(d.Save("temp.csv"));


		Database d2;
		d2.Load("temp.csv");

		EXPECT_TRUE(d2.GetNumJudoka() == 2);

		ASSERT_TRUE(d2.FindJudoka(j1.GetUUID()));
		EXPECT_TRUE(d2.FindJudoka(j1.GetUUID())->GetUUID() == j1.GetUUID());
		EXPECT_TRUE(d2.FindJudoka(j1.GetUUID())->GetWeight() == 50);
		EXPECT_TRUE(d2.FindJudoka(j1.GetUUID())->GetGender() == j1.GetGender());

		ASSERT_TRUE(d2.FindJudoka(j2.GetUUID()));
		EXPECT_TRUE(d2.FindJudoka(j2.GetUUID())->GetUUID() == j2.GetUUID());
		EXPECT_TRUE(d2.FindJudoka(j2.GetUUID())->GetWeight() == 60);
		EXPECT_TRUE(d2.FindJudoka(j2.GetUUID())->GetGender() == j2.GetGender());

		auto rule_set = d2.FindRuleSet("Test");
		ASSERT_TRUE(rule_set);
		EXPECT_TRUE(rule_set->GetMatchTime() == 60);
		EXPECT_TRUE(rule_set->GetGoldenScoreTime() == 30);
		EXPECT_TRUE(rule_set->GetOsaeKomiTime(false) == 20);
		EXPECT_TRUE(rule_set->GetOsaeKomiTime(true) == 10);
		EXPECT_TRUE(rule_set->IsYukoEnabled());
		EXPECT_TRUE(rule_set->IsKokaEnabled());
		EXPECT_TRUE(rule_set->IsDrawAllowed());
		EXPECT_TRUE(rule_set->GetBreakTime() == 1);
	}

	ZED::Core::RemoveFile("temp.csv");
	ZED::Core::RemoveFile("temp2.csv");
}



TEST(Database, OnlyOneDefaultRuleSet)
{
	{
		initialize();
		Database d;

		EXPECT_TRUE(d.Save("temp.csv"));

		Database d2;
		d2.Load("temp.csv");
		d2.Save("temp.csv");

		Database d3;
		d3.Load("temp.csv");

		EXPECT_TRUE(d3.GetRuleSets().size() == 1);
	}

	ZED::Core::RemoveFile("temp.csv");
}



TEST(Database, EmptyDatabaseShouldHaveDefaultRuleSet)
{
	initialize();
	Database d;

	EXPECT_TRUE(d.GetNumAccounts() == 0);

	EXPECT_TRUE(d.GetRuleSets().size() == 1);
	EXPECT_TRUE(d.GetRuleSets()[0]->GetName() == "Default");
}



TEST(Database, AccountTest)
{
	{
		initialize();
		Database d;

		EXPECT_TRUE(d.GetNumAccounts() == 0);

		Account j1("Username", "Password");
		Account j2("Username2", "Password2");

		d.AddAccount(j1);
		d.AddAccount(j2);

		EXPECT_EQ(d.FindAccount(j1.GetUsername())->GetUsername(), j1.GetUsername());
		EXPECT_EQ(d.FindAccount(j2.GetUsername())->GetUsername(), j2.GetUsername());

		EXPECT_TRUE(d.Save("temp.csv"));
		EXPECT_TRUE(d.Save("temp2.csv"));

		EXPECT_TRUE(d.DeleteAccount(j1.GetUsername()));
		EXPECT_FALSE(d.DeleteAccount(j1.GetUsername()));

		EXPECT_TRUE(d.FindAccount(j1.GetUsername()) == nullptr);
		EXPECT_EQ(d.FindAccount(j2.GetUsername())->GetUsername(), j2.GetUsername());

		EXPECT_TRUE(d.DeleteAccount(j2.GetUsername()));
		EXPECT_FALSE(d.DeleteAccount(j2.GetUsername()));

		EXPECT_TRUE(d.FindAccount(j1.GetUsername()) == nullptr);
		EXPECT_TRUE(d.FindAccount(j2.GetUsername()) == nullptr);

		EXPECT_TRUE(d.Load("temp.csv"));

		EXPECT_EQ(d.FindAccount(j1.GetUsername())->GetUsername(), j1.GetUsername());
		EXPECT_EQ(d.FindAccount(j2.GetUsername())->GetUsername(), j2.GetUsername());
	}

	ZED::Core::RemoveFile("temp.csv");
	ZED::Core::RemoveFile("temp2.csv");
}



TEST(Database, Login)
{
	srand(ZED::Core::CurrentTimestamp());

	Database d;
	EXPECT_TRUE(d.GetNumAccounts() == 0);

	for (int i = 0; i < 300; i++)
	{
		int ip = rand();
		int port = rand();

		std::string username = "Username" + std::to_string(rand()) + std::to_string(rand());
		std::string password = "Password" + std::to_string(rand());

		Account j1(username, password);

		d.AddAccount(j1);

		auto mynonce = d.CreateNonce(ip, port);

		EXPECT_TRUE(mynonce.GetIP() == ip);

		std::string response = ZED::SHA512(j1.GetUsername() + mynonce.GetChallenge() + password);

		EXPECT_FALSE(d.CheckLogin(ip, response));
		EXPECT_TRUE(d.DoLogin(j1.GetUsername(), ip, response));
		EXPECT_TRUE(d.CheckLogin(ip, response));

		for (auto nonce : d.GetNonces())
		{
			if (nonce.first.GetChallenge() == mynonce.GetChallenge())
				EXPECT_TRUE(nonce.second->GetUsername() == j1.GetUsername());
			else
				EXPECT_FALSE(nonce.second->GetUsername() == j1.GetUsername());
		}
	}
}



TEST(Database, RuleSets)
{
	srand(ZED::Core::CurrentTimestamp());

	Database d;

	for (int i = 0; i < 1000; i++)
	{
		int ip = rand();
		int port = rand();

		std::string name = "Rules" + std::to_string(rand()) + std::to_string(rand());

		RuleSet* rules = new RuleSet(name, 60, 30, 30, 20);

		EXPECT_TRUE(d.FindRuleSet(name) == nullptr);

		d.AddRuleSet(rules);

		EXPECT_TRUE(d.FindRuleSet(name)->GetUUID() == rules->GetUUID());
		EXPECT_TRUE(d.FindRuleSet(name)->GetName() == rules->GetName());
	}
}