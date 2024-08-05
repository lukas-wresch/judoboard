#include "tests.h"



TEST(Ajax, AgeGroup_Add)
{
	initialize();

	{
		Application app;

		auto r = app.GetDatabase().GetRuleSets()[0];
		app.Ajax_AddAgeGroup(HttpServer::Request("", "name=test&min_age=3&max_age=5&gender=0&rule=" + (std::string)r->GetUUID()));
		
		auto& age_groups = app.GetDatabase().GetAgeGroups();

		ASSERT_EQ(age_groups.size(), 7);
		EXPECT_EQ(age_groups[6]->GetName(), "test");
		EXPECT_EQ(age_groups[6]->GetMinAge(), 3);
		EXPECT_EQ(age_groups[6]->GetMaxAge(), 5);
		EXPECT_EQ(age_groups[6]->GetGender(), Gender::Male);
		ASSERT_TRUE(age_groups[6]->GetRuleSet());
		EXPECT_EQ(*age_groups[6]->GetRuleSet(), *r);
	}
}



TEST(Ajax, AgeGroup_Edit)
{
	initialize();

	{
		Application app;

		auto r = app.GetDatabase().GetRuleSets()[0];
		app.Ajax_AddAgeGroup(HttpServer::Request("", "name=test&min_age=3&max_age=5&gender=0&rule=" + (std::string)r->GetUUID()));

		auto& age_groups = app.GetDatabase().GetAgeGroups();
		ASSERT_EQ(age_groups.size(), 7);
		auto a = age_groups[6];

		app.Ajax_EditAgeGroup(HttpServer::Request("id=" + (std::string)a->GetUUID(), "name=test2&min_age=6&max_age=10&gender=1&rule=" + (std::string)r->GetUUID()));

		EXPECT_EQ(age_groups[6]->GetName(), "test2");
		EXPECT_EQ(age_groups[6]->GetMinAge(), 6);
		EXPECT_EQ(age_groups[6]->GetMaxAge(), 10);
		EXPECT_EQ(age_groups[6]->GetGender(), Gender::Female);
		ASSERT_TRUE(age_groups[6]->GetRuleSet());
		EXPECT_EQ(*age_groups[6]->GetRuleSet(), *r);


		auto a1 = std::make_shared<AgeGroup>("age 1", 10, 20, nullptr);
		auto a2 = std::make_shared<AgeGroup>("age 2", 30, 40, nullptr);

		app.GetTournament()->AddAgeGroup(a1);
		app.GetTournament()->AddAgeGroup(a2);

		EXPECT_TRUE(app.Ajax_EditAgeGroup(HttpServer::Request("id=" + (std::string)a1->GetUUID(), "name=test3&min_age=6&max_age=10&gender=1&rule=" + (std::string)r->GetUUID())));

		EXPECT_EQ(a1->GetName(), "test3");
		EXPECT_EQ(a1->GetMinAge(), 6);
		EXPECT_EQ(a1->GetMaxAge(), 10);
		EXPECT_EQ(a1->GetGender(), Gender::Female);
		ASSERT_TRUE(a1->GetRuleSet());
		EXPECT_EQ(*a1->GetRuleSet(), *r);

		EXPECT_TRUE(app.Ajax_EditAgeGroup(HttpServer::Request("id=" + (std::string)a2->GetUUID(), "name=test4&min_age=16&max_age=20&gender=0&rule=0")));

		EXPECT_EQ(a2->GetName(), "test4");
		EXPECT_EQ(a2->GetMinAge(), 16);
		EXPECT_EQ(a2->GetMaxAge(), 20);
		EXPECT_EQ(a2->GetGender(), Gender::Male);
		EXPECT_FALSE(a2->GetRuleSet());
	}
}



TEST(Ajax, AgeGroup_Import)
{
	initialize();

	{
		Application app;

		Tournament* tourney = new Tournament;

		auto a1 = std::make_shared<AgeGroup>("age 1", 10, 20, nullptr);
		auto a2 = std::make_shared<AgeGroup>("age 2", 30, 40, nullptr);

		tourney->AddAgeGroup(a1);
		tourney->AddAgeGroup(a2);

		EXPECT_FALSE(app.Ajax_ImportAgeGroup(HttpServer::Request("id="+(std::string)a1->GetUUID())));
		EXPECT_FALSE(app.Ajax_ImportAgeGroup(HttpServer::Request("id="+(std::string)a2->GetUUID())));

		app.AddTournament(tourney);

		auto start_size = app.GetDatabase().GetAgeGroups().size();


		EXPECT_FALSE(app.GetDatabase().FindAgeGroup(*a1));
		EXPECT_FALSE(app.GetDatabase().FindAgeGroup(*a2));

		EXPECT_TRUE(app.Ajax_ImportAgeGroup(HttpServer::Request("id="+(std::string)a1->GetUUID())));

		EXPECT_EQ(app.GetDatabase().GetAgeGroups().size(), start_size + 1);
		EXPECT_TRUE(app.GetDatabase().FindAgeGroup(*a1));
		EXPECT_FALSE(app.GetDatabase().FindAgeGroup(*a2));

		EXPECT_TRUE(app.Ajax_ImportAgeGroup(HttpServer::Request("id="+(std::string)a2->GetUUID())));

		EXPECT_EQ(app.GetDatabase().GetAgeGroups().size(), start_size + 2);
		EXPECT_TRUE(app.GetDatabase().FindAgeGroup(*a1));
		EXPECT_TRUE(app.GetDatabase().FindAgeGroup(*a2));
	}
}



TEST(Ajax, AgeGroup_Delete)
{
	initialize();

	{
		Application app;

		auto r = app.GetDatabase().GetRuleSets()[0];
		app.Ajax_AddAgeGroup(HttpServer::Request("", "name=test&min_age=3&max_age=5&gender=0&rule=" + (std::string)r->GetUUID()));

		auto& age_groups = app.GetDatabase().GetAgeGroups();
		ASSERT_EQ(age_groups.size(), 7);
		auto a = age_groups[6];

		app.Ajax_DeleteAgeGroup(HttpServer::Request("id=" + (std::string)a->GetUUID()));

		ASSERT_EQ(age_groups.size(), 6);
	}
}



TEST(Ajax, AgeGroup_Get)
{
	initialize();

	{
		Application app;

		auto r  = std::make_shared<RuleSet>("test", 1, 2, 3, 4);
		auto a1 = std::make_shared<AgeGroup>("age 1", 10, 20, r);
		auto a2 = std::make_shared<AgeGroup>("age 2", 30, 40, nullptr);

		app.GetDatabase().AddAgeGroup(a1);
		app.GetTournament()->AddAgeGroup(a2);

		YAML::Node yaml = YAML::Load(app.Ajax_GetAgeGroup(HttpServer::Request("id=" + (std::string)a1->GetUUID())));

		EXPECT_EQ(yaml["uuid"].as<std::string>(), a1->GetUUID());
		EXPECT_EQ(yaml["name"].as<std::string>(), a1->GetName());
		EXPECT_EQ(yaml["min_age"].as<int>(), a1->GetMinAge());
		EXPECT_EQ(yaml["max_age"].as<int>(), a1->GetMaxAge());
		EXPECT_EQ(yaml["in_db"].as<bool>(), true);
		ASSERT_TRUE(yaml["rules_name"]);
		EXPECT_EQ(yaml["rules_name"].as<std::string>(), "test");
		EXPECT_TRUE(yaml["rules_uuid"]);
		EXPECT_EQ(yaml["rules_uuid"].as<std::string>(), *r);

		yaml = YAML::Load(app.Ajax_GetAgeGroup(HttpServer::Request("id=" + (std::string)a2->GetUUID())));

		EXPECT_EQ(yaml["uuid"].as<std::string>(), a2->GetUUID());
		EXPECT_EQ(yaml["name"].as<std::string>(), a2->GetName());
		EXPECT_EQ(yaml["min_age"].as<int>(), a2->GetMinAge());
		EXPECT_EQ(yaml["max_age"].as<int>(), a2->GetMaxAge());
		EXPECT_EQ(yaml["in_db"].as<bool>(), false);
		EXPECT_FALSE(yaml["rules_name"]);
		EXPECT_FALSE(yaml["rules_uuid"]);
	}
}



TEST(Ajax, AgeGroup_List)
{
	initialize();

	{
		Application app;
		auto r = std::make_shared<RuleSet>("rule set", 10, 20, 30, 40);

		auto a1 = std::make_shared<AgeGroup>("age 1", 10, 20, nullptr);
		auto a2 = std::make_shared<AgeGroup>("age 2", 30, 40, r);

		app.GetDatabase().AddAgeGroup(a1);
		app.GetTournament()->AddAgeGroup(a2);

		YAML::Node yaml = YAML::Load(app.Ajax_ListAllAgeGroups());

		ASSERT_EQ(yaml.size(), 8);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "U11");
		EXPECT_EQ(yaml[0]["is_used"].as<bool>(), false);
		EXPECT_EQ(yaml[0]["in_db"].as<bool>(), true);
		EXPECT_EQ(yaml[1]["name"].as<std::string>(), "U13");
		EXPECT_EQ(yaml[1]["is_used"].as<bool>(), false);
		EXPECT_EQ(yaml[1]["in_db"].as<bool>(), true);
		EXPECT_EQ(yaml[2]["name"].as<std::string>(), "U15");
		EXPECT_EQ(yaml[2]["is_used"].as<bool>(), false);
		EXPECT_EQ(yaml[2]["in_db"].as<bool>(), true);
		EXPECT_EQ(yaml[3]["name"].as<std::string>(), "U18");
		EXPECT_EQ(yaml[3]["is_used"].as<bool>(), false);
		EXPECT_EQ(yaml[3]["in_db"].as<bool>(), true);
		EXPECT_EQ(yaml[4]["name"].as<std::string>(), "U21");
		EXPECT_EQ(yaml[4]["is_used"].as<bool>(), false);
		EXPECT_EQ(yaml[4]["in_db"].as<bool>(), true);
		EXPECT_EQ(yaml[5]["name"].as<std::string>(), "Seniors");
		EXPECT_EQ(yaml[5]["is_used"].as<bool>(), false);
		EXPECT_EQ(yaml[5]["in_db"].as<bool>(), true);
		EXPECT_EQ(yaml[6]["name"].as<std::string>(), "age 1");
		EXPECT_EQ(yaml[6]["is_used"].as<bool>(), false);
		EXPECT_EQ(yaml[6]["in_db"].as<bool>(), true);
		EXPECT_EQ(yaml[7]["name"].as<std::string>(), "age 2");
		EXPECT_EQ(yaml[7]["is_used"].as<bool>(), true);
		EXPECT_EQ(yaml[7]["in_db"].as<bool>(), false);
		EXPECT_EQ(yaml[7]["num_match_tables"].as<int>(), 0);
		EXPECT_EQ(yaml[7]["num_matches"].as<int>(), 0);
		EXPECT_EQ(yaml[7]["num_participants"].as<int>(), 0);
		EXPECT_EQ(yaml[7]["rules_name"].as<std::string>(), "rule set");
		EXPECT_EQ(yaml[7]["rules_uuid"].as<std::string>(), r->GetUUID());
	}
}



TEST(Ajax, AgeGroups_List2)
{
	initialize();

	Application app;

	YAML::Node result = YAML::Load(app.Ajax_ListAllAgeGroups());

	ASSERT_TRUE(result);
	ASSERT_TRUE(result.IsSequence());
	ASSERT_EQ(result.size(), 6);

	EXPECT_EQ(result[0]["name"].as<std::string>(), "U11");
	EXPECT_EQ(result[1]["name"].as<std::string>(), "U13");
	EXPECT_EQ(result[2]["name"].as<std::string>(), "U15");
	EXPECT_EQ(result[3]["name"].as<std::string>(), "U18");
	EXPECT_EQ(result[4]["name"].as<std::string>(), "U21");
	EXPECT_EQ(result[5]["name"].as<std::string>(), "Seniors");
}



TEST(Ajax, RuleSet_Add)
{
	initialize();

	{
		Application app;
		app.GetDatabase().EnableAutoSave(false);

		auto& rule_sets = app.GetDatabase().GetRuleSets();
		auto count = rule_sets.size();

		EXPECT_TRUE(app.Ajax_AddRuleSet(HttpServer::Request("", "name=test&match_time=60&goldenscore_time=120&osaekomi_ippon=5&osaekomi_wazaari=3&yuko=true&koka=true&draw=true&break_time=30&extend_break_time=true")));

		ASSERT_EQ(rule_sets.size(), count + 1);
		EXPECT_EQ(rule_sets[count]->GetName(), "test");
		EXPECT_EQ(rule_sets[count]->GetMatchTime(), 60);
		EXPECT_EQ(rule_sets[count]->GetGoldenScoreTime(), 120);
		EXPECT_EQ(rule_sets[count]->GetOsaeKomiTime(false), 5);
		EXPECT_EQ(rule_sets[count]->GetOsaeKomiTime(true),  3);
		EXPECT_EQ(rule_sets[count]->IsYukoEnabled(), true);
		EXPECT_EQ(rule_sets[count]->IsKokaEnabled(), true);
		EXPECT_EQ(rule_sets[count]->IsDrawAllowed(), true);
		EXPECT_EQ(rule_sets[count]->GetBreakTime(),  30);
		EXPECT_EQ(rule_sets[count]->IsExtendBreakTime(), true);
	}
}



TEST(Ajax, RuleSet_Edit)
{
	initialize();

	{
		Application app;
		app.GetDatabase().EnableAutoSave(false);

		auto& rule_sets = app.GetDatabase().GetRuleSets();
		auto count = rule_sets.size();

		EXPECT_TRUE(app.Ajax_AddRuleSet(HttpServer::Request("", "name=test&match_time=60&goldenscore_time=120&osaekomi_ippon=5&osaekomi_wazaari=3&yuko=true&koka=true&draw=true&break_time=30&extend_break_time=true")));

		ASSERT_EQ(rule_sets.size(), count + 1);
		
		EXPECT_TRUE(app.Ajax_EditRuleSet(HttpServer::Request("id=" + (std::string)rule_sets[count]->GetUUID(), "name=test2&match_time=120&goldenscore_time=180&osaekomi_ippon=20&osaekomi_wazaari=13&yuko=false&koka=false&draw=false&break_time=60&extend_break_time=false")));

		EXPECT_EQ(rule_sets[count]->GetName(), "test2");
		EXPECT_EQ(rule_sets[count]->GetMatchTime(), 120);
		EXPECT_EQ(rule_sets[count]->GetGoldenScoreTime(), 180);
		EXPECT_EQ(rule_sets[count]->GetOsaeKomiTime(false), 20);
		EXPECT_EQ(rule_sets[count]->GetOsaeKomiTime(true),  13);
		EXPECT_EQ(rule_sets[count]->IsYukoEnabled(), false);
		EXPECT_EQ(rule_sets[count]->IsKokaEnabled(), false);
		EXPECT_EQ(rule_sets[count]->IsDrawAllowed(), false);
		EXPECT_EQ(rule_sets[count]->GetBreakTime(),  60);
		EXPECT_EQ(rule_sets[count]->IsExtendBreakTime(), false);
	}
}



TEST(Ajax, RuleSet_Get)
{
	initialize();

	{
		Application app;
		app.GetDatabase().EnableAutoSave(false);

		auto& rule_sets = app.GetDatabase().GetRuleSets();
		auto count = rule_sets.size();

		EXPECT_TRUE(app.Ajax_AddRuleSet(HttpServer::Request("", "name=test&match_time=60&goldenscore_time=120&osaekomi_ippon=5&osaekomi_wazaari=3&yuko=true&koka=true&draw=true&break_time=30&extend_break_time=true")));

		ASSERT_EQ(rule_sets.size(), count + 1);

		auto yaml = YAML::Load( app.Ajax_GetRuleSet(HttpServer::Request("id=" + (std::string)rule_sets[count]->GetUUID())) );

		EXPECT_EQ(rule_sets[count]->GetName(),      yaml["name"].as<std::string>());
		EXPECT_EQ(rule_sets[count]->GetMatchTime(), yaml["match_time"].as<int>());
		EXPECT_EQ(rule_sets[count]->GetGoldenScoreTime(),   yaml["golden_score_time"].as<int>());
		EXPECT_EQ(rule_sets[count]->GetOsaeKomiTime(false), yaml["osaekomi_time"].as<int>());
		EXPECT_EQ(rule_sets[count]->GetOsaeKomiTime(true),  yaml["osaekomi_with_wazaari_time"].as<int>());
		EXPECT_EQ(rule_sets[count]->IsYukoEnabled(), yaml["yuko"].as<bool>());
		EXPECT_EQ(rule_sets[count]->IsKokaEnabled(), yaml["koka"].as<bool>());
		EXPECT_EQ(rule_sets[count]->IsDrawAllowed(), yaml["draw"].as<bool>());
		EXPECT_EQ(rule_sets[count]->GetBreakTime(),  yaml["break_time"].as<int>());
		EXPECT_EQ(rule_sets[count]->IsExtendBreakTime(), yaml["extend_break_time"].as<bool>());
	}
}



TEST(Ajax, RuleSet_List)
{
	initialize();

	{
		Application app;
		app.GetDatabase().EnableAutoSave(false);

		auto& rule_sets = app.GetDatabase().GetRuleSets();
		app.GetTournament()->SetDefaultRuleSet(rule_sets[0]);

		auto yaml = YAML::Load( app.Ajax_ListRuleSets() );

		ASSERT_TRUE(yaml.IsMap());
		EXPECT_EQ((std::string)app.GetTournament()->GetDefaultRuleSet()->GetUUID(), yaml["default"].as<std::string>());

		for (int i = 0; i < rule_sets.size(); i++)
		{
			EXPECT_EQ((std::string)rule_sets[i]->GetUUID(), yaml["rules"][i]["uuid"].as<std::string>());
			EXPECT_EQ(rule_sets[i]->GetName(),              yaml["rules"][i]["name"].as<std::string>());
			EXPECT_EQ(rule_sets[i]->GetDescription(),       yaml["rules"][i]["desc"].as<std::string>());
		}
	}
}



TEST(Ajax, GetMats)
{
	initialize();

	Application app;

	auto yaml = YAML::Load(app.Ajax_GetMats());

	EXPECT_EQ(yaml["highest_mat_id"].as<int>(), 1);

	app.StartLocalMat(1);

	yaml = YAML::Load(app.Ajax_GetMats());

	EXPECT_EQ(yaml["highest_mat_id"].as<int>(), 2);
	EXPECT_TRUE(yaml["monitors"].IsSequence());
	EXPECT_TRUE(yaml["audio_devices"].IsSequence());
	EXPECT_EQ(yaml["mats"][0]["type"].as<int>(), (int)Mat::Type::LocalMat);
	EXPECT_EQ(yaml["mats"][0]["name"].as<std::string>(), "Mat 1");
	EXPECT_EQ(yaml["mats"][0]["is_paused"].as<bool>(), false);
	EXPECT_EQ(yaml["mats"][0]["ippon_style"].as<int>(), (int)IMat::IpponStyle::DoubleDigit);
	EXPECT_EQ(yaml["mats"][0]["osaekomi_style"].as<int>(), (int)IMat::OsaekomiStyle::ProgressBar);
	EXPECT_EQ(yaml["mats"][0]["name_style"].as<int>(),  (int)NameStyle::FamilyName);
	EXPECT_TRUE(yaml["mats"][0]["sound_enabled"].as<bool>());
	EXPECT_EQ(yaml["mats"][0]["sound_filename"].as<std::string>(), "test");
}



TEST(Ajax, OpenMat)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);
		app.GetLocalMat()->Close();
		EXPECT_FALSE(app.GetLocalMat()->IsOpen());

		app.Ajax_OpenMat(HttpServer::Request("id=1"));

		ASSERT_TRUE(app.GetLocalMat());
		EXPECT_EQ(app.GetLocalMat()->GetMatID(), 1);
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
	}


	{
		Application app;

		app.StartLocalMat(5);
		app.GetLocalMat()->Close();

		app.Ajax_OpenMat(HttpServer::Request("id=1"));

		EXPECT_FALSE(app.GetLocalMat()->IsOpen());

		app.Ajax_OpenMat(HttpServer::Request("id=5"));

		ASSERT_TRUE(app.GetLocalMat());
		EXPECT_EQ(app.GetLocalMat()->GetMatID(), 5);
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
	}
}



TEST(Ajax, OpenNewMat)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);

		EXPECT_TRUE(app.FindMat(1));
		EXPECT_FALSE(app.FindMat(2));

		EXPECT_TRUE(app.Ajax_OpenMat(HttpServer::Request("id=0")));

		EXPECT_TRUE(app.FindMat(1));
		EXPECT_TRUE(app.FindMat(2));
	}
}



TEST(Ajax, CloseMat)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);

		app.Ajax_OpenMat( HttpServer::Request("id=1"));
		app.Ajax_CloseMat(HttpServer::Request("id=5"));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());

		app.Ajax_CloseMat(HttpServer::Request("id=1"));

		ASSERT_TRUE(app.GetLocalMat());
		EXPECT_FALSE(app.GetLocalMat()->IsOpen());
	}


	{
		Application app;

		app.StartLocalMat(5);

		app.Ajax_OpenMat( HttpServer::Request("id=5"));
		app.Ajax_CloseMat(HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());

		app.Ajax_CloseMat(HttpServer::Request("id=5"));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_FALSE(app.GetLocalMat()->IsOpen());
	}
}



TEST(Ajax, UpdateMat)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());

		EXPECT_TRUE(app.Ajax_UpdateMat(HttpServer::Request("id=1", "id=5&name=Test&ipponStyle=0&osaekomiStyle=0&timerStyle=1&nameStyle=0&sound=false&sound_filename=changed&sound_device=7")));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
		EXPECT_EQ(app.GetLocalMat()->GetMatID(), 5);
		EXPECT_EQ(app.GetLocalMat()->GetName(), "Test");
		EXPECT_EQ((int)app.GetLocalMat()->GetIpponStyle(), 0);
		EXPECT_EQ((int)app.GetLocalMat()->GetOsaekomiStyle(), 0);
		EXPECT_EQ((int)app.GetLocalMat()->GetTimerStyle(), 1);
		EXPECT_EQ((int)app.GetLocalMat()->GetNameStyle(),  0);
		EXPECT_FALSE(app.GetLocalMat()->IsSoundEnabled());
		EXPECT_EQ(app.GetLocalMat()->GetSoundFilename(), "changed");
		EXPECT_EQ(app.GetLocalMat()->GetAudioDeviceID(), 7);


		EXPECT_TRUE(app.Ajax_UpdateMat(HttpServer::Request("id=5", "id=1&name=Test2&ipponStyle=1&osaekomiStyle=1&timerStyle=2&nameStyle=1&sound=true&sound_filename=changed2&sound_device=-1")));


		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
		EXPECT_EQ(app.GetLocalMat()->GetMatID(), 1);
		EXPECT_EQ(app.GetLocalMat()->GetName(), "Test2");
		EXPECT_EQ((int)app.GetLocalMat()->GetIpponStyle(), 1);
		EXPECT_EQ((int)app.GetLocalMat()->GetOsaekomiStyle(), 1);
		EXPECT_EQ((int)app.GetLocalMat()->GetTimerStyle(), 2);
		EXPECT_EQ((int)app.GetLocalMat()->GetNameStyle(),  1);
		EXPECT_TRUE(app.GetLocalMat()->IsSoundEnabled());
		EXPECT_EQ(app.GetLocalMat()->GetSoundFilename(), "changed2");
		EXPECT_EQ(app.GetLocalMat()->GetAudioDeviceID(), -1);


		EXPECT_TRUE(app.Ajax_UpdateMat(HttpServer::Request("id=1", "id=1&name=Test3&ipponStyle=2&osaekomiStyle=0&timerStyle=0&nameStyle=0&sound=false&sound_filename=changed3&sound_device=42")));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
		EXPECT_EQ(app.GetLocalMat()->GetMatID(), 1);
		EXPECT_EQ(app.GetLocalMat()->GetName(), "Test3");
		EXPECT_EQ((int)app.GetLocalMat()->GetIpponStyle(), 2);
		EXPECT_EQ((int)app.GetLocalMat()->GetOsaekomiStyle(), 0);
		EXPECT_EQ((int)app.GetLocalMat()->GetTimerStyle(), 0);
		EXPECT_EQ((int)app.GetLocalMat()->GetNameStyle(),  0);
		EXPECT_FALSE(app.GetLocalMat()->IsSoundEnabled());
		EXPECT_EQ(app.GetLocalMat()->GetSoundFilename(), "changed3");
		EXPECT_EQ(app.GetLocalMat()->GetAudioDeviceID(), 42);
	}
}



TEST(Ajax, PauseMat)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());

		EXPECT_TRUE(app.Ajax_PauseMat(HttpServer::Request("id=1&enable=true")));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
		EXPECT_TRUE(app.GetLocalMat()->IsPaused());

		EXPECT_FALSE(app.Ajax_PauseMat(HttpServer::Request("id=5&enable=false")));
		EXPECT_TRUE( app.Ajax_PauseMat(HttpServer::Request("id=1&enable=false")));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
		EXPECT_EQ(app.GetLocalMat()->GetMatID(), 1);
		EXPECT_FALSE(app.GetLocalMat()->IsPaused());

		EXPECT_FALSE(app.Ajax_PauseMat(HttpServer::Request("id=5&enable=true")));
		EXPECT_TRUE( app.Ajax_PauseMat(HttpServer::Request("id=1&enable=true")));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
		EXPECT_TRUE(app.GetLocalMat()->IsPaused());
	}
}



TEST(Ajax, PauseAllMats)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);
		app.StartLocalMat(2);
		app.StartLocalMat(3);

		auto mat1 = app.FindMat(1);
		auto mat2 = app.FindMat(2);
		auto mat3 = app.FindMat(3);

		EXPECT_FALSE(app.Ajax_PauseAllMats(HttpServer::Request("enable=false")));

		EXPECT_TRUE(app.Ajax_PauseAllMats(HttpServer::Request("enable=true")));

		EXPECT_TRUE(mat1->IsPaused());
		EXPECT_TRUE(mat2->IsPaused());
		EXPECT_TRUE(mat3->IsPaused());

		EXPECT_FALSE(app.Ajax_PauseAllMats(HttpServer::Request("enable=true")));

		EXPECT_TRUE(mat1->IsPaused());
		EXPECT_TRUE(mat2->IsPaused());
		EXPECT_TRUE(mat3->IsPaused());

		EXPECT_TRUE(app.Ajax_PauseAllMats(HttpServer::Request("enable=false")));

		EXPECT_FALSE(mat1->IsPaused());
		EXPECT_FALSE(mat2->IsPaused());
		EXPECT_FALSE(mat3->IsPaused());		
	}
}



TEST(Ajax, UpdatePassword)
{
	initialize();

	{
		Application app;

		Account* acc = new Account("test", "pwd1");
		app.GetDatabase().AddAccount(acc);

		app.Ajax_UpdatePassword(nullptr, HttpServer::Request("", "password=pwd2"));

		EXPECT_EQ(acc->GetPassword(), "pwd1");
		EXPECT_EQ(acc->GetAccessLevel(), Account::AccessLevel::User);

		app.Ajax_UpdatePassword(acc, HttpServer::Request("", "password=pwd2"));

		EXPECT_EQ(acc->GetPassword(), "pwd2");
		EXPECT_EQ(acc->GetAccessLevel(), Account::AccessLevel::User);

		app.Ajax_UpdatePassword(acc, HttpServer::Request("", "password=pwd3"));

		EXPECT_EQ(acc->GetPassword(), "pwd3");
		EXPECT_EQ(acc->GetAccessLevel(), Account::AccessLevel::User);
	}
}



TEST(Ajax, Setup_Get)
{
	initialize();

	{
		Application app;

		auto yaml = YAML::Load(app.Ajax_GetSetup(false));

		EXPECT_EQ(yaml["language"].as<int>(), (int)Localizer::GetLanguage());
		EXPECT_EQ(yaml["mat_count"].as<int>(), 1);
		EXPECT_EQ(yaml["port"].as<int>(), app.GetDatabase().GetServerPort());
		EXPECT_LE(yaml["uptime"].as<uint32_t>(), 100u);
		EXPECT_EQ(yaml["version"].as<std::string>(), Application::Version);
		EXPECT_TRUE(yaml["results_server"]);
		EXPECT_TRUE(yaml["results_server_url"]);

		ZED::Core::Pause(1000);

		yaml = YAML::Load(app.Ajax_GetSetup(true));

		EXPECT_EQ(yaml["language"].as<int>(), (int)Localizer::GetLanguage());
		EXPECT_EQ(yaml["mat_count"].as<int>(), 1);
		EXPECT_EQ(yaml["port"].as<int>(), app.GetDatabase().GetServerPort());
		EXPECT_LE(yaml["uptime"].as<uint32_t>(), 1100u);
		EXPECT_EQ(yaml["version"].as<std::string>(), Application::Version);
		EXPECT_TRUE(yaml["results_server"]);
		EXPECT_TRUE(yaml["results_server_url"]);
	}

}



TEST(Ajax, SwapMatches)
{
	initialize();

	{
		Application app;

		Tournament* t = new Tournament("../test-data/randori-2022");

		ASSERT_EQ(t->GetMatchTables().size(), 3);

		app.AddTournament(t);

		EXPECT_TRUE(app.Ajax_SwapMatchesOfTournament(HttpServer::Request("id=" + (std::string)t->GetUUID())));

		auto results = t->GetMatchTables()[0]->CalculateResults();
		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Joris");
		EXPECT_EQ(results[0].Wins,   2);
		EXPECT_EQ(results[0].Score, 17);

		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Richard");
		EXPECT_EQ(results[1].Wins,  1);
		EXPECT_EQ(results[1].Score, 7);

		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Theo");
		EXPECT_EQ(results[2].Wins,  0);
		EXPECT_EQ(results[2].Score, 0);

		results = t->GetMatchTables()[1]->CalculateResults();
		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Tom");
		EXPECT_EQ(results[0].Wins,   2);
		EXPECT_EQ(results[0].Score, 20);

		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Julius");
		EXPECT_EQ(results[1].Wins,  0);
		EXPECT_EQ(results[1].Score, 0);

		results = t->GetMatchTables()[2]->CalculateResults();
		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leni");
		EXPECT_EQ(results[0].Wins,  2);
		EXPECT_EQ(results[0].Score, 8);

		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Clara");
		EXPECT_EQ(results[1].Wins,   1);
		EXPECT_EQ(results[1].Score, 10);

		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maja");
		EXPECT_EQ(results[2].Wins,  0);
		EXPECT_EQ(results[2].Score, 0);
	}

	ZED::Core::RemoveFile("tournaments/randori-2022.yml");
}



TEST(Ajax, Sound_ListFiles)
{
	initialize();

	{
		Application app;

		auto yaml = YAML::Load(app.Ajax_ListSoundFiles());

		ASSERT_TRUE(yaml.IsSequence());

		std::vector<std::string> filenames = { "airhorn", "alert", "gong sabi", "gong", "gong2", "high gong", "low gong", "spooky gong", "tiger gong", "wind chime" };

		int i = 0;
		for (auto node : yaml)
		{
			EXPECT_EQ(node["filename"].as<std::string>(), filenames[i++]);
		}
	}
}



TEST(Ajax, Setup_Set)
{
	initialize();

	{
		Application app;
		
		EXPECT_EQ(app.Ajax_SetSetup(HttpServer::Request("", "port=1234&language=0&mat_count=5&ipponStyle=1&osaekomiStyle=1&timerStyle=2&nameStyle=0&results_server=true&results_server_url=abc")), "ok");

		auto yaml = YAML::Load(app.Ajax_GetSetup(false));

		EXPECT_EQ(yaml["language"].as<int>(), 0);
		EXPECT_EQ(yaml["mat_count"].as<int>(), 5);
		EXPECT_EQ(yaml["port"].as<int>(), 1234);
		EXPECT_EQ(yaml["ippon_style"].as<int>(), 1);
		EXPECT_EQ(yaml["osaekomi_style"].as<int>(), 1);
		EXPECT_EQ(yaml["timer_style"].as<int>(), 2);
		EXPECT_EQ(yaml["name_style"].as<int>(),  0);
		EXPECT_LE(yaml["uptime"].as<uint32_t>(), 100u);
		EXPECT_EQ(yaml["version"].as<std::string>(), Application::Version);
		EXPECT_EQ(yaml["results_server"].as<bool>(), true);
		EXPECT_EQ(yaml["results_server_url"].as<std::string>(), "abc");

		EXPECT_EQ(app.Ajax_SetSetup(HttpServer::Request("", "port=567&language=1&mat_count=8&ipponStyle=0&osaekomiStyle=0&timerStyle=1&nameStyle=1&results_server=false&results_server_url=def")), "ok");

		yaml = YAML::Load(app.Ajax_GetSetup(true));

		EXPECT_EQ(yaml["language"].as<int>(), 1);
		EXPECT_EQ(yaml["mat_count"].as<int>(), 8);
		EXPECT_EQ(yaml["port"].as<int>(), 567);
		EXPECT_EQ(yaml["ippon_style"].as<int>(), 0);
		EXPECT_EQ(yaml["osaekomi_style"].as<int>(), 0);
		EXPECT_EQ(yaml["timer_style"].as<int>(), 1);
		EXPECT_EQ(yaml["name_style"].as<int>(),  1);
		EXPECT_LE(yaml["uptime"].as<uint32_t>(), 100u);
		EXPECT_EQ(yaml["version"].as<std::string>(), Application::Version);
		EXPECT_EQ(yaml["results_server"].as<bool>(), false);
		EXPECT_EQ(yaml["results_server_url"].as<std::string>(), "def");
		EXPECT_EQ(yaml["license_valid"].as<bool>(), false);
		EXPECT_EQ(yaml["license_expired"].as<bool>(), false);
		EXPECT_TRUE(yaml["license_expiration_date"]);
		EXPECT_TRUE(yaml["license_expiration_type"]);
		EXPECT_TRUE(yaml["license_expiration_id"]);
	}
}



TEST(Ajax, ExecuteCommand)
{
	initialize();

	{
		Application app;

		std::string result = app.Ajax_Execute(HttpServer::Request("cmd=dir"));

		EXPECT_GE(result.length(), 120);

		//TODO?!
	}
}



/*TEST(Ajax, SetFullscreen)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
		Mat* mat = (Mat*)app.GetLocalMat();

		app.Ajax_SetFullscreen(true, HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsFullscreen());
		EXPECT_EQ(app.GetLocalMat()->GetMatID(), 1);


		app.Ajax_SetFullscreen(false, HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
		EXPECT_FALSE(app.GetLocalMat()->IsFullscreen());
		EXPECT_EQ(app.GetLocalMat()->GetMatID(), 1);


		app.Ajax_SetFullscreen(true, HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetLocalMat());
		EXPECT_TRUE(app.GetLocalMat()->IsOpen());
		EXPECT_TRUE(app.GetLocalMat()->IsFullscreen());
		EXPECT_EQ(app.GetLocalMat()->GetMatID(), 1);
	}
}*/



TEST(Ajax, GetHansokumake)
{
	initialize();
	
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		auto match = std::make_shared<Match>(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		EXPECT_TRUE(mat->StartMatch(match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);
		
		YAML::Node ret2 = YAML::Load(app.Ajax_GetHansokumake());

		ASSERT_TRUE(ret2[0]["match"].IsDefined());
		ASSERT_TRUE(ret2[0]["match"].IsMap());
		EXPECT_EQ(ret2[0]["match"]["uuid"].as<std::string>(), (std::string)match->GetUUID());
		EXPECT_EQ(ret2[0]["match"]["white_name"].as<std::string>(), match->GetFighter(Fighter::White)->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["match"]["blue_name"].as<std::string>(),  match->GetFighter(Fighter::Blue )->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["fighter"].as<int>(), (int)f);
		EXPECT_EQ(ret2[0]["disqualification_state"].as<int>(), (int)IMat::Scoreboard::DisqualificationState::Unknown);
	}


	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		auto match = std::make_shared<Match>(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		EXPECT_TRUE(mat->StartMatch(match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);
		mat->AddDisqualification(f);

		YAML::Node ret2 = YAML::Load(app.Ajax_GetHansokumake());

		ASSERT_TRUE(ret2[0]["match"].IsDefined());
		ASSERT_TRUE(ret2[0]["match"].IsMap());
		EXPECT_EQ(ret2[0]["match"]["uuid"].as<std::string>(), (std::string)match->GetUUID());
		EXPECT_EQ(ret2[0]["match"]["white_name"].as<std::string>(), match->GetFighter(Fighter::White)->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["match"]["blue_name"].as<std::string>(),  match->GetFighter(Fighter::Blue )->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["fighter"].as<int>(), (int)f);
		EXPECT_EQ(ret2[0]["disqualification_state"].as<int>(), (int)IMat::Scoreboard::DisqualificationState::Disqualified);
	}


	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		auto match = std::make_shared<Match>(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		EXPECT_TRUE(mat->StartMatch(match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);
		mat->AddNoDisqualification(f);

		YAML::Node ret2 = YAML::Load(app.Ajax_GetHansokumake());

		ASSERT_TRUE(ret2[0]["match"].IsDefined());
		ASSERT_TRUE(ret2[0]["match"].IsMap());
		EXPECT_EQ(ret2[0]["match"]["uuid"].as<std::string>(), (std::string)match->GetUUID());
		EXPECT_EQ(ret2[0]["match"]["white_name"].as<std::string>(), match->GetFighter(Fighter::White)->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["match"]["blue_name"].as<std::string>(),  match->GetFighter(Fighter::Blue )->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["fighter"].as<int>(), (int)f);
		EXPECT_EQ(ret2[0]["disqualification_state"].as<int>(), (int)IMat::Scoreboard::DisqualificationState::NotDisqualified);
	}
}



TEST(Ajax, GetHansokumake2)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		auto match = std::make_shared<Match>(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		EXPECT_TRUE(mat->StartMatch(match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		for (int i = 0;i < 5; i++)
			mat->AddShido(f);

		auto ret2 = app.Ajax_GetHansokumake();

		EXPECT_EQ(ret, "[]");
	}
}



TEST(Ajax, Judoka_Add)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddJudoka(HttpServer::Request("", "firstname=first&lastname=last&weight=10&gender=0&birthyear=2000&number=A123")), "ok");

		auto judokas = app.GetDatabase().GetAllJudokas();

		ASSERT_EQ(judokas.size(), 1);
		auto judoka = judokas[0];

		EXPECT_EQ(judoka->GetFirstname(), "first");
		EXPECT_EQ(judoka->GetLastname(),  "last");
		EXPECT_EQ(judoka->GetWeight(),  Weight(10));
		EXPECT_EQ(judoka->GetGender(),  Gender::Male);
		EXPECT_EQ(judoka->GetBirthyear(), 2000);
		EXPECT_EQ(judoka->GetNumber(), "A123");
	}
}



TEST(Ajax, Judoka_Get)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddJudoka(HttpServer::Request("", "firstname=first&lastname=last&weight=10,2&gender=0&birthyear=2000&number=A123")), "ok");

		auto judokas = app.GetDatabase().GetAllJudokas();

		ASSERT_EQ(judokas.size(), 1);
		auto judoka = judokas[0];

		auto yaml = app.Ajax_GetJudoka(HttpServer::Request("id="+(std::string)judoka->GetUUID()));

		Judoka judoka2(YAML::Load(yaml), nullptr);

		EXPECT_EQ(judoka->GetFirstname(), judoka2.GetFirstname());
		EXPECT_EQ(judoka->GetLastname(),  judoka2.GetLastname());
		EXPECT_EQ(judoka->GetWeight(),    judoka2.GetWeight());
		EXPECT_EQ(judoka->GetGender(),    judoka2.GetGender());
		EXPECT_EQ(judoka->GetBirthyear(), judoka2.GetBirthyear());
		EXPECT_EQ(judoka->GetNumber(),    judoka2.GetNumber());



		auto t = new Tournament("deleteMe");
		t->EnableAutoSave(false);

		app.AddTournament(t);

		auto j1 = new Judoka("firstname", "lastname");
		t->AddParticipant(j1);


		yaml = app.Ajax_GetJudoka(HttpServer::Request("id="+(std::string)j1->GetUUID()));

		auto node = YAML::Load(yaml);
		Judoka judoka3(node, nullptr);

		ASSERT_TRUE(node["is_participant"]);
		EXPECT_TRUE(node["is_participant"].as<bool>());

		EXPECT_EQ(j1->GetFirstname(), judoka3.GetFirstname());
		EXPECT_EQ(j1->GetLastname(),  judoka3.GetLastname());
		EXPECT_EQ(j1->GetWeight(),    judoka3.GetWeight());
		EXPECT_EQ(j1->GetGender(),    judoka3.GetGender());
		EXPECT_EQ(j1->GetBirthyear(), judoka3.GetBirthyear());
		EXPECT_EQ(j1->GetNumber(),    judoka3.GetNumber());
	}
}



TEST(Ajax, Judoka_Assign_AgeGroup)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Application app;

		auto t = new Tournament("deleteMe");
		t->EnableAutoSave(false);

		app.AddTournament(t);

		auto a1 = std::make_shared<AgeGroup>("a1", 0, 0, nullptr);
		auto a2 = std::make_shared<AgeGroup>("a2", 0, 0, nullptr);

		auto j1 = new Judoka("firstname", "lastname");
		t->AddParticipant(j1);

		auto j2 = new Judoka("firstname", "lastname");
		auto c1 = std::make_shared<Club>("Club 1");
		j2->SetClub(c1);
		t->AddParticipant(j2);

		EXPECT_FALSE(app.Ajax_AssignAgeGroup(HttpServer::Request("id="+(std::string)j1->GetUUID() + "&age=" + (std::string)a1->GetUUID())));

		t->AddAgeGroup(a1);
		t->AddAgeGroup(a2);

		EXPECT_TRUE(app.Ajax_AssignAgeGroup(HttpServer::Request("id="+(std::string)j1->GetUUID() + "&age=" + (std::string)a1->GetUUID())));

		EXPECT_EQ(*t->GetAgeGroupOfJudoka(j1), *a1);

		EXPECT_TRUE(app.Ajax_AssignAgeGroup(HttpServer::Request("id="+(std::string)j2->GetUUID() + "&age=" + (std::string)a2->GetUUID())));

		EXPECT_EQ(*t->GetAgeGroupOfJudoka(j1), *a1);
		EXPECT_EQ(*t->GetAgeGroupOfJudoka(j2), *a2);

		EXPECT_TRUE(app.Ajax_AssignAgeGroup(HttpServer::Request("id="+(std::string)j1->GetUUID() + "&age=" + (std::string)a2->GetUUID())));

		EXPECT_EQ(*t->GetAgeGroupOfJudoka(j1), *a2);
		EXPECT_EQ(*t->GetAgeGroupOfJudoka(j2), *a2);
	}
}



TEST(Ajax, Judoka_Search)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Application app;
		app.GetDatabase().EnableAutoSave(false);

		auto a1 = std::make_shared<AgeGroup>("u100", 0, 0, nullptr);

		auto c1 = std::make_shared<Club>("c1");
		auto c2 = std::make_shared<Club>("c2");
		auto c3 = std::make_shared<Club>("c3");

		auto j1 = new Judoka("ab", "def");
		j1->SetClub(c1);
		auto j2 = new Judoka("hg", "ij");
		j2->SetClub(c2);
		auto j3 = new Judoka("kl", "mn");
		j3->SetClub(c3);
		auto j4 = new Judoka("op", "qr");
		j4->SetClub(c1);
		auto j5 = new Judoka("st", "uv");
		j5->SetClub(c2);

		app.GetDatabase().AddJudoka(j1);
		app.GetDatabase().AddJudoka(j2);

		app.GetTournament()->AddAgeGroup(a1);
		app.GetTournament()->AddParticipant(j3);
		app.GetTournament()->AddParticipant(j4);
		app.GetTournament()->AddParticipant(j5);

		auto yaml = YAML::Load( app.Ajax_SearchJudoka(HttpServer::Request("name=ab")) );
		ASSERT_TRUE(yaml.IsSequence());
		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)j1->GetUUID());

		yaml = YAML::Load( app.Ajax_SearchJudoka(HttpServer::Request("name=st")) );
		ASSERT_EQ(yaml.size(), 0);

		yaml = YAML::Load( app.Ajax_SearchJudoka(HttpServer::Request("name=c")) );
		ASSERT_EQ(yaml.size(), 2);
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)j1->GetUUID());
		EXPECT_EQ(yaml[1]["uuid"].as<std::string>(), (std::string)j2->GetUUID());
		EXPECT_EQ(yaml[1]["gender"].as<int>(), (int)j2->GetGender());

		yaml = YAML::Load( app.Ajax_SearchJudoka(HttpServer::Request("name=kl&participants=true")) );
		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)j3->GetUUID());

		yaml = YAML::Load( app.Ajax_SearchJudoka(HttpServer::Request("name=c3&participants=true")) );
		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)j3->GetUUID());

		yaml = YAML::Load( app.Ajax_SearchJudoka(HttpServer::Request("name=u100&participants=true")) );
		ASSERT_EQ(yaml.size(), 3);
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)j3->GetUUID());
		EXPECT_EQ(yaml[1]["uuid"].as<std::string>(), (std::string)j4->GetUUID());
		EXPECT_EQ(yaml[2]["uuid"].as<std::string>(), (std::string)j5->GetUUID());
		EXPECT_EQ(yaml[2]["is_weighted"].as<bool>(), false);
	}
}



TEST(Ajax, Judoka_Edit)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddJudoka(HttpServer::Request("", "firstname=first&lastname=last&weight=10&gender=0&birthyear=2000&number=A123")), "ok");

		auto judokas = app.GetDatabase().GetAllJudokas();

		ASSERT_EQ(judokas.size(), 1);
		auto judoka = judokas[0];

		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id="+(std::string)judoka->GetUUID(), "firstname=first2&lastname=last2&weight=12,5&gender=1&birthyear=2001&number=A1234")));

		EXPECT_EQ(judoka->GetFirstname(), "first2");
		EXPECT_EQ(judoka->GetLastname(),  "last2");
		EXPECT_EQ(judoka->GetWeight(),  Weight("12,5"));
		EXPECT_EQ(judoka->GetGender(),  Gender::Female);
		EXPECT_EQ(judoka->GetBirthyear(), 2001);
		EXPECT_EQ(judoka->GetNumber(), "A1234");
	}
}



TEST(Ajax, Judoka_Edit_Participant)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Application app;

		auto t = new Tournament("deleteMe");
		t->EnableAutoSave(false);

		app.AddTournament(t);

		auto j1 = new Judoka("firstname", "lastname");
		t->AddParticipant(j1);

		auto j2 = new Judoka("firstname", "lastname");
		auto c1 = std::make_shared<Club>("Club 1");
		j2->SetClub(c1);
		t->AddParticipant(j2);

		EXPECT_FALSE(t->IsMarkedAsWeighted(*j1));
		EXPECT_FALSE(t->IsMarkedAsWeighted(*j2));

		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id="+(std::string)j1->GetUUID(), "firstname=first2&lastname=last2&weight=12,5&gender=1&birthyear=2001&number=A1234&club=" + (std::string)c1->GetUUID())));

		EXPECT_TRUE(t->IsMarkedAsWeighted(*j1));
		EXPECT_FALSE(t->IsMarkedAsWeighted(*j2));

		EXPECT_EQ(j1->GetFirstname(), "first2");
		EXPECT_EQ(j1->GetLastname(),  "last2");
		EXPECT_EQ(j1->GetWeight(),  Weight("12,5"));
		EXPECT_EQ(j1->GetGender(),  Gender::Female);
		EXPECT_EQ(j1->GetBirthyear(), 2001);
		EXPECT_EQ(j1->GetNumber(), "A1234");
		ASSERT_TRUE(j1->GetClub());
		EXPECT_EQ(*j1->GetClub(), *c1);


		app.GetDatabase().AddJudoka(new Judoka(*j1));

		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id="+(std::string)j1->GetUUID(), "firstname=first3&lastname=last3&weight=13,5&gender=0&birthyear=2002&number=A12345&club=" + (std::string)c1->GetUUID())));

		EXPECT_EQ(j1->GetFirstname(), "first3");
		EXPECT_EQ(j1->GetLastname(),  "last3");
		EXPECT_EQ(j1->GetWeight(),  Weight("13,5"));
		EXPECT_EQ(j1->GetGender(),  Gender::Male);
		EXPECT_EQ(j1->GetBirthyear(), 2002);
		EXPECT_EQ(j1->GetNumber(), "A12345");
		ASSERT_TRUE(j1->GetClub());
		EXPECT_EQ(*j1->GetClub(), *c1);

		EXPECT_EQ(app.GetDatabase().GetAllJudokas()[0]->GetFirstname(), "first3");
		EXPECT_EQ(app.GetDatabase().GetAllJudokas()[0]->GetLastname(),  "last3");
		EXPECT_EQ(app.GetDatabase().GetAllJudokas()[0]->GetWeight(),  Weight("13,5"));
		EXPECT_EQ(app.GetDatabase().GetAllJudokas()[0]->GetGender(),  Gender::Male);
		EXPECT_EQ(app.GetDatabase().GetAllJudokas()[0]->GetBirthyear(), 2002);
		EXPECT_EQ(app.GetDatabase().GetAllJudokas()[0]->GetNumber(), "A12345");
		ASSERT_TRUE(app.GetDatabase().GetAllJudokas()[0]->GetClub());
		EXPECT_EQ(*app.GetDatabase().GetAllJudokas()[0]->GetClub(), *c1);		
	}
}



TEST(Ajax, Judoka_Update_Participant_Weight)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Application app;

		auto t = new Tournament("deleteMe");
		t->EnableAutoSave(false);

		app.AddTournament(t);

		auto j1 = new Judoka("firstname", "lastname");
		t->AddParticipant(j1);

		auto j2 = new Judoka("firstname", "lastname");
		t->AddParticipant(j2);

		EXPECT_FALSE(t->IsMarkedAsWeighted(*j1));
		EXPECT_FALSE(t->IsMarkedAsWeighted(*j2));

		EXPECT_TRUE(app.Ajax_UpdateParticipantWeight(HttpServer::Request("id="+(std::string)j1->GetUUID()+"&weight=12,5")));

		EXPECT_TRUE(t->IsMarkedAsWeighted(*j1));
		EXPECT_FALSE(t->IsMarkedAsWeighted(*j2));
		EXPECT_EQ(j1->GetWeight(),  Weight("12,5"));

		app.GetDatabase().AddJudoka(new Judoka(*j1));

		EXPECT_TRUE(app.Ajax_UpdateParticipantWeight(HttpServer::Request("id="+(std::string)j1->GetUUID()+"&weight=13,5")));

		EXPECT_TRUE(t->IsMarkedAsWeighted(*j1));
		EXPECT_EQ(j1->GetWeight(),  Weight("13,5"));
	}
}



TEST(Ajax, Judoka_Delete)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddJudoka(HttpServer::Request("", "firstname=first&lastname=last&weight=10&gender=0&birthyear=2000&number=A123")), "ok");

		auto& judokas = app.GetDatabase().GetAllJudokas();

		auto id = judokas[0]->GetUUID();

		EXPECT_TRUE(app.Ajax_DeleteJudoka(HttpServer::Request("id="+(std::string)id)));

		EXPECT_EQ(judokas.size(), 0);
	}
}



TEST(Ajax, Judoka_Import)
{
	initialize();

	{
		Application app;

		Tournament* tourney = new Tournament;

		auto c1 = std::make_shared<Club>("Club 1");
		auto c2 = std::make_shared<Club>("Club 2");
		c1->SetShortName("c");
		auto j1 = new Judoka("first", "last");
		j1->SetClub(c1);
		auto j2 = new Judoka("first", "last");
		j2->SetClub(c2);

		tourney->AddParticipant(j1);
		tourney->AddParticipant(j2);

		app.AddTournament(tourney);
		

		auto& judokas = app.GetDatabase().GetAllJudokas();
		auto& clubs   = app.GetDatabase().GetAllClubs();

		EXPECT_TRUE(app.Ajax_ImportJudoka(HttpServer::Request("id="+(std::string)j1->GetUUID())));

		ASSERT_EQ(judokas.size(), 1);
		EXPECT_EQ(judokas[0]->GetUUID(), *j1);

		ASSERT_EQ(clubs.size(), 1);
		EXPECT_EQ(clubs[0]->GetUUID(), *c1);

		EXPECT_TRUE(app.Ajax_ImportJudoka(HttpServer::Request("id="+(std::string)j2->GetUUID())));

		ASSERT_EQ(judokas.size(), 2);

		ASSERT_EQ(clubs.size(), 1);//Don't import second club since it has the same name
		EXPECT_EQ(clubs[0]->GetUUID(), *c1);
	}
}



TEST(Ajax, GetNamesOnMat)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(5);
		auto mat = app.GetLocalMat();
		mat->SetName("mat name");

		Tournament* tourney = new Tournament;

		auto match1 = std::make_shared<Match>(new Judoka("A", "B"), new Judoka("C", "D"), nullptr, 5);
		auto match2 = std::make_shared<Match>(new Judoka("E", "F"), new Judoka("G", "H"), nullptr, 5);
		auto match3 = std::make_shared<Match>(new Judoka("I", "J"), new Judoka("K", "L"), nullptr, 5);

		tourney->AddMatch(match1);
		tourney->AddMatch(match2);
		tourney->AddMatch(match3);

		for (auto table : tourney->GetMatchTables())
		{
			if (table->FindMatch(*match1))
				table->SetName("table1");
			else if (table->FindMatch(*match2))
				table->SetName("table2");
			else if (table->FindMatch(*match3))
				table->SetName("table3");
		}

		app.AddTournament(tourney);

		YAML::Node yaml = YAML::Load( app.Ajax_GetNamesOnMat(HttpServer::Request("id=5")) );

		ASSERT_TRUE(yaml.IsMap());
		ASSERT_TRUE(yaml["white_name"]);
		EXPECT_EQ(yaml["white_name"].as<std::string>(), "- - -");
		EXPECT_EQ(yaml["blue_name" ].as<std::string>(), "- - -");
		EXPECT_EQ(yaml["mat_name"  ].as<std::string>(), "mat name");
		EXPECT_EQ(yaml["match_table_name" ].as<std::string>(), "- - -");
		ASSERT_TRUE(yaml["next_matches"]);
		ASSERT_TRUE(yaml["next_matches"].IsSequence());
		ASSERT_TRUE(yaml["next_matches"][0]);
		ASSERT_TRUE(yaml["next_matches"][1]);
		ASSERT_TRUE(yaml["next_matches"][2]);
		EXPECT_EQ(yaml["next_matches"][0]["white_name"].as<std::string>(), "A B");
		EXPECT_EQ(yaml["next_matches"][0]["blue_name" ].as<std::string>(), "C D");
		EXPECT_TRUE(yaml["next_matches"][0]["current_breaktime"].as<int>() > 0);
		EXPECT_EQ(yaml["next_matches"][0]["breaktime"].as<int>(), 0);
		EXPECT_EQ(yaml["next_matches"][1]["white_name"].as<std::string>(), "E F");
		EXPECT_EQ(yaml["next_matches"][1]["blue_name" ].as<std::string>(), "G H");
		EXPECT_EQ(yaml["next_matches"][2]["white_name"].as<std::string>(), "I J");
		EXPECT_EQ(yaml["next_matches"][2]["blue_name" ].as<std::string>(), "K L");

		EXPECT_EQ(yaml["next_matches"][0]["uuid"].as<std::string>(), match1->GetUUID());
		EXPECT_EQ(yaml["next_matches"][1]["uuid"].as<std::string>(), match2->GetUUID());
		EXPECT_EQ(yaml["next_matches"][2]["uuid"].as<std::string>(), match3->GetUUID());

		mat->StartMatch(match1);

		yaml = YAML::Load( app.Ajax_GetNamesOnMat(HttpServer::Request("id=5")) );

		ASSERT_TRUE(yaml.IsMap());
		ASSERT_TRUE(yaml["white_name"]);
		EXPECT_EQ(yaml["white_name"].as<std::string>(), "A B");
		EXPECT_EQ(yaml["blue_name" ].as<std::string>(), "C D");
		EXPECT_EQ(yaml["mat_name"  ].as<std::string>(), "mat name");
		EXPECT_EQ(yaml["match_table_name" ].as<std::string>(), "table1");
		ASSERT_TRUE(yaml["next_matches"].IsSequence());
		ASSERT_TRUE(yaml["next_matches"][0]);
		ASSERT_TRUE(yaml["next_matches"][1]);
		EXPECT_EQ(yaml["next_matches"][0]["white_name"].as<std::string>(), "E F");
		EXPECT_EQ(yaml["next_matches"][0]["blue_name" ].as<std::string>(), "G H");
		EXPECT_EQ(yaml["next_matches"][1]["white_name"].as<std::string>(), "I J");
		EXPECT_EQ(yaml["next_matches"][1]["blue_name" ].as<std::string>(), "K L");

		EXPECT_EQ(yaml["next_matches"][0]["uuid"].as<std::string>(), match2->GetUUID());
		EXPECT_EQ(yaml["next_matches"][1]["uuid"].as<std::string>(), match3->GetUUID());

		mat->AddIppon(Fighter::White);
		mat->EndMatch();

		yaml = YAML::Load( app.Ajax_GetNamesOnMat(HttpServer::Request("id=5")) );

		ASSERT_TRUE(yaml.IsMap());
		ASSERT_TRUE(yaml["white_name"]);
		EXPECT_EQ(yaml["white_name"].as<std::string>(), "- - -");
		EXPECT_EQ(yaml["blue_name" ].as<std::string>(), "- - -");
		EXPECT_EQ(yaml["mat_name"  ].as<std::string>(), "mat name");
		EXPECT_EQ(yaml["match_table_name" ].as<std::string>(), "- - -");
		ASSERT_TRUE(yaml["next_matches"].IsSequence());
		ASSERT_TRUE(yaml["next_matches"][0]);
		ASSERT_TRUE(yaml["next_matches"][1]);
		EXPECT_EQ(yaml["next_matches"][0]["white_name"].as<std::string>(), "E F");
		EXPECT_EQ(yaml["next_matches"][0]["blue_name" ].as<std::string>(), "G H");
		EXPECT_EQ(yaml["next_matches"][1]["white_name"].as<std::string>(), "I J");
		EXPECT_EQ(yaml["next_matches"][1]["blue_name" ].as<std::string>(), "K L");

		mat->StartMatch(match2);

		yaml = YAML::Load( app.Ajax_GetNamesOnMat(HttpServer::Request("id=5")) );

		ASSERT_TRUE(yaml.IsMap());
		ASSERT_TRUE(yaml["white_name"]);
		EXPECT_EQ(yaml["white_name"].as<std::string>(), "E F");
		EXPECT_EQ(yaml["blue_name" ].as<std::string>(), "G H");
		EXPECT_EQ(yaml["mat_name"  ].as<std::string>(), "mat name");
		EXPECT_EQ(yaml["match_table_name" ].as<std::string>(), "table2");
		ASSERT_TRUE(yaml["next_matches"].IsSequence());
		EXPECT_EQ(yaml["next_matches"][0]["white_name"].as<std::string>(), "I J");
		EXPECT_EQ(yaml["next_matches"][0]["blue_name" ].as<std::string>(), "K L");

		mat->AddIppon(Fighter::White);
		mat->EndMatch();

		yaml = YAML::Load( app.Ajax_GetNamesOnMat(HttpServer::Request("id=5")) );

		ASSERT_TRUE(yaml.IsMap());
		ASSERT_TRUE(yaml["white_name"]);
		EXPECT_EQ(yaml["white_name"].as<std::string>(), "- - -");
		EXPECT_EQ(yaml["blue_name" ].as<std::string>(), "- - -");
		ASSERT_TRUE(yaml["next_matches"].IsSequence());
		EXPECT_EQ(yaml["next_matches"].size(), 1);
		EXPECT_EQ(yaml["next_matches"][0]["white_name"].as<std::string>(), "I J");
		EXPECT_EQ(yaml["next_matches"][0]["blue_name" ].as<std::string>(), "K L");

		EXPECT_TRUE(mat->Close());
		ZED::Core::Pause(500);
	}
}



TEST(Ajax, Clubs_Add)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddClub(HttpServer::Request("", "name=Test Club&shortname=short")), "ok");

		auto& clubs = app.GetDatabase().GetAllClubs();

		ASSERT_EQ(clubs.size(), 1);
		EXPECT_EQ(clubs[0]->GetName(), "Test Club");
		EXPECT_EQ(clubs[0]->GetShortName(), "short");


		auto inter = std::make_shared<Judoboard::Association>("International", nullptr);

		app.GetDatabase().AddAssociation(inter);

		EXPECT_EQ((std::string)app.Ajax_AddClub(HttpServer::Request("", "name=Test Club 2&shortname=test&parent=" + (std::string)inter->GetUUID())), "ok");

		ASSERT_EQ(clubs.size(), 2);
		EXPECT_EQ(clubs[1]->GetName(), "Test Club 2");
		EXPECT_EQ(clubs[1]->GetShortName(), "test");
		ASSERT_TRUE(clubs[1]->GetParent());
		EXPECT_EQ(*clubs[1]->GetParent(), *inter);

		EXPECT_EQ(app.Ajax_AddClub(HttpServer::Request("", "name=Test Club 3&parent=XXX")), Error(Error::Type::OperationFailed));
	}
}



TEST(Ajax, AddAssociation)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddClub(HttpServer::Request("is_association=true", "name=Test Club&shortname=short")), "ok");

		auto& clubs = app.GetDatabase().GetAllAssociations();

		ASSERT_EQ(clubs.size(), 1);
		EXPECT_EQ(clubs[0]->GetName(), "Test Club");
		EXPECT_EQ(clubs[0]->GetShortName(), "short");
	}
}



TEST(Ajax, GetClub)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddClub(HttpServer::Request("", "name=Test Club&shortname=club")), "ok");

		auto clubs = app.GetDatabase().GetAllClubs();

		ASSERT_EQ(clubs.size(), 1);
		EXPECT_EQ(clubs[0]->GetName(), "Test Club");
		EXPECT_EQ(clubs[0]->GetShortName(), "club");

		Club club2(YAML::Load(app.Ajax_GetClub(HttpServer::Request("id="+(std::string)clubs[0]->GetUUID()))));

		EXPECT_EQ(clubs[0]->GetName(),   club2.GetName());
		EXPECT_EQ(clubs[0]->GetUUID(),   club2.GetUUID());
		EXPECT_EQ(clubs[0]->GetParent(), club2.GetParent());
		EXPECT_EQ(clubs[0]->GetLevel(),  club2.GetLevel());
		EXPECT_EQ(clubs[0]->GetShortName(), club2.GetShortName());
	}
}



TEST(Ajax, EditClub)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddClub(HttpServer::Request("", "name=Test Club")), "ok");

		auto clubs = app.GetDatabase().GetAllClubs();

		ASSERT_EQ(clubs.size(), 1);
		EXPECT_EQ(clubs[0]->GetName(), "Test Club");

		EXPECT_EQ((std::string)app.Ajax_EditClub(HttpServer::Request("id="+(std::string)clubs[0]->GetUUID(), "name=Test Club 2&shortname=test")), "ok");

		EXPECT_EQ(clubs[0]->GetName(), "Test Club 2");
		EXPECT_EQ(clubs[0]->GetShortName(), "test");
	}
}



TEST(Ajax, DeleteClub)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddClub(HttpServer::Request("", "name=Test Club")), "ok");

		auto& clubs = app.GetDatabase().GetAllClubs();

		ASSERT_EQ(clubs.size(), 1);

		EXPECT_EQ((std::string)app.Ajax_DeleteClub(HttpServer::Request("id="+(std::string)clubs[0]->GetUUID(), "")), "ok");

		EXPECT_EQ(clubs.size(), 0);
	}
}



TEST(Ajax, DeleteAssociation)
{
	initialize();

	{
		Application app;

		auto inter = std::make_shared<Judoboard::Association>("International", nullptr);

		auto de = std::make_shared<Judoboard::Association>("Deutschland", inter);

		auto dn = std::make_shared<Judoboard::Association>("Deutschland-Nord", de);
		auto ds = std::make_shared<Judoboard::Association>(u8"Deutschland-S\u00fcd", de);

		auto nord  = std::make_shared<Judoboard::Association>("Nord", dn);
		auto west  = std::make_shared<Judoboard::Association>("West", dn);
		auto nost  = std::make_shared<Judoboard::Association>("Nordost", dn);
		auto sued  = std::make_shared<Judoboard::Association>(u8"S\u00fcd", ds);
		auto swest = std::make_shared<Judoboard::Association>(u8"S\u00fcdwest", ds);

		auto nieder  = std::make_shared<Judoboard::Association>("Niedersachsen", nord);
		auto hamburg = std::make_shared<Judoboard::Association>("Hamburg", nord);
		auto berlin  = std::make_shared<Judoboard::Association>("Berlin", nost);
		auto nrw     = std::make_shared<Judoboard::Association>("Nordrhein-Westfalen", west);

		app.GetDatabase().AddAssociation(nieder);
		app.GetDatabase().AddAssociation(hamburg);
		app.GetDatabase().AddAssociation(berlin);
		app.GetDatabase().AddAssociation(nrw);


		EXPECT_EQ(app.Ajax_DeleteClub(HttpServer::Request("id=" + (std::string)de->GetUUID())), Error(Error::Type::OperationFailed));
		EXPECT_EQ((std::string)app.Ajax_DeleteClub(HttpServer::Request("id=" + (std::string)nieder->GetUUID())), "ok");

		EXPECT_TRUE(app.GetDatabase().FindAssociation(de->GetUUID()));
		EXPECT_FALSE(app.GetDatabase().FindAssociation(nieder->GetUUID()));
	}
}



TEST(Ajax, EditAssociation)
{
	initialize();

	{
		Application app;

		auto inter = std::make_shared<Judoboard::Association>("International", nullptr);

		auto de = std::make_shared<Judoboard::Association>("Deutschland", inter);

		auto dn = std::make_shared<Judoboard::Association>("Deutschland-Nord", de);
		auto ds = std::make_shared<Judoboard::Association>(u8"Deutschland-S\u00fcd", de);

		auto nord  = std::make_shared<Judoboard::Association>("Nord", dn);
		auto west  = std::make_shared<Judoboard::Association>("West", dn);
		auto nost  = std::make_shared<Judoboard::Association>("Nordost", dn);
		auto sued  = std::make_shared<Judoboard::Association>(u8"S\u00fcd", ds);
		auto swest = std::make_shared<Judoboard::Association>(u8"S\u00fcdwest", ds);

		auto nieder  = std::make_shared<Judoboard::Association>("Niedersachsen", nord);
		auto hamburg = std::make_shared<Judoboard::Association>("Hamburg", nord);
		auto berlin  = std::make_shared<Judoboard::Association>("Berlin", nost);
		auto nrw     = std::make_shared<Judoboard::Association>("Nordrhein-Westfalen", west);

		app.GetDatabase().AddAssociation(nieder);
		app.GetDatabase().AddAssociation(hamburg);
		app.GetDatabase().AddAssociation(berlin);
		app.GetDatabase().AddAssociation(nrw);


		EXPECT_EQ((std::string)app.Ajax_EditClub(HttpServer::Request("id=" + (std::string)nieder->GetUUID(), "name=Niedersachen 2&shortname=NS&parent=" + (std::string)de->GetUUID())), "ok");

		EXPECT_EQ(nieder->GetName(), "Niedersachen 2");
		EXPECT_EQ(nieder->GetShortName(), "NS");
		EXPECT_EQ(nieder->GetLevel(), 2);
		EXPECT_EQ(*nieder->GetParent(), *de);
	}
}



TEST(Ajax, Clubs_List)
{
	initialize();

	{
		Application app;

		auto c1 = std::make_shared<Club>("Club 1");
		c1->SetShortName("c1");
		app.GetDatabase().AddClub(c1);

		YAML::Node yaml = YAML::Load(app.Ajax_ListClubs(HttpServer::Request("")));

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Club 1");


		app.GetDatabase().AddClub(std::make_shared<Club>("Club 2"));

		yaml = YAML::Load(app.Ajax_ListClubs(HttpServer::Request("")));

		ASSERT_EQ(yaml.size(), 2);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Club 1");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "c1");
		EXPECT_EQ(yaml[1]["name"].as<std::string>(), "Club 2");
	}
}



TEST(Ajax, Clubs_List_All)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Application app;

		auto c = std::make_shared<Club>("Club 1");
		c->SetShortName("c");

		auto t = new Tournament("deleteMe");
		t->EnableAutoSave(false);

		app.AddTournament(t);

		auto j = new Judoka("first", "last");
		j->SetClub(c);
		t->AddParticipant(j);

		auto yaml = YAML::Load(app.Ajax_ListClubs(HttpServer::Request("all=true")));

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Club 1");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "c");
	}
}



TEST(Ajax, Clubs_Get)
{
	initialize();

	{
		Application app;

		auto c = std::make_shared<Club>("Club 1");
		c->SetShortName("c");
		app.GetDatabase().AddClub(c);

		YAML::Node yaml = YAML::Load(app.Ajax_GetClub(HttpServer::Request("id=" + (std::string)c->GetUUID())));

		EXPECT_EQ(yaml["uuid"].as<std::string>(), c->GetUUID());
		EXPECT_EQ(yaml["name"].as<std::string>(), c->GetName());
		EXPECT_EQ(yaml["short_name"].as<std::string>(), c->GetShortName());
	}
}



TEST(Ajax, Clubs_Get_From_Tournament)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	{
		Application app;

		auto c = std::make_shared<Club>("Club 1");
		c->SetShortName("c");

		auto t = new Tournament("deleteMe");
		t->EnableAutoSave(false);

		app.AddTournament(t);

		auto j = new Judoka("first", "last");
		j->SetClub(c);
		t->AddParticipant(j);

		YAML::Node yaml = YAML::Load(app.Ajax_GetClub(HttpServer::Request("id=" + (std::string)c->GetUUID())));

		EXPECT_EQ(yaml["uuid"].as<std::string>(), c->GetUUID());
		EXPECT_EQ(yaml["name"].as<std::string>(), c->GetName());
		EXPECT_EQ(yaml["short_name"].as<std::string>(), c->GetShortName());
	}
}



TEST(Ajax, Clubs_Edit)
{
	initialize();

	{
		Application app;

		auto c = std::make_shared<Club>("Club 1");
		app.GetDatabase().AddClub(c);

		EXPECT_EQ((std::string)app.Ajax_EditClub(HttpServer::Request("id=" + (std::string)c->GetUUID(), "name=NewName&shortname=c")), "ok");


		auto yaml = YAML::Load(app.Ajax_ListClubs(HttpServer::Request("")));

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), c->GetUUID());
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "NewName");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "c");
	}
}



TEST(Ajax, Clubs_Delete)
{
	initialize();

	{
		Application app;

		auto c = std::make_shared<Club>("Club 1");
		app.GetDatabase().AddClub(c);

		EXPECT_EQ((std::string)app.Ajax_DeleteClub(HttpServer::Request("id=" + (std::string)c->GetUUID())), "ok");

		auto yaml = YAML::Load(app.Ajax_ListClubs(HttpServer::Request("")));

		ASSERT_EQ(yaml.size(), 0);
	}
}



TEST(Ajax, ListAssociations)
{
	initialize();

	{
		Application app;

		auto assoc1 = std::make_shared<Association>("Assoc 1", nullptr);
		assoc1->SetShortName("assoc1");
		app.GetDatabase().AddAssociation(assoc1);

		YAML::Node yaml = YAML::Load(app.Ajax_ListAssociations(HttpServer::Request("")));

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "assoc1");
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)assoc1->GetUUID());


		auto assoc2 = std::make_shared<Association>("Assoc 2", assoc1);
		assoc2->SetShortName("assoc2");
		app.GetDatabase().AddAssociation(assoc2);

		auto club1 = std::make_shared<Club>("Club 1", assoc1);
		app.GetDatabase().AddClub(club1);

		yaml = YAML::Load(app.Ajax_ListAssociations(HttpServer::Request("also_clubs=true")));

		ASSERT_EQ(yaml.size(), 3);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "assoc1");
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)assoc1->GetUUID());

		EXPECT_EQ(yaml[1]["name"].as<std::string>(), "Assoc 2");
		EXPECT_EQ(yaml[1]["short_name"].as<std::string>(), "assoc2");
		EXPECT_EQ(yaml[1]["uuid"].as<std::string>(), (std::string)assoc2->GetUUID());
		EXPECT_EQ(yaml[1]["parent_name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[1]["parent_uuid"].as<std::string>(), (std::string)assoc1->GetUUID());

		EXPECT_EQ(yaml[2]["name"].as<std::string>(), "Club 1");
		EXPECT_EQ(yaml[2]["uuid"].as<std::string>(), (std::string)club1->GetUUID());
		EXPECT_EQ(yaml[2]["parent_name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[2]["parent_uuid"].as<std::string>(), (std::string)assoc1->GetUUID());

		yaml = YAML::Load(app.Ajax_ListAssociations(HttpServer::Request("")));

		ASSERT_EQ(yaml.size(), 2);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "assoc1");
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)assoc1->GetUUID());

		EXPECT_EQ(yaml[1]["name"].as<std::string>(), "Assoc 2");
		EXPECT_EQ(yaml[1]["short_name"].as<std::string>(), "assoc2");
		EXPECT_EQ(yaml[1]["uuid"].as<std::string>(), (std::string)assoc2->GetUUID());
		EXPECT_EQ(yaml[1]["parent_name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[1]["parent_uuid"].as<std::string>(), (std::string)assoc1->GetUUID());

		yaml = YAML::Load(app.Ajax_ListAssociations(HttpServer::Request("only_children=true")));

		ASSERT_EQ(yaml.size(), 1);

		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Assoc 2");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "assoc2");
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)assoc2->GetUUID());
		EXPECT_EQ(yaml[0]["parent_name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[0]["parent_uuid"].as<std::string>(), (std::string)assoc1->GetUUID());
	}
}



TEST(Ajax, Lottery)
{
	initialize();

	{
		ZED::Core::RemoveFile("tournaments/deleteMe.yml");

		size_t c1_count = 0;
		size_t c2_count = 0;

		for (int i = 0; i < 100; ++i)
		{
			Application app;
			Tournament* tourney = new Tournament("deleteMe");
			tourney->EnableAutoSave(false);

			app.AddTournament(tourney);

			auto assoc = std::make_shared<Association>(GetRandomName(), nullptr);
			auto c1 = std::make_shared<Club>(GetRandomName());
			c1->SetParent(assoc);
			auto c2 = std::make_shared<Club>(GetRandomName());
			c2->SetParent(assoc);

			Judoka* j1 = new Judoka(GetRandomName(), GetRandomName());
			j1->SetClub(c1);
			Judoka* j2 = new Judoka(GetRandomName(), GetRandomName());
			j2->SetClub(c2);

			tourney->SetOrganizer(assoc);

			EXPECT_TRUE(tourney->AddParticipant(j1));
			EXPECT_TRUE(tourney->AddParticipant(j2));

			EXPECT_TRUE(app.Ajax_PerformLottery());

			auto lot1 = tourney->GetLotOfAssociation(*c1);
			auto lot2 = tourney->GetLotOfAssociation(*c2);

			c1_count += lot1;
			c2_count += lot2;
		}

		EXPECT_GE(c1_count, 35);
		EXPECT_LE(c1_count, 65);
		EXPECT_GE(c2_count, 35);
		EXPECT_LE(c2_count, 65);
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Ajax, LotteryTier)
{
	initialize();	

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	for (int tier = 0; tier < 10; ++tier)
	{
		Application app;
		Tournament* tourney = new Tournament("deleteMe");
		tourney->EnableAutoSave(false);

		app.AddTournament(tourney);

		auto inter = std::make_shared<Judoboard::Association>("International", nullptr);

		auto de = std::make_shared<Judoboard::Association>("Deutschland", inter);

		auto dn = std::make_shared<Judoboard::Association>("Deutschland-Nord", de);
		auto ds = std::make_shared<Judoboard::Association>(u8"Deutschland-S\u00fcd", de);

		auto nord  = std::make_shared<Judoboard::Association>("Nord", dn);
		auto west  = std::make_shared<Judoboard::Association>("West", dn);
		auto nost  = std::make_shared<Judoboard::Association>("Nordost", dn);
		auto sued  = std::make_shared<Judoboard::Association>(u8"S\u00fcd", ds);
		auto swest = std::make_shared<Judoboard::Association>(u8"S\u00fcdwest", ds);

		auto nieder  = std::make_shared<Judoboard::Association>("Niedersachsen", nord);
		auto hamburg = std::make_shared<Judoboard::Association>("Hamburg", nord);
		auto berlin  = std::make_shared<Judoboard::Association>("Berlin", nost);
		auto nrw     = std::make_shared<Judoboard::Association>("Nordrhein-Westfalen", west);

		auto detmold = std::make_shared<Judoboard::Association>("Detmold", nrw);

		auto biegue = std::make_shared<Judoboard::Association>(u8"Bielefeld/G\u00fctersloh", detmold);

		auto c1 = std::make_shared<Club>("club1", biegue);
		auto c2 = std::make_shared<Club>("club2", biegue);

		Judoka* j1 = new Judoka(GetRandomName(), GetRandomName());
		j1->SetClub(c1);
		Judoka* j2 = new Judoka(GetRandomName(), GetRandomName());
		j2->SetClub(c2);

		tourney->SetOrganizer(de);
		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));

		EXPECT_TRUE( app.Ajax_SetLotteryTier(HttpServer::Request("tier=" + std::to_string(tier))) );

		EXPECT_EQ(tourney->GetLotteryTier(), tier);

		YAML::Node yaml = YAML::Load( app.Ajax_GetLotteryTier() );

		ASSERT_TRUE(yaml["tier"].IsDefined());
		EXPECT_EQ(yaml["tier"].as<uint32_t>(), tourney->GetLotteryTier());
	}
}



TEST(Ajax, ListLots)
{
	initialize();

	{
		ZED::Core::RemoveFile("tournaments/deleteMe.yml");

		Application app;
		Tournament* tourney = new Tournament("deleteMe");
		tourney->EnableAutoSave(false);

		app.AddTournament(tourney);

		auto assoc = std::make_shared<Association>(GetRandomName(), nullptr);
		auto c1 = std::make_shared<Club>(GetRandomName());
		c1->SetParent(assoc);
		auto c2 = std::make_shared<Club>(GetRandomName());
		c2->SetParent(assoc);

		Judoka* j1 = new Judoka(GetRandomName(), GetRandomName());
		j1->SetClub(c1);
		Judoka* j2 = new Judoka(GetRandomName(), GetRandomName());
		j2->SetClub(c2);

		tourney->SetOrganizer(assoc);

		EXPECT_TRUE(tourney->AddParticipant(j1));
		EXPECT_TRUE(tourney->AddParticipant(j2));

		EXPECT_TRUE(app.Ajax_PerformLottery());

		YAML::Node lots = YAML::Load(app.Ajax_ListLots());

		auto lot1 = tourney->GetLotOfAssociation(*c1);
		auto lot2 = tourney->GetLotOfAssociation(*c2);

		ASSERT_TRUE(lots["0"]["uuid"].IsDefined());
		ASSERT_TRUE(lots["1"]["uuid"].IsDefined());
		ASSERT_TRUE(lots["0"]["lot"].IsDefined());
		ASSERT_TRUE(lots["1"]["lot"].IsDefined());

		if (lot1 == lots["0"]["lot"].as<int>())
		{
			EXPECT_EQ(lots["0"]["uuid"].as<std::string>(), c1->GetUUID());
			EXPECT_EQ(lots["0"]["name"].as<std::string>(), c1->GetName());
			EXPECT_EQ(lots["0"]["lot"].as<int>(), lot1);

			EXPECT_EQ(lots["1"]["uuid"].as<std::string>(), c2->GetUUID());
			EXPECT_EQ(lots["1"]["name"].as<std::string>(), c2->GetName());
			EXPECT_EQ(lots["1"]["lot"].as<int>(), lot2);
		}
		else
		{
			EXPECT_EQ(lots["0"]["uuid"].as<std::string>(), c2->GetUUID());
			EXPECT_EQ(lots["0"]["name"].as<std::string>(), c2->GetName());
			EXPECT_EQ(lots["0"]["lot"].as<int>(), lot2);

			EXPECT_EQ(lots["1"]["uuid"].as<std::string>(), c1->GetUUID());
			EXPECT_EQ(lots["1"]["name"].as<std::string>(), c1->GetName());
			EXPECT_EQ(lots["1"]["lot"].as<int>(), lot1);
		}
	}

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
}



TEST(Ajax, AddDisqualification)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		auto match = std::make_shared<Match>(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		EXPECT_TRUE(mat->StartMatch(match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsUnknownDisqualification());

		EXPECT_TRUE(app.Ajax_AddDisqualification(f, HttpServer::Request("id=1")));

		EXPECT_TRUE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsUnknownDisqualification());
	}
}



TEST(Ajax, RemoveDisqualification)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		auto match = std::make_shared<Match>(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		EXPECT_TRUE(mat->StartMatch(match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsUnknownDisqualification());

		EXPECT_TRUE(app.Ajax_AddDisqualification(f, HttpServer::Request("id=1")));

		EXPECT_TRUE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsUnknownDisqualification());

		EXPECT_FALSE(app.Ajax_RemoveDisqualification(f, HttpServer::Request("id=2")));

		EXPECT_TRUE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsUnknownDisqualification());

		EXPECT_TRUE(app.Ajax_RemoveDisqualification(f, HttpServer::Request("id=1")));

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsUnknownDisqualification());
	}
}



TEST(Ajax, Match_Add)
{
	initialize();

	{
		Application app;

		auto j1 = new Judoka("a", "b");
		auto j2 = new Judoka("c", "d");

		auto r1 = std::make_shared<RuleSet>("test1", 60, 30, 10, 5);
		auto r2 = std::make_shared<RuleSet>("test2", 60, 30, 10, 5);

		app.GetTournament()->AddParticipant(j1);
		app.GetTournament()->AddParticipant(j2);
		app.GetTournament()->AddRuleSet(r1);
		app.GetTournament()->AddRuleSet(r2);

		EXPECT_TRUE(app.Ajax_AddMatch(HttpServer::Request("", "white=" + (std::string)j1->GetUUID() + "&blue=" + (std::string)j2->GetUUID() + "&mat=2&rule=" + (std::string)r1->GetUUID() )));

		auto table = app.GetTournament()->GetMatchTables()[0];
		auto match = app.GetTournament()->GetSchedule()[0];

		EXPECT_EQ(*match->GetFighter(Fighter::White), *j1);
		EXPECT_EQ(*match->GetFighter(Fighter::Blue),  *j2);
		EXPECT_EQ(match->GetMatID(), 2);
		EXPECT_EQ(match->GetRuleSet().GetUUID(), r1->GetUUID());
		EXPECT_EQ(table->GetRuleSet().GetUUID(), r1->GetUUID());

		EXPECT_TRUE(app.Ajax_AddMatch(HttpServer::Request("", "white=" + (std::string)j2->GetUUID() + "&blue=" + (std::string)j1->GetUUID() + "&mat=2&rule=" + (std::string)r1->GetUUID() + "&match_table=" + (std::string)table->GetUUID() )));

		auto match2 = app.GetTournament()->GetSchedule()[1];

		EXPECT_EQ(*match2->GetFighter(Fighter::White), *j2);
		EXPECT_EQ(*match2->GetFighter(Fighter::Blue),  *j1);
		EXPECT_EQ(match2->GetMatID(), 2);
		EXPECT_EQ(match2->GetRuleSet().GetUUID(), r1->GetUUID());
		EXPECT_EQ(*match2->GetMatchTable(), *table);

		EXPECT_TRUE(app.Ajax_AddMatch(HttpServer::Request("", "white=" + (std::string)j1->GetUUID() + "&blue=" + (std::string)j2->GetUUID() + "&mat=1&rule=" + (std::string)r2->GetUUID() + "&match_table=" + (std::string)table->GetUUID() )));

		auto match3 = app.GetTournament()->GetSchedule()[2];

		EXPECT_EQ(*match3->GetFighter(Fighter::White), *j1);
		EXPECT_EQ(*match3->GetFighter(Fighter::Blue),  *j2);
		EXPECT_EQ(match3->GetMatID(), 1);
		EXPECT_EQ(match3->GetRuleSet().GetUUID(), r2->GetUUID());
		EXPECT_EQ(*match3->GetMatchTable(), *table);
	}
}



TEST(Ajax, Match_Edit)
{
	initialize();

	{
		Application app;

		auto j1 = new Judoka("a", "b");
		auto j2 = new Judoka("c", "d");

		auto r1 = std::make_shared<RuleSet>("test", 60, 30, 10, 5);

		auto match = std::make_shared<Match>(j1, j2, nullptr);
		app.GetTournament()->AddMatch(match);
		app.GetTournament()->AddRuleSet(r1);

		EXPECT_TRUE(app.Ajax_EditMatch(HttpServer::Request("id=" + (std::string)match->GetUUID(), "mat=1&rule=" + (std::string)r1->GetUUID())));

		EXPECT_EQ(match->GetMatID(), 1);
		EXPECT_EQ(match->GetRuleSet().GetUUID(), r1->GetUUID());

		EXPECT_TRUE(app.Ajax_EditMatch(HttpServer::Request("id=" + (std::string)match->GetUUID(), "mat=2&rule=0")));

		EXPECT_EQ(match->GetMatID(), 2);
		EXPECT_NE(match->GetRuleSet().GetUUID(), *r1);
	}
}



TEST(Ajax, MatchTable_Add)
{
	initialize();

	{
		Application app;

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test&mat=7&minWeight=9,7&maxWeight=19.3")));

		ASSERT_EQ(tables.size(), 1);
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::RoundRobin);
		EXPECT_EQ(tables[0]->GetName(), "Test");
		EXPECT_EQ(tables[0]->GetMatID(), 7);
		ASSERT_TRUE(tables[0]->GetFilter());


		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test2&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")));

		ASSERT_EQ(tables.size(), 2);
		ASSERT_EQ(tables[1]->GetType(), MatchTable::Type::RoundRobin);
		EXPECT_EQ(tables[1]->GetName(), "Test2");
		EXPECT_EQ(tables[1]->GetMatID(), 5);
		ASSERT_TRUE(tables[1]->GetFilter());
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[1]->GetFilter())->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[1]->GetFilter())->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[1]->GetFilter())->GetGender(), Gender::Male);
		EXPECT_EQ(std::dynamic_pointer_cast<RoundRobin>(tables[1])->IsBestOfThree(), true);

		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=3&name=Test3&mat=5&minWeight=10,8&maxWeight=20.4&gender=1&bo3=true&mf3=true&mf5=true")));

		ASSERT_EQ(tables.size(), 3);
		ASSERT_EQ(tables[2]->GetType(), MatchTable::Type::SingleElimination);
		EXPECT_EQ(tables[2]->GetName(), "Test3");
		EXPECT_EQ(tables[2]->GetMatID(), 5);
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[2]->GetFilter())->GetMinWeight(), Weight("10,8"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[2]->GetFilter())->GetMaxWeight(), Weight("20.4"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[2]->GetFilter())->GetGender(), Gender::Female);
		EXPECT_EQ(std::dynamic_pointer_cast<SingleElimination>(tables[2])->IsBestOfThree(), true);
		EXPECT_EQ(std::dynamic_pointer_cast<SingleElimination>(tables[2])->IsThirdPlaceMatch(), true);
		EXPECT_EQ(std::dynamic_pointer_cast<SingleElimination>(tables[2])->IsFifthPlaceMatch(), true);

		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=4&name=Test4&mat=5&minWeight=10,9&maxWeight=20.5&gender=1&bo3=true&mf3=true&mf5=true")), "ok");

		ASSERT_EQ(tables.size(), 4);
		ASSERT_EQ(tables[3]->GetType(), MatchTable::Type::Pool);
		EXPECT_EQ(tables[3]->GetName(), "Test4");
		EXPECT_EQ(tables[3]->GetMatID(), 5);
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[3]->GetFilter())->GetMinWeight(), Weight("10,9"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[3]->GetFilter())->GetMaxWeight(), Weight("20.5"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[3]->GetFilter())->GetGender(), Gender::Female);
		EXPECT_EQ(std::dynamic_pointer_cast<Pool>(tables[3])->IsBestOfThree(), true);
		EXPECT_EQ(std::dynamic_pointer_cast<Pool>(tables[3])->IsThirdPlaceMatch(), true);
		EXPECT_EQ(std::dynamic_pointer_cast<Pool>(tables[3])->IsFifthPlaceMatch(), true);



		Judoka* j1 = new Judoka("Firstname1", "Lastname1", 40, Gender::Female);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 40, Gender::Female);
		Judoka* j3 = new Judoka("Firstname1", "Lastname1", 40, Gender::Female);
		Judoka* j4 = new Judoka("Firstname2", "Lastname2", 40, Gender::Female);
		Judoka* j5 = new Judoka("Firstname1", "Lastname1", 40, Gender::Female);
		Judoka* j6 = new Judoka("Firstname2", "Lastname2", 40, Gender::Female);
		Judoka* j7 = new Judoka("Firstname1", "Lastname1", 40, Gender::Female);
		Judoka* j8 = new Judoka("Firstname2", "Lastname2", 40, Gender::Female);

		app.GetTournament()->AddParticipant(j1);
		app.GetTournament()->AddParticipant(j2);
		app.GetTournament()->AddParticipant(j3);
		app.GetTournament()->AddParticipant(j4);
		app.GetTournament()->AddParticipant(j5);
		app.GetTournament()->AddParticipant(j6);
		app.GetTournament()->AddParticipant(j7);
		app.GetTournament()->AddParticipant(j8);

		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=6&name=Test5&mat=5&minWeight=30,9&maxWeight=50.5&gender=1&bo3=false&mf3=false&mf5=false")));

		ASSERT_EQ(tables.size(), 5);
		ASSERT_EQ(tables[4]->GetType(), MatchTable::Type::DoubleElimination);
		EXPECT_EQ(tables[4]->GetName(), "Test5");
		EXPECT_EQ(tables[4]->GetMatID(), 5);
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[4]->GetFilter())->GetMinWeight(), Weight("30,9"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[4]->GetFilter())->GetMaxWeight(), Weight("50.5"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[4]->GetFilter())->GetGender(), Gender::Female);
		EXPECT_EQ(std::dynamic_pointer_cast<DoubleElimination>(tables[4])->IsBestOfThree(), false);
		EXPECT_EQ(std::dynamic_pointer_cast<DoubleElimination>(tables[4])->IsThirdPlaceMatch(), false);
		EXPECT_EQ(std::dynamic_pointer_cast<DoubleElimination>(tables[4])->IsFifthPlaceMatch(), false);
	}
}



TEST(Ajax, MatchTable_Edit)
{
	initialize();

	{
		Application app;

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test&mat=7")), "ok");

		ASSERT_EQ(tables.size(), 1);
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::RoundRobin);
		EXPECT_EQ(tables[0]->GetName(), "Test");
		EXPECT_EQ(tables[0]->GetMatID(), 7);


		EXPECT_EQ((std::string)app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID(), "name=Test2&fight_system=1&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")), "ok");

		ASSERT_EQ(tables.size(), 1);
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::RoundRobin);
		EXPECT_EQ(tables[0]->GetName(), "Test2");
		EXPECT_EQ(tables[0]->GetMatID(), 5);
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[0]->GetFilter())->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[0]->GetFilter())->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[0]->GetFilter())->GetGender(), Gender::Male);
		EXPECT_EQ(std::dynamic_pointer_cast<RoundRobin>(tables[0])->IsBestOfThree(), true);

		tables[0]->SetColor(Color::Name::Purple);
		tables[0]->SetScheduleIndex(10);

		auto old_uuid = tables[0]->GetUUID();

		EXPECT_TRUE(app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID(), "name=Test2&fight_system=3&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")) );

		ASSERT_EQ(tables.size(), 1);
		EXPECT_EQ(tables[0]->GetUUID(), old_uuid);
		EXPECT_EQ(tables[0]->GetColor(), Color::Name::Purple);
		EXPECT_EQ(tables[0]->GetScheduleIndex(), 9);//Gets changed since 9 is unused
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::SingleElimination);
		EXPECT_EQ(tables[0]->GetName(), "Test2");
		EXPECT_EQ(tables[0]->GetMatID(), 5);
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[0]->GetFilter())->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[0]->GetFilter())->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[0]->GetFilter())->GetGender(), Gender::Male);
		EXPECT_EQ(std::dynamic_pointer_cast<RoundRobin>(tables[0])->IsBestOfThree(), true);


		EXPECT_TRUE(app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID(), "name=Test2&fight_system=4&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")));

		ASSERT_EQ(tables.size(), 1);
		EXPECT_EQ(tables[0]->GetUUID(), old_uuid);
		EXPECT_EQ(tables[0]->GetColor(), Color::Name::Purple);
		EXPECT_EQ(tables[0]->GetScheduleIndex(), 8);//Gets changed since 8 is unused
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::Pool);
		EXPECT_EQ(tables[0]->GetName(), "Test2");
		EXPECT_EQ(tables[0]->GetMatID(), 5);
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[0]->GetFilter())->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[0]->GetFilter())->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ(std::dynamic_pointer_cast<Weightclass>(tables[0]->GetFilter())->GetGender(), Gender::Male);
		EXPECT_EQ(std::dynamic_pointer_cast<RoundRobin>(tables[0])->IsBestOfThree(), true);

		EXPECT_TRUE(app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)(std::dynamic_pointer_cast<Pool>(tables[0])->GetFinals())->GetUUID(), "name=Test4&fight_system=3&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")));

		auto finals = std::dynamic_pointer_cast<Pool>(tables[0])->GetFinals();

		EXPECT_EQ(finals->GetName(), "Test4");
		EXPECT_EQ(finals->IsBestOfThree(), true);



		auto a1 = std::make_shared<AgeGroup>("age 1", 10, 20, nullptr);
		auto a2 = std::make_shared<AgeGroup>("age 2", 30, 40, nullptr);
		auto r  = std::make_shared<RuleSet>("rules", 30, 60, 40, 30);

		app.GetTournament()->AddAgeGroup(a1);
		app.GetTournament()->AddAgeGroup(a2);
		app.GetTournament()->AddRuleSet(r);


		EXPECT_TRUE(app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID(), "name=Test3&fight_system=4&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true&age_group=" + (std::string)a1->GetUUID())));

		ASSERT_EQ(tables.size(), 1);
		EXPECT_EQ(tables[0]->GetName(), "Test3");
		EXPECT_EQ(tables[0]->GetMatID(), 5);
		ASSERT_TRUE(tables[0]->GetAgeGroup());
		EXPECT_EQ(*tables[0]->GetAgeGroup(), *a1);

		EXPECT_TRUE(app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID(), "name=Test9&fight_system=4&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true&age_group=" + (std::string)a2->GetUUID() + "&rule=" + (std::string)r->GetUUID())));

		ASSERT_EQ(tables.size(), 1);
		EXPECT_EQ(tables[0]->GetName(), "Test9");
		EXPECT_EQ(tables[0]->GetMatID(), 5);
		ASSERT_TRUE(tables[0]->GetAgeGroup());
		EXPECT_EQ(*tables[0]->GetAgeGroup(), *a2);
		ASSERT_TRUE(tables[0]->GetRuleSet());
		EXPECT_EQ(tables[0]->GetRuleSet(), *r);
	}
}



TEST(Ajax, MatchTable_Move)
{
	initialize();

	{
		Application app;

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_TRUE( app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test&mat=7")) );

		ASSERT_EQ(tables.size(), 1);
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::RoundRobin);
		EXPECT_EQ(tables[0]->GetName(), "Test");
		EXPECT_EQ(tables[0]->GetMatID(), 7);


		for (int i = 0; i < 100; i++)
		{
			int mat   = rand()%10 + 1;
			int index = rand()%100;

			EXPECT_TRUE(app.Ajax_MoveMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID() + "&schedule_index=" + std::to_string(index) + "&mat=" + std::to_string(mat))) );

			EXPECT_EQ(tables[0]->GetMatID(), mat);
			EXPECT_EQ(tables[0]->GetScheduleIndex(), index);
		}
	}
}



TEST(Ajax, MatchTable_Move2)
{
	initialize();

	{
		Application app;

		auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
		app.GetTournament()->AddParticipant(j1);

		auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
		app.GetTournament()->AddParticipant(j2);

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_TRUE( app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test1&mat=1&minWeight=40&maxWeight=70")) );
		EXPECT_TRUE( app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test2&mat=1&minWeight=40&maxWeight=70")) );

		ASSERT_EQ(tables.size(), 2);
		EXPECT_EQ(tables[0]->GetName(), "Test1");
		EXPECT_EQ(tables[1]->GetName(), "Test2");

		auto schedule = app.GetTournament()->GetSchedule();
		EXPECT_EQ(schedule[0]->GetMatchTable()->GetName(), "Test1");

		EXPECT_TRUE(app.Ajax_MoveMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID() + "&schedule_index=3") ));

		schedule = app.GetTournament()->GetSchedule();
		EXPECT_EQ(schedule[0]->GetMatchTable()->GetName(), "Test2");
	}
}



TEST(Ajax, MatchTable_MoveWithOneFinished)
{
	initialize();

	{
		Application app;

		auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
		app.GetTournament()->AddParticipant(j1);
		auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
		app.GetTournament()->AddParticipant(j2);
		auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
		app.GetTournament()->AddParticipant(j3);

		auto j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
		app.GetTournament()->AddParticipant(j4);
		auto j5 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
		app.GetTournament()->AddParticipant(j5);
		auto j6 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
		app.GetTournament()->AddParticipant(j6);

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_TRUE( app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test1&mat=1&minWeight=40&maxWeight=70")) );
		EXPECT_TRUE( app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test2&mat=1&minWeight=40&maxWeight=70")) );

		ASSERT_EQ(tables.size(), 2);
		EXPECT_EQ(tables[0]->GetName(), "Test1");
		EXPECT_EQ(tables[1]->GetName(), "Test2");

		auto schedule = app.GetTournament()->GetSchedule();
		EXPECT_EQ(schedule[0]->GetMatchTable()->GetName(), "Test1");

		auto mat = app.StartLocalMat();
		EXPECT_TRUE(mat->StartMatch(app.GetTournament()->GetNextMatch()));
		mat->AddIppon(Fighter::White);
		EXPECT_TRUE(mat->EndMatch());

		EXPECT_TRUE(app.Ajax_MoveMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID() + "&schedule_index=3") ));

		schedule = app.GetTournament()->GetSchedule();
		EXPECT_EQ(schedule[0]->GetMatchTable()->GetName(), "Test2");
	}
}



TEST(Ajax, MatchTable_MoveAll)
{
	initialize();

	{
		Application app;

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_TRUE( app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test&mat=7")) );
		EXPECT_TRUE( app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test&mat=3")) );
		EXPECT_TRUE( app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test&mat=2")) );

		ASSERT_EQ(tables.size(), 3);


		for (int i = 0; i < 100; i++)
		{
			int mat = rand()%10 + 1;

			EXPECT_TRUE(app.Ajax_MoveAllMatchTables(HttpServer::Request("mat=" + std::to_string(mat))));

			EXPECT_EQ(tables[0]->GetMatID(), mat);
			EXPECT_EQ(tables[1]->GetMatID(), mat);
			EXPECT_EQ(tables[2]->GetMatID(), mat);
		}
	}
}



TEST(Ajax, MatchTable_DistributeEvenly)
{
	initialize();

	{
		Application app;

		auto& tables = app.GetTournament()->GetMatchTables();

		for (int i = 0; i < 100; i++)
			app.GetTournament()->AddMatchTable(std::make_shared<RoundRobin>(10, 50));

		for (int i = 0; i < 10; i++)
		{
			int mats = rand() % 10 + 1;
			int sim  = rand() % 10 + 1;
			EXPECT_TRUE(app.Ajax_DistributeMatchTablesEvenly(HttpServer::Request("mats=" + std::to_string(mats) + "&simultaneous=" + std::to_string(sim))) );

			int data[20][100];//mats, schedule_index
			for (int i = 0; i < 20; i++)
				for (int j = 0; j < 100; j++)
					data[i][j] = 0;

			for (auto table : tables)
				data[table->GetMatID()][table->GetScheduleIndex()]++;
			
			for (int i = 1; i < 20; i++)
				for (int j = 0; j < 100; j++)
			{
				if (i > mats)
					EXPECT_EQ(data[i][j], 0);
				else
				{
					EXPECT_LE(data[i][j], sim);

					if (j == 0)
						EXPECT_EQ(data[i][j], sim);
				}
			}
		}
	}
}



TEST(Ajax, MatchTable_Get)
{
	initialize();

	{
		Application app;

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test&mat=7&minWeight=1,7&maxWeight=2.3")));

		ASSERT_EQ(tables.size(), 1);

		auto output = app.Ajax_GetMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID()));

		YAML::Emitter yaml1;
		yaml1 << YAML::BeginMap;
		tables[0]->ToString(yaml1);
		yaml1 << YAML::EndMap;
		EXPECT_EQ(yaml1.c_str(), output);


		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test2&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")));

		ASSERT_EQ(tables.size(), 2);

		output = app.Ajax_GetMatchTable(HttpServer::Request("id=" + (std::string)tables[1]->GetUUID()));

		YAML::Emitter yaml2;
		yaml2 << YAML::BeginMap;
		tables[1]->ToString(yaml2);
		yaml2 << YAML::EndMap;
		EXPECT_EQ(yaml2.c_str(), output);

		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=3&name=Test3&mat=5&minWeight=10,8&maxWeight=20.4&gender=1&bo3=true&mf3=true&mf5=true")));

		ASSERT_EQ(tables.size(), 3);

		output = app.Ajax_GetMatchTable(HttpServer::Request("id=" + (std::string)tables[2]->GetUUID()));

		YAML::Emitter yaml3;
		yaml3 << YAML::BeginMap;
		tables[2]->ToString(yaml3);
		yaml3 << YAML::EndMap;
		EXPECT_EQ(yaml3.c_str(), output);

		auto node = YAML::Load(output);

		EXPECT_EQ(node["name"].as<std::string>(), "Test3");

		ASSERT_TRUE(node["best_of_three"].IsDefined());
		ASSERT_TRUE(node["third_place"].IsDefined());
		ASSERT_TRUE(node["fifth_place"].IsDefined());

		EXPECT_EQ(node["best_of_three"].as<bool>(), true);
		EXPECT_EQ(node["third_place"].as<bool>(), true);
		EXPECT_EQ(node["fifth_place"].as<bool>(), true);


		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=4&name=Test4&mat=5&minWeight=10,9&maxWeight=20.3&gender=1&bo3=true&mf3=true&mf5=true")), "ok");

		ASSERT_EQ(tables.size(), 4);

		output = app.Ajax_GetMatchTable(HttpServer::Request("id=" + (std::string)tables[3]->GetUUID()));

		YAML::Emitter yaml4;
		yaml4 << YAML::BeginMap;
		tables[3]->ToString(yaml4);
		yaml4 << YAML::EndMap;
		EXPECT_EQ(yaml4.c_str(), output);

		node = YAML::Load(output);

		ASSERT_EQ(tables[3]->GetType(), MatchTable::Type::Pool);
		auto pool = std::dynamic_pointer_cast<Pool>(tables[3]);

		output = app.Ajax_GetMatchTable(HttpServer::Request("id=" + (std::string)pool->GetFinals()->GetUUID()));

		YAML::Emitter yaml5;
		yaml5 << YAML::BeginMap;
		pool->GetFinals()->ToString(yaml5);
		yaml5 << YAML::EndMap;
		EXPECT_EQ(yaml5.c_str(), output);
	}
}



TEST(Ajax, MatchTable_StartPositionsAfterUpdate)
{
	initialize();

	{
		Application app;

		
		for (int i = 0; i < 100; i++)
		{
			auto t = new Tournament("deleteMe" + std::to_string(i));
			t->EnableAutoSave(false);

			ASSERT_TRUE(app.AddTournament(t));

			auto group = std::make_shared<SingleElimination>(Weight(0), Weight(200));
			t->AddMatchTable(group);

			auto j1 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50);
			t->AddParticipant(j1);

			auto j2 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 60);
			t->AddParticipant(j2);

			auto j3 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 70);
			t->AddParticipant(j3);

			auto j4 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 80);
			t->AddParticipant(j4);

			auto j5 = new Judoka(GetFakeFirstname(), GetFakeLastname(), 90);
			t->AddParticipant(j5);


			size_t start_j1 = rand() % 8;
			size_t start_j2 = rand() % 8;
			size_t start_j3 = rand() % 8;
			size_t start_j4 = rand() % 8;
			size_t start_j5 = rand() % 8;

			group->SetStartPosition(j1, start_j1);
			group->SetStartPosition(j2, start_j2);
			group->SetStartPosition(j3, start_j3);
			group->SetStartPosition(j4, start_j4);
			group->SetStartPosition(j5, start_j5);

			start_j1 = group->GetStartPosition(j1);
			start_j2 = group->GetStartPosition(j2);
			start_j3 = group->GetStartPosition(j3);
			start_j4 = group->GetStartPosition(j4);
			start_j5 = group->GetStartPosition(j5);

			EXPECT_TRUE(app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)group->GetUUID(), "type=1&fight_system=3&name=Test2&mat=5&minWeight=0,7&maxWeight=200.3&bo3=true")));


			ASSERT_EQ(group->GetStartPosition(j1), start_j1);
			ASSERT_EQ(group->GetStartPosition(j2), start_j2);
			ASSERT_EQ(group->GetStartPosition(j3), start_j3);
			ASSERT_EQ(group->GetStartPosition(j4), start_j4);
			ASSERT_EQ(group->GetStartPosition(j5), start_j5);

			app.CloseTournament();
		}
	}
}



TEST(Ajax, NoDisqualification)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		auto match = std::make_shared<Match>(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		mat->StartMatch(match);

		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsUnknownDisqualification());

		app.Ajax_NoDisqualification(f, HttpServer::Request("id=1"));

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsUnknownDisqualification());
	}
}



TEST(Ajax, RemoveNoDisqualification)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		auto match = std::make_shared<Match>(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		EXPECT_TRUE(mat->StartMatch(match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsUnknownDisqualification());

		app.Ajax_NoDisqualification(f, HttpServer::Request("id=1"));

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsUnknownDisqualification());

		app.Ajax_RemoveNoDisqualification(f, HttpServer::Request("id=1"));

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsUnknownDisqualification());
	}
}



TEST(Ajax, ReviseMatch)
{
	initialize();
	
	Judoka* j1 = new Judoka("Firstname",  "Lastname",  50, Gender::Male);
	Judoka* j2 = new Judoka("Firstname2", "Lastname2", 50, Gender::Male);

	for (Fighter f = Fighter::White; f <= Fighter::Blue; ++f)
	{
		Application app;

		auto tourney = app.GetTournament();

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		auto match = std::make_shared<Match>(j1, j2, tourney, 1);
		tourney->AddMatch(match);

		EXPECT_TRUE(mat->StartMatch(match));
		ZED::Core::Pause(200);

		mat->Hajime();
		ZED::Core::Pause(1000);
		mat->AddIppon(f);

		EXPECT_TRUE(mat->EndMatch());
		EXPECT_TRUE(match->HasConcluded());
		EXPECT_EQ(match->GetResult().m_Winner, f);

		EXPECT_TRUE(app.Ajax_ReviseMatch(HttpServer::Request("id=" + (std::string)match->GetUUID())));

		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->RemoveIppon(f);
		mat->Hajime();
		ZED::Core::Pause(1000);
		mat->AddIppon(!f);

		EXPECT_TRUE(mat->EndMatch());
		EXPECT_TRUE(match->HasConcluded());
		EXPECT_EQ(match->GetResult().m_Winner, !f);
		EXPECT_LE(std::abs((int)match->GetResult().m_Time - 2000), 50);
	}
}



TEST(Ajax, GetParticipantsFromMatchTable)
{
	initialize();

	Application app;

	auto j1 = new Judoka(GetRandomName(), GetRandomName(), 50);
	auto j2 = new Judoka(GetRandomName(), GetRandomName(), 50);
	auto j3 = new Judoka(GetRandomName(), GetRandomName(), 50);

	app.GetTournament()->AddParticipant(j1);
	//app.GetTournament()->AddParticipant(j2);
	//app.GetTournament()->AddParticipant(j3);

	auto table = std::make_shared<RoundRobin>(Weight(10), Weight(100));
	app.GetTournament()->AddMatchTable(table);

	YAML::Node yaml = YAML::Load(app.Ajax_GetParticipantsFromMatchTable(HttpServer::Request("id=" + (std::string)table->GetUUID())));

	ASSERT_EQ(yaml.size(), 1);
	EXPECT_EQ(j1->GetUUID(), yaml[0]["uuid"].as<std::string>());
	EXPECT_EQ(j1->GetFirstname(), yaml[0]["firstname"].as<std::string>());
	EXPECT_EQ(j1->GetLastname(),  yaml[0]["lastname" ].as<std::string>());

	//Changed to only 1 judoka since there is no guarantee that the judoka will come out in the same order

	/*result >> id >> name;
	EXPECT_EQ(j2->GetID(), id);
	EXPECT_EQ(j2->GetName(), name);

	result >> id >> name;
	EXPECT_EQ(j3->GetID(), id);
	EXPECT_EQ(j3->GetName(), name);*/
}



TEST(Ajax, GetMatchesFromMatchTable)
{
	initialize();

	Application app;

	auto j1 = new Judoka(GetRandomName(), GetRandomName(), 50);
	auto j2 = new Judoka(GetRandomName(), GetRandomName(), 50);
	auto j3 = new Judoka(GetRandomName(), GetRandomName(), 50);

	app.GetTournament()->AddParticipant(j1);
	app.GetTournament()->AddParticipant(j2);
	app.GetTournament()->AddParticipant(j3);

	auto table = std::make_shared<RoundRobin>(Weight(10), Weight(100));
	app.GetTournament()->AddMatchTable(table);

	YAML::Node yaml = YAML::Load(app.Ajax_GetMatchesFromMatchTable(HttpServer::Request("id=" + (std::string)table->GetUUID())));

	ASSERT_EQ(yaml.size(), 3);

	int i = 0;
	for (const auto& node : yaml)
	{
		EXPECT_EQ(node["uuid"].as<std::string>(),       (std::string)table->GetSchedule()[i]->GetUUID());
		EXPECT_EQ(node["white_name"].as<std::string>(), table->GetSchedule()[i]->GetFighter(Fighter::White)->GetName(NameStyle::GivenName));
		EXPECT_EQ(node["blue_name"].as<std::string>(),  table->GetSchedule()[i]->GetFighter(Fighter::Blue )->GetName(NameStyle::GivenName));
		EXPECT_EQ(node["mat_id"].as<int>(),             table->GetSchedule()[i]->GetMatID());
		i++;
	}
}



TEST(Ajax, SetStartPosition)
{
	initialize();

	Application app;

	auto j1 = new Judoka(GetRandomName(), GetRandomName(), 50);
	auto j2 = new Judoka(GetRandomName(), GetRandomName(), 50);
	auto j3 = new Judoka(GetRandomName(), GetRandomName(), 50);

	app.GetTournament()->AddParticipant(j1);
	app.GetTournament()->AddParticipant(j2);
	app.GetTournament()->AddParticipant(j3);

	auto table = std::make_shared<SingleElimination>(Weight(10), Weight(100));
	app.GetTournament()->AddMatchTable(table);

	for (int i = 0; i < 100; ++i)
	{
		int startpos = rand() % 4;
		EXPECT_EQ((std::string)app.Ajax_SetStartPosition(HttpServer::Request( "id=" + (std::string)table->GetUUID() + "&judoka=" + (std::string)j1->GetUUID() + "&startpos=" + std::to_string(startpos) )), "ok");

		EXPECT_EQ(table->GetStartPosition(j1), startpos);
	}

	for (int i = 0; i < 100; ++i)
	{
		int startpos = rand() % 4;
		EXPECT_EQ((std::string)app.Ajax_SetStartPosition(HttpServer::Request( "id=" + (std::string)table->GetUUID() + "&judoka=" + (std::string)j2->GetUUID() + "&startpos=" + std::to_string(startpos) )), "ok");

		EXPECT_EQ(table->GetStartPosition(j2), startpos);
	}
}



TEST(Ajax, MoveSchedule)
{
	initialize();

	Application app;

	Judoka j1("Firstname", "Lastname", 50, Gender::Male);
	Judoka j2("Firstname", "Lastname", 50, Gender::Male);

	Tournament* tourney = new Tournament("deleteMe");
	tourney->Reset();
	tourney->EnableAutoSave(false);

	auto match1 = std::make_shared<Match>(&j1, &j2, tourney, 1);
	auto match2 = std::make_shared<Match>(&j1, &j2, tourney, 1);
	auto match3 = std::make_shared<Match>(&j1, &j2, tourney, 1);
	auto match4 = std::make_shared<Match>(&j1, &j2, tourney, 2);
	auto match5 = std::make_shared<Match>(&j1, &j2, tourney, 2);
	auto match6 = std::make_shared<Match>(&j1, &j2, tourney, 2);

	tourney->AddMatch(match1);
	tourney->AddMatch(match2);
	tourney->AddMatch(match3);
	tourney->AddMatch(match4);
	tourney->AddMatch(match5);
	tourney->AddMatch(match6);

	app.AddTournament(tourney);

	EXPECT_EQ(*tourney->GetSchedule()[0], *match1);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match2);

	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match1->GetUUID())), Error::Type::OperationFailed);
	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match2->GetUUID())), Error::Type::NoError);

	EXPECT_EQ(*tourney->GetSchedule()[0], *match2);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match1);

	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match4->GetUUID())), Error::Type::NoError);
	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match4->GetUUID())), Error::Type::NoError);
	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match4->GetUUID())), Error::Type::NoError);
	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match4->GetUUID())), Error::Type::OperationFailed);

	EXPECT_EQ(*tourney->GetSchedule()[0], *match4);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match2);
	EXPECT_EQ(*tourney->GetSchedule()[2], *match1);
	EXPECT_EQ(*tourney->GetSchedule()[3], *match3);
	EXPECT_EQ(*tourney->GetSchedule()[4], *match5);
	EXPECT_EQ(*tourney->GetSchedule()[5], *match6);

	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match5->GetUUID() + "&mat=2")), Error::Type::NoError);
	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match5->GetUUID() + "&mat=5")), Error::Type::OperationFailed);

	EXPECT_EQ(*tourney->GetSchedule()[0], *match5);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match2);
	EXPECT_EQ(*tourney->GetSchedule()[2], *match1);
	EXPECT_EQ(*tourney->GetSchedule()[3], *match3);
	EXPECT_EQ(*tourney->GetSchedule()[4], *match4);
	EXPECT_EQ(*tourney->GetSchedule()[5], *match6);

	EXPECT_EQ(app.Ajax_MoveMatchDown(HttpServer::Request("id=" + (std::string)match6->GetUUID())), Error::Type::OperationFailed);

	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match4->GetUUID())), Error::Type::NoError);
	EXPECT_EQ(app.Ajax_MoveMatchUp(HttpServer::Request("id=" + (std::string)match4->GetUUID())), Error::Type::NoError);

	EXPECT_EQ(*tourney->GetSchedule()[0], *match5);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match2);
	EXPECT_EQ(*tourney->GetSchedule()[2], *match4);
	EXPECT_EQ(*tourney->GetSchedule()[3], *match1);
	EXPECT_EQ(*tourney->GetSchedule()[4], *match3);
	EXPECT_EQ(*tourney->GetSchedule()[5], *match6);

	EXPECT_EQ(app.Ajax_MoveMatchDown(HttpServer::Request("id=" + (std::string)match4->GetUUID() + "&mat=2")), Error::Type::NoError);

	EXPECT_EQ(*tourney->GetSchedule()[0], *match5);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match2);
	EXPECT_EQ(*tourney->GetSchedule()[2], *match6);
	EXPECT_EQ(*tourney->GetSchedule()[3], *match1);
	EXPECT_EQ(*tourney->GetSchedule()[4], *match3);
	EXPECT_EQ(*tourney->GetSchedule()[5], *match4);

	EXPECT_TRUE(app.Ajax_MoveMatchTo(HttpServer::Request("from=" + (std::string)match3->GetUUID() + "&to=" + (std::string)match6->GetUUID() + "&position=above")));

	EXPECT_EQ(*tourney->GetSchedule()[0], *match5);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match2);
	EXPECT_EQ(*tourney->GetSchedule()[2], *match3);
	EXPECT_EQ(*tourney->GetSchedule()[3], *match6);
	EXPECT_EQ(*tourney->GetSchedule()[4], *match1);
	EXPECT_EQ(*tourney->GetSchedule()[5], *match4);

	EXPECT_TRUE(app.Ajax_MoveMatchTo(HttpServer::Request("from=" + (std::string)match2->GetUUID() + "&to=" + (std::string)match1->GetUUID() + "&position=below")));

	EXPECT_EQ(*tourney->GetSchedule()[0], *match5);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match3);
	EXPECT_EQ(*tourney->GetSchedule()[2], *match6);
	EXPECT_EQ(*tourney->GetSchedule()[3], *match1);
	EXPECT_EQ(*tourney->GetSchedule()[4], *match2);
	EXPECT_EQ(*tourney->GetSchedule()[5], *match4);

	EXPECT_TRUE(app.Ajax_MoveMatchTo(HttpServer::Request("from=" + (std::string)match5->GetUUID() + "&to=" + (std::string)match4->GetUUID() + "&position=below")));

	EXPECT_EQ(*tourney->GetSchedule()[0], *match3);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match6);
	EXPECT_EQ(*tourney->GetSchedule()[2], *match1);
	EXPECT_EQ(*tourney->GetSchedule()[3], *match2);
	EXPECT_EQ(*tourney->GetSchedule()[4], *match4);
	EXPECT_EQ(*tourney->GetSchedule()[5], *match5);

	EXPECT_TRUE(app.Ajax_MoveMatchTo(HttpServer::Request("from=" + (std::string)match4->GetUUID() + "&to=" + (std::string)match3->GetUUID() + "&position=above")));

	EXPECT_EQ(*tourney->GetSchedule()[0], *match4);
	EXPECT_EQ(*tourney->GetSchedule()[1], *match3);
	EXPECT_EQ(*tourney->GetSchedule()[2], *match6);
	EXPECT_EQ(*tourney->GetSchedule()[3], *match1);
	EXPECT_EQ(*tourney->GetSchedule()[4], *match2);
	EXPECT_EQ(*tourney->GetSchedule()[5], *match5);
}



TEST(Ajax, Tournament_Add)
{
	initialize();

	Application app;

	auto rules = std::make_shared<RuleSet>("Test Rules", 100, 100, 20, 10);
	app.GetDatabase().AddRuleSet(rules);

	auto assoc = std::make_shared<Association>("Organizer", nullptr);
	app.GetDatabase().AddAssociation(assoc);

	EXPECT_TRUE(app.Ajax_AddTournament(HttpServer::Request("", "name=test&year=2000&rules=" + (std::string)rules->GetUUID() + "&organizer=" + (std::string)assoc->GetUUID() + "&description=test description")));

	auto tour = app.FindTournamentByName("test");
	ASSERT_TRUE(tour);
	ASSERT_TRUE(tour->GetDefaultRuleSet());
	EXPECT_EQ(*tour->GetDefaultRuleSet(), *rules);
	EXPECT_EQ(tour->GetDatabase().GetYear(), 2000);
	ASSERT_TRUE(tour->GetOrganizer());
	EXPECT_EQ(*tour->GetOrganizer(), *assoc);
	EXPECT_EQ(tour->GetDescription(), "test description");
}



TEST(Ajax, Tournament_Get)
{
	initialize();

	Application app;

	auto rules = std::make_shared<RuleSet>("Test Rules", 100, 100, 20, 10);
	app.GetDatabase().AddRuleSet(rules);

	auto assoc = std::make_shared<Association>("Organizer", nullptr);
	app.GetDatabase().AddAssociation(assoc);

	EXPECT_TRUE(app.Ajax_AddTournament(HttpServer::Request("", "name=test&year=2000&rules=" + (std::string)rules->GetUUID() + "&organizer=" + (std::string)assoc->GetUUID())));

	auto tour = app.FindTournamentByName("test");
	ASSERT_TRUE(tour);

	tour->SetDescription("test desc");

	YAML::Node yaml = YAML::Load( app.Ajax_GetTournament(HttpServer::Request("id=" + (std::string)tour->GetUUID()) ) );

	EXPECT_EQ(yaml["name"].as<std::string>(), tour->GetName());
	EXPECT_EQ(yaml["rule_set_uuid"].as<std::string>(),  *rules);
	EXPECT_EQ(yaml["organizer_uuid"].as<std::string>(), *assoc);
	EXPECT_EQ(yaml["description"].as<std::string>(), tour->GetDescription());
	EXPECT_EQ(yaml["is_locked"].as<bool>(), false);
}



TEST(Ajax, Tournament_List)
{
	initialize();

	Application app;

	EXPECT_TRUE(app.Ajax_AddTournament(HttpServer::Request("", "name=test1&year=2000")));
	EXPECT_TRUE(app.Ajax_AddTournament(HttpServer::Request("", "name=test2&year=2001")));

	auto tour1 = app.FindTournamentByName("test1");
	auto tour2 = app.FindTournamentByName("test2");
	ASSERT_TRUE(tour1);
	ASSERT_TRUE(tour2);

	YAML::Node yaml = YAML::Load( app.Ajax_ListTournaments() );

	EXPECT_EQ(yaml[0]["name"].as<std::string>(), "test1");
	EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), tour1->GetUUID());
	EXPECT_EQ(yaml[0]["is_locked"].as<bool>(), false);

	EXPECT_EQ(yaml[1]["name"].as<std::string>(), "test2");
	EXPECT_EQ(yaml[1]["uuid"].as<std::string>(), tour2->GetUUID());
	EXPECT_EQ(yaml[1]["is_locked"].as<bool>(), false);	
}



TEST(Ajax, Tournament_Edit)
{
	initialize();
	ZED::Core::RemoveFile("tournaments/test.yml");

	{
		Application app;

		auto rules1 = std::make_shared<RuleSet>("Test Rules1", 100, 100, 20, 10);
		auto rules2 = std::make_shared<RuleSet>("Test Rules2", 100, 100, 20, 10);
		app.GetDatabase().AddRuleSet(rules1);
		app.GetDatabase().AddRuleSet(rules2);

		auto assoc1 = std::make_shared<Association>("Organizer1", nullptr);
		auto assoc2 = std::make_shared<Association>("Organizer2", nullptr);
		app.GetDatabase().AddAssociation(assoc1);
		app.GetDatabase().AddAssociation(assoc2);

		EXPECT_TRUE(app.Ajax_AddTournament(HttpServer::Request("", "name=test&year=2000&rules=" + (std::string)rules1->GetUUID() + "&organizer=" + (std::string)assoc1->GetUUID())));

		EXPECT_FALSE(app.Ajax_EditTournament(HttpServer::Request("", "name=test2&year=2001&rules=" + (std::string)rules2->GetUUID() + "&organizer=" + (std::string)assoc2->GetUUID())));

		auto tour1 = app.FindTournamentByName("test");
		ASSERT_TRUE(tour1);
		EXPECT_TRUE(tour1->Save());

		EXPECT_FALSE(app.Ajax_EditTournament(HttpServer::Request("id=" + (std::string)tour1->GetUUID(), "name=test2&year=2001&rules=" + (std::string)rules2->GetUUID() + "&organizer=" + (std::string)assoc2->GetUUID())));

		EXPECT_TRUE(app.CloseTournament());

		EXPECT_TRUE(app.Ajax_EditTournament(HttpServer::Request("id=" + (std::string)tour1->GetUUID(), "name=test2&year=2001&readonly=true&rules=" + (std::string)rules2->GetUUID() + "&organizer=" + (std::string)assoc2->GetUUID() + "&description=test description")));

		EXPECT_FALSE(app.FindTournamentByName("test"));

		//Check if the old tournament file still exists
		EXPECT_FALSE(ZED::Core::RemoveFile("tournaments/test.yml"));

		auto tour = app.FindTournamentByName("test2");
		ASSERT_TRUE(tour);
		ASSERT_TRUE(tour->GetDefaultRuleSet());
		EXPECT_EQ(tour->GetDescription(), "test description");
		EXPECT_EQ(*tour->GetDefaultRuleSet(), *rules2);
		EXPECT_EQ(tour->GetDatabase().GetYear(), 2001);
		ASSERT_TRUE(tour->GetOrganizer());
		EXPECT_EQ(*tour->GetOrganizer(), *assoc2);
		EXPECT_EQ(tour->IsReadonly(), true);


		EXPECT_TRUE(app.Ajax_EditTournament(HttpServer::Request("id=" + (std::string)tour1->GetUUID(), "name=test3&year=2000&readonly=false&rules=" + (std::string)rules2->GetUUID() + "&organizer=" + (std::string)assoc2->GetUUID())));

		tour = app.FindTournamentByName("test3");
		ASSERT_TRUE(tour);
		ASSERT_TRUE(tour->GetDefaultRuleSet());
		EXPECT_EQ(*tour->GetDefaultRuleSet(), *rules2);
		EXPECT_EQ(tour->GetDatabase().GetYear(), 2000);
		ASSERT_TRUE(tour->GetOrganizer());
		EXPECT_EQ(*tour->GetOrganizer(), *assoc2);
		EXPECT_EQ(tour->IsReadonly(), false);
	}

	ZED::Core::RemoveFile("tournaments/test.yml");
	ZED::Core::RemoveFile("tournaments/test2.yml");
}



TEST(Ajax, Tournament_DeleteMatchlessTables)
{
	initialize();

	{
		Application app;

		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 50));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 51));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 52));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 60));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 61));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 62));

		auto m1 = std::make_shared<RoundRobin>(Weight(0), Weight(49));
		auto m2 = std::make_shared<RoundRobin>(Weight(50), Weight(55));
		auto m3 = std::make_shared<RoundRobin>(Weight(60), Weight(65));
		auto m4 = std::make_shared<RoundRobin>(Weight(70), Weight(80));
		app.GetTournament()->AddMatchTable(m1);
		app.GetTournament()->AddMatchTable(m2);
		app.GetTournament()->AddMatchTable(m3);
		app.GetTournament()->AddMatchTable(m4);

		EXPECT_EQ(app.GetTournament()->GetMatchTables().size(), 4);

		EXPECT_TRUE(app.Ajax_DeleteMatchlessMatchTables());

		EXPECT_EQ(app.GetTournament()->GetMatchTables().size(), 2);
		EXPECT_EQ(app.GetTournament()->GetMatchTables()[0]->GetNumberOfMatches(), 3);
		EXPECT_EQ(app.GetTournament()->GetMatchTables()[1]->GetNumberOfMatches(), 3);
	}
}



TEST(Ajax, Tournament_DeleteCompletedTables)
{
	initialize();

	{
		Application app;

		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 50));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 51));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 52));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 60));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 61));
		app.GetTournament()->AddParticipant(new Judoka(GetFakeFirstname(), GetFakeLastname(), 62));

		auto m1 = std::make_shared<RoundRobin>(Weight(0), Weight(49));
		auto m2 = std::make_shared<RoundRobin>(Weight(50), Weight(55));
		auto m3 = std::make_shared<RoundRobin>(Weight(60), Weight(65));
		auto m4 = std::make_shared<RoundRobin>(Weight(70), Weight(80));
		app.GetTournament()->AddMatchTable(m1);
		app.GetTournament()->AddMatchTable(m2);
		app.GetTournament()->AddMatchTable(m3);
		app.GetTournament()->AddMatchTable(m4);

		EXPECT_EQ(app.GetTournament()->GetMatchTables().size(), 4);

		EXPECT_TRUE(app.Ajax_DeleteCompletedMatchTables());

		EXPECT_EQ(app.GetTournament()->GetMatchTables().size(), 2);
		EXPECT_EQ(app.GetTournament()->GetMatchTables()[0]->GetNumberOfMatches(), 3);
		EXPECT_EQ(app.GetTournament()->GetMatchTables()[1]->GetNumberOfMatches(), 3);

		app.StartLocalMat();
		auto mat = app.FindMat(1);

		auto match1 = app.GetTournament()->GetNextMatch();
		EXPECT_TRUE(mat->StartMatch(match1));
		mat->AddIppon(Fighter::White);
		EXPECT_TRUE(mat->EndMatch());

		auto match2 = app.GetTournament()->GetNextMatch();
		EXPECT_TRUE(mat->StartMatch(match2));
		mat->AddIppon(Fighter::White);
		EXPECT_TRUE(mat->EndMatch());

		auto match3 = app.GetTournament()->GetNextMatch();
		EXPECT_TRUE(mat->StartMatch(match3));
		mat->AddIppon(Fighter::White);
		EXPECT_TRUE(mat->EndMatch());

		EXPECT_EQ(app.GetTournament()->GetMatchTables().size(), 2);

		EXPECT_TRUE(app.Ajax_DeleteCompletedMatchTables());

		EXPECT_EQ(app.GetTournament()->GetMatchTables().size(), 1);
		EXPECT_EQ(app.GetTournament()->GetMatchTables()[0]->GetNumberOfMatches(), 3);
	}
}



TEST(Ajax, UpdateWeight)
{
	initialize();

	{
		Application app;

		Judoka* j1 = new Judoka("Firstname1", "Lastname1", 40, Gender::Female, 2000);
		Judoka* j2 = new Judoka("Firstname2", "Lastname2", 40, Gender::Female, 2000);
		Judoka* j3 = new Judoka("Firstname1", "Lastname1", 40, Gender::Female, 2000);
		Judoka* j4 = new Judoka("Firstname2", "Lastname2", 40, Gender::Female, 2000);
		Judoka* j5 = new Judoka("Firstname1", "Lastname1", 40, Gender::Female, 2000);
		Judoka* j6 = new Judoka("Firstname2", "Lastname2", 40, Gender::Female, 2000);
		Judoka* j7 = new Judoka("Firstname1", "Lastname1", 40, Gender::Female, 2000);
		Judoka* j8 = new Judoka("Firstname2", "Lastname2", 40, Gender::Female, 2000);

		app.GetTournament()->AddParticipant(j1);
		app.GetTournament()->AddParticipant(j2);
		app.GetTournament()->AddParticipant(j3);
		app.GetTournament()->AddParticipant(j4);
		app.GetTournament()->AddParticipant(j5);
		app.GetTournament()->AddParticipant(j6);
		app.GetTournament()->AddParticipant(j7);
		app.GetTournament()->AddParticipant(j8);

		auto a1 = std::make_shared<AgeGroup>("Age", 100, 200, nullptr);
		app.GetTournament()->AddAgeGroup(a1);

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test1&mat=1&minWeight=20&maxWeight=30")));
		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test2&mat=1&minWeight=20&maxWeight=30&age_group=" + (std::string)a1->GetUUID())));
		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=1&name=Test3&mat=1&minWeight=20&maxWeight=50&gender=0")));

		ASSERT_EQ(tables.size(), 3);
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::RoundRobin);
		EXPECT_EQ(tables[0]->GetName(), "Test1");
		EXPECT_EQ(tables[0]->GetMatID(), 1);
		EXPECT_EQ(tables[1]->GetName(), "Test2");
		EXPECT_EQ(tables[2]->GetName(), "Test3");
		ASSERT_TRUE(tables[0]->GetFilter());



		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id=" + (std::string)j1->GetUUID(), "firstname=a&lastname=b&weight=21&gender=1")));
		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id=" + (std::string)j2->GetUUID(), "firstname=a&lastname=b&weight=22&gender=1")));

		EXPECT_EQ(tables[0]->GetSchedule().size(), 1);

		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id=" + (std::string)j3->GetUUID(), "firstname=a&lastname=b&weight=23&gender=1")));

		EXPECT_EQ(tables[0]->GetSchedule().size(), 3);

		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id=" + (std::string)j4->GetUUID(), "firstname=a&lastname=b&weight=24&gender=1")));

		EXPECT_EQ(tables[0]->GetSchedule().size(), 6);

		EXPECT_EQ(tables[1]->GetSchedule().size(), 0);

		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id=" + (std::string)j5->GetUUID(), "firstname=a&lastname=b&weight=24&gender=1&birthyear=1900")));
		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id=" + (std::string)j6->GetUUID(), "firstname=a&lastname=b&weight=24&gender=1&birthyear=1900")));

		EXPECT_EQ(tables[1]->GetSchedule().size(), 1);

		EXPECT_EQ(tables[2]->GetSchedule().size(), 0);

		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id=" + (std::string)j7->GetUUID(), "firstname=a&lastname=b&weight=40&gender=0")));
		EXPECT_TRUE(app.Ajax_EditJudoka(HttpServer::Request("id=" + (std::string)j8->GetUUID(), "firstname=a&lastname=b&weight=40&gender=0")));

		EXPECT_EQ(tables[2]->GetSchedule().size(), 1);
	}
}