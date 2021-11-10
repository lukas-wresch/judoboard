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
		EXPECT_EQ(d.FindJudoka(j1.GetUUID())->GetWeight(), 50);
		ASSERT_TRUE(d.FindJudoka(j2.GetUUID()));
		EXPECT_EQ(d.FindJudoka(j2.GetUUID())->GetWeight(), 60);
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

		Judoka j1("Firstname",  "Lastname",  50, Gender::Male);
		Judoka j2("Firstname2", "Lastname2", 60, Gender::Female);

		d.AddJudoka(&j1);
		d.AddJudoka(&j2);

		EXPECT_NE(j1.GetID(),   j2.GetID());
		EXPECT_NE(j1.GetUUID(), j2.GetUUID());

		d.AddRuleSet(new RuleSet("Test", 60, 30, 20, 10, true, true, true, 1));

		EXPECT_TRUE(d.FindJudoka(j1.GetID())->GetWeight() == 50);
		EXPECT_TRUE(d.FindJudoka(j2.GetID())->GetWeight() == 60);

		EXPECT_TRUE(d.Save("temp.csv"));


		Database d2;
		EXPECT_TRUE(d2.Load("temp.csv"));

		EXPECT_TRUE(d2.GetNumJudoka() == 2);

		ASSERT_TRUE(d2.FindJudoka(j1.GetUUID()));
		EXPECT_EQ(d2.FindJudoka(j1.GetUUID())->GetUUID(), j1.GetUUID());
		EXPECT_EQ(d2.FindJudoka(j1.GetUUID())->GetWeight(), 50);
		EXPECT_EQ(d2.FindJudoka(j1.GetUUID())->GetGender(), j1.GetGender());

		ASSERT_TRUE(d2.FindJudoka(j2.GetUUID()));
		EXPECT_EQ(d2.FindJudoka(j2.GetUUID())->GetUUID(), j2.GetUUID());
		EXPECT_EQ(d2.FindJudoka(j2.GetUUID())->GetWeight(), 60);
		EXPECT_EQ(d2.FindJudoka(j2.GetUUID())->GetGender(), j2.GetGender());

		auto rule_set = d2.FindRuleSetByName("Test");
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
	initialize();
	{
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

	EXPECT_EQ(d.GetNumAccounts(), 0);

	EXPECT_EQ(d.GetRuleSets().size(), 1);
	EXPECT_EQ(d.GetRuleSets()[0]->GetName(), "Default");
}



TEST(Database, AccountTest)
{
	initialize();
	{
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



TEST(Database, Nonces)
{
	const int size = 1000;
	Account::Nonce* nonces[size] = {};

	int ip   = rand();
	int port = rand();

	for (int i = 0; i < size; i++)
		nonces[i] = new Account::Nonce(ip, port);

	for (int i = 0; i < size; i++)
		for (int j = i + 1; j < size; j++)
			ASSERT_NE(nonces[i]->GetChallenge(), nonces[j]->GetChallenge());
}



TEST(Database, Login)
{
	initialize();
	srand(ZED::Core::CurrentTimestamp());

	Database d;
	EXPECT_EQ(d.GetNumAccounts(), 0);

	for (int i = 0; i < 300; i++)
	{
		int ip   = rand();
		int port = rand();

		std::string username = "Username" + std::to_string(rand()) + std::to_string(rand());
		std::string password = "Password" + std::to_string(rand());

		Account acc(username, password);

		d.AddAccount(acc);

		auto mynonce = d.CreateNonce(ip, port);

		EXPECT_EQ(mynonce.GetIP(), ip);

		std::string response = ZED::SHA512(acc.GetUsername() + mynonce.GetChallenge() + password);

		ASSERT_FALSE(d.IsLoggedIn(ip, response));
		ASSERT_TRUE(d.DoLogin(acc.GetUsername(), ip, response));
		ASSERT_TRUE(d.IsLoggedIn(ip, response));

		for (auto nonce : d.GetNonces())
		{
			if (nonce.first.GetChallenge() == mynonce.GetChallenge())
				EXPECT_EQ(nonce.second->GetUsername(), acc.GetUsername());
			else
				EXPECT_NE(nonce.second->GetUsername(), acc.GetUsername());
		}
	}
}



TEST(Database, RuleSets)
{
	initialize();
	srand(ZED::Core::CurrentTimestamp());

	Database d;

	for (int i = 0; i < 1000; i++)
	{
		std::string name = "Rules" + std::to_string(rand()) + std::to_string(rand()) + std::to_string(rand());

		RuleSet* rules = new RuleSet(name, 60, 30, 30, 20);

		ASSERT_TRUE(d.FindRuleSetByName(name) == nullptr);

		d.AddRuleSet(rules);

		ASSERT_TRUE(d.FindRuleSetByName(name));

		EXPECT_EQ(d.FindRuleSetByName(name)->GetUUID(), rules->GetUUID());
		EXPECT_EQ(d.FindRuleSetByName(name)->GetName(), rules->GetName());
	}
}