#include "tests.h"



TEST(Ajax, AgeGroup_Get)
{
	initialize();

	{
		Application app;

		auto a1 = new AgeGroup("age 1", 10, 20, nullptr, app.GetDatabase());
		auto a2 = new AgeGroup("age 2", 30, 40, nullptr, app.GetDatabase());

		app.GetDatabase().AddAgeGroup(a1);
		app.GetTournament()->AddAgeGroup(a2);

		YAML::Node yaml = YAML::Load(app.Ajax_GetAgeGroup(HttpServer::Request("id=" + (std::string)a1->GetUUID())));

		EXPECT_EQ(yaml["uuid"].as<std::string>(), a1->GetUUID());
		EXPECT_EQ(yaml["name"].as<std::string>(), a1->GetName());
		EXPECT_EQ(yaml["min_age"].as<int>(), a1->GetMinAge());
		EXPECT_EQ(yaml["max_age"].as<int>(), a1->GetMaxAge());

		yaml = YAML::Load(app.Ajax_GetAgeGroup(HttpServer::Request("id=" + (std::string)a2->GetUUID())));

		EXPECT_EQ(yaml["uuid"].as<std::string>(), a2->GetUUID());
		EXPECT_EQ(yaml["name"].as<std::string>(), a2->GetName());
		EXPECT_EQ(yaml["min_age"].as<int>(), a2->GetMinAge());
		EXPECT_EQ(yaml["max_age"].as<int>(), a2->GetMaxAge());
	}
}



TEST(Ajax, AgeGroup_List)
{
	initialize();

	{
		Application app;

		auto a1 = new AgeGroup("age 1", 10, 20, nullptr, app.GetDatabase());
		auto a2 = new AgeGroup("age 2", 30, 40, nullptr, app.GetDatabase());

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



TEST(Ajax, GetMats)
{
	initialize();

	Application app;

	auto yaml = YAML::Load(app.Ajax_GetMats());

	EXPECT_EQ(yaml["highest_mat_id"].as<int>(), 1);

	app.StartLocalMat(1);

	yaml = YAML::Load(app.Ajax_GetMats());

	EXPECT_EQ(yaml["highest_mat_id"].as<int>(), 2);
	EXPECT_EQ(yaml["mats"][0]["type"].as<int>(), (int)Mat::Type::LocalMat);
	EXPECT_EQ(yaml["mats"][0]["name"].as<std::string>(), "Mat 1");
	EXPECT_EQ(yaml["mats"][0]["ippon_style"].as<int>(), (int)IMat::IpponStyle::DoubleDigit);
	EXPECT_EQ(yaml["mats"][0]["name_style"].as<int>(),  (int)NameStyle::FamilyName);
}



TEST(Ajax, OpenMat)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);
		app.GetDefaultMat()->Close();
		EXPECT_FALSE(app.GetDefaultMat()->IsOpen());

		app.Ajax_OpenMat(HttpServer::Request("id=1"));

		ASSERT_TRUE(app.GetDefaultMat());
		EXPECT_EQ(app.GetDefaultMat()->GetMatID(), 1);
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
	}


	{
		Application app;

		app.StartLocalMat(5);
		app.GetDefaultMat()->Close();

		app.Ajax_OpenMat(HttpServer::Request("id=1"));

		EXPECT_FALSE(app.GetDefaultMat()->IsOpen());

		app.Ajax_OpenMat(HttpServer::Request("id=5"));

		ASSERT_TRUE(app.GetDefaultMat());
		EXPECT_EQ(app.GetDefaultMat()->GetMatID(), 5);
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
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

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());

		app.Ajax_CloseMat(HttpServer::Request("id=1"));

		ASSERT_TRUE(app.GetDefaultMat());
		EXPECT_FALSE(app.GetDefaultMat()->IsOpen());
	}


	{
		Application app;

		app.StartLocalMat(5);

		app.Ajax_OpenMat( HttpServer::Request("id=5"));
		app.Ajax_CloseMat(HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());

		app.Ajax_CloseMat(HttpServer::Request("id=5"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_FALSE(app.GetDefaultMat()->IsOpen());
	}
}



TEST(Ajax, UpdateMat)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());

		app.Ajax_UpdateMat(HttpServer::Request("id=1", "id=5&name=Test&ipponStyle=0&timerStyle=1&nameStyle=0"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_EQ(app.GetDefaultMat()->GetMatID(), 5);
		EXPECT_EQ(app.GetDefaultMat()->GetName(), "Test");
		EXPECT_EQ((int)app.GetDefaultMat()->GetIpponStyle(), 0);
		EXPECT_EQ((int)app.GetDefaultMat()->GetTimerStyle(), 1);
		EXPECT_EQ((int)app.GetDefaultMat()->GetNameStyle(),  0);


		app.Ajax_UpdateMat(HttpServer::Request("id=5", "id=1&name=Test2&ipponStyle=1&timerStyle=2&nameStyle=1"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_EQ(app.GetDefaultMat()->GetMatID(), 1);
		EXPECT_EQ(app.GetDefaultMat()->GetName(), "Test2");
		EXPECT_EQ((int)app.GetDefaultMat()->GetIpponStyle(), 1);
		EXPECT_EQ((int)app.GetDefaultMat()->GetTimerStyle(), 2);
		EXPECT_EQ((int)app.GetDefaultMat()->GetNameStyle(),  1);


		app.Ajax_UpdateMat(HttpServer::Request("id=1", "id=1&name=Test3&ipponStyle=2&timerStyle=0&nameStyle=0"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_EQ(app.GetDefaultMat()->GetMatID(), 1);
		EXPECT_EQ(app.GetDefaultMat()->GetName(), "Test3");
		EXPECT_EQ((int)app.GetDefaultMat()->GetIpponStyle(), 2);
		EXPECT_EQ((int)app.GetDefaultMat()->GetTimerStyle(), 0);
		EXPECT_EQ((int)app.GetDefaultMat()->GetNameStyle(),  0);
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

		auto yaml = YAML::Load(app.Ajax_GetSetup());

		EXPECT_EQ(yaml["language"].as<int>(), (int)Localizer::GetLanguage());
		EXPECT_EQ(yaml["port"].as<int>(), app.GetDatabase().GetServerPort());
		EXPECT_LE(yaml["uptime"].as<uint32_t>(), 100u);
		EXPECT_EQ(yaml["version"].as<std::string>(), Application::Version);

		ZED::Core::Pause(1000);

		yaml = YAML::Load(app.Ajax_GetSetup());

		EXPECT_EQ(yaml["language"].as<int>(), (int)Localizer::GetLanguage());
		EXPECT_EQ(yaml["port"].as<int>(), app.GetDatabase().GetServerPort());
		EXPECT_LE(yaml["uptime"].as<uint32_t>(), 1100u);
		EXPECT_EQ(yaml["version"].as<std::string>(), Application::Version);
	}

}



TEST(Ajax, Setup_Set)
{
	initialize();

	{
		Application app;
		
		EXPECT_EQ(app.Ajax_SetSetup(HttpServer::Request("", "port=1234&language=0&ipponStyle=1&timerStyle=2&nameStyle=0")), "ok");

		auto yaml = YAML::Load(app.Ajax_GetSetup());

		EXPECT_EQ(yaml["language"].as<int>(), 0);
		EXPECT_EQ(yaml["port"].as<int>(), 1234);
		EXPECT_EQ(yaml["ippon_style"].as<int>(), 1);
		EXPECT_EQ(yaml["timer_style"].as<int>(), 2);
		EXPECT_EQ(yaml["name_style"].as<int>(),  0);
		EXPECT_LE(yaml["uptime"].as<uint32_t>(), 100u);
		EXPECT_EQ(yaml["version"].as<std::string>(), Application::Version);

		EXPECT_EQ(app.Ajax_SetSetup(HttpServer::Request("", "port=567&language=1&ipponStyle=0&timerStyle=1&nameStyle=1")), "ok");

		yaml = YAML::Load(app.Ajax_GetSetup());

		EXPECT_EQ(yaml["language"].as<int>(), 1);
		EXPECT_EQ(yaml["port"].as<int>(), 567);
		EXPECT_EQ(yaml["ippon_style"].as<int>(), 0);
		EXPECT_EQ(yaml["timer_style"].as<int>(), 1);
		EXPECT_EQ(yaml["name_style"].as<int>(),  1);
		EXPECT_LE(yaml["uptime"].as<uint32_t>(), 100u);
		EXPECT_EQ(yaml["version"].as<std::string>(), Application::Version);
	}
}



TEST(Ajax, ExecuteCommand)
{
	initialize();

	{
		Application app;

		std::string result = app.Ajax_Execute(HttpServer::Request("cmd=dir"));

		EXPECT_GE(result.length(), 170);

		//TODO?!
	}
}



TEST(Ajax, SetFullscreen)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(1);

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		Mat* mat = (Mat*)app.GetDefaultMat();

		app.Ajax_SetFullscreen(true, HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsFullscreen());
		EXPECT_EQ(app.GetDefaultMat()->GetMatID(), 1);


		app.Ajax_SetFullscreen(false, HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_FALSE(app.GetDefaultMat()->IsFullscreen());
		EXPECT_EQ(app.GetDefaultMat()->GetMatID(), 1);


		app.Ajax_SetFullscreen(true, HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_TRUE(app.GetDefaultMat()->IsFullscreen());
		EXPECT_EQ(app.GetDefaultMat()->GetMatID(), 1);
	}
}



TEST(Ajax, GetHansokumake)
{
	initialize();
	
	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		Match match(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		EXPECT_TRUE(mat->StartMatch(&match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);
		
		YAML::Node ret2 = YAML::Load(app.Ajax_GetHansokumake());

		ASSERT_TRUE(ret2[0]["match"].IsDefined());
		ASSERT_TRUE(ret2[0]["match"].IsMap());
		EXPECT_EQ(ret2[0]["match"]["uuid"].as<std::string>(), (std::string)match.GetUUID());
		EXPECT_EQ(ret2[0]["match"]["white_name"].as<std::string>(), match.GetFighter(Fighter::White)->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["match"]["blue_name"].as<std::string>(),  match.GetFighter(Fighter::Blue )->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["fighter"].as<int>(), (int)f);
		EXPECT_EQ(ret2[0]["disqualification_state"].as<int>(), (int)IMat::Scoreboard::DisqualificationState::Unknown);
	}


	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		Match match(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		EXPECT_TRUE(mat->StartMatch(&match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);
		mat->AddDisqualification(f);

		YAML::Node ret2 = YAML::Load(app.Ajax_GetHansokumake());

		ASSERT_TRUE(ret2[0]["match"].IsDefined());
		ASSERT_TRUE(ret2[0]["match"].IsMap());
		EXPECT_EQ(ret2[0]["match"]["uuid"].as<std::string>(), (std::string)match.GetUUID());
		EXPECT_EQ(ret2[0]["match"]["white_name"].as<std::string>(), match.GetFighter(Fighter::White)->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["match"]["blue_name"].as<std::string>(),  match.GetFighter(Fighter::Blue )->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["fighter"].as<int>(), (int)f);
		EXPECT_EQ(ret2[0]["disqualification_state"].as<int>(), (int)IMat::Scoreboard::DisqualificationState::Disqualified);
	}


	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		Match match(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		EXPECT_TRUE(mat->StartMatch(&match));
		EXPECT_TRUE(mat->AreFightersOnMat());
		mat->AddHansokuMake(f);
		mat->AddNoDisqualification(f);

		YAML::Node ret2 = YAML::Load(app.Ajax_GetHansokumake());

		ASSERT_TRUE(ret2[0]["match"].IsDefined());
		ASSERT_TRUE(ret2[0]["match"].IsMap());
		EXPECT_EQ(ret2[0]["match"]["uuid"].as<std::string>(), (std::string)match.GetUUID());
		EXPECT_EQ(ret2[0]["match"]["white_name"].as<std::string>(), match.GetFighter(Fighter::White)->GetName(NameStyle::GivenName));
		EXPECT_EQ(ret2[0]["match"]["blue_name"].as<std::string>(),  match.GetFighter(Fighter::Blue )->GetName(NameStyle::GivenName));
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

		Match match(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		EXPECT_TRUE(mat->StartMatch(&match));
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
		auto c1 = new Club("Club 1");
		j2->SetClub(c1);
		t->AddParticipant(j2);

		EXPECT_EQ((std::string)app.Ajax_EditJudoka(HttpServer::Request("id="+(std::string)j1->GetUUID(), "firstname=first2&lastname=last2&weight=12,5&gender=1&birthyear=2001&number=A1234&club=" + (std::string)c1->GetUUID())), "ok");

		EXPECT_EQ(j1->GetFirstname(), "first2");
		EXPECT_EQ(j1->GetLastname(),  "last2");
		EXPECT_EQ(j1->GetWeight(),  Weight("12,5"));
		EXPECT_EQ(j1->GetGender(),  Gender::Female);
		EXPECT_EQ(j1->GetBirthyear(), 2001);
		EXPECT_EQ(j1->GetNumber(), "A1234");
		ASSERT_TRUE(j1->GetClub());
		EXPECT_EQ(*j1->GetClub(), *c1);
	}
}



TEST(Ajax, GetNamesOnMat)
{
	initialize();

	{
		Application app;

		app.StartLocalMat(5);
		auto mat = app.GetDefaultMat();
		mat->SetName("mat name");

		Tournament* tourney = new Tournament;

		Match* match1 = new Match(new Judoka("A", "B"), new Judoka("C", "D"), nullptr, 5);
		Match* match2 = new Match(new Judoka("E", "F"), new Judoka("G", "H"), nullptr, 5);
		Match* match3 = new Match(new Judoka("I", "J"), new Judoka("K", "L"), nullptr, 5);

		tourney->AddMatch(match1);
		tourney->AddMatch(match2);
		tourney->AddMatch(match3);

		tourney->GetMatchTables()[0]->SetName("table1");
		tourney->GetMatchTables()[1]->SetName("table2");
		tourney->GetMatchTables()[2]->SetName("table3");

		app.AddTournament(tourney);

		ZED::Core::Pause(1000);

		YAML::Node yaml = YAML::Load( app.Ajax_GetNamesOnMat(HttpServer::Request("id=5")) );

		ASSERT_TRUE(yaml.IsMap());
		ASSERT_TRUE(yaml["white_name"]);
		EXPECT_EQ(yaml["white_name"].as<std::string>(), "- - -");
		EXPECT_EQ(yaml["blue_name" ].as<std::string>(), "- - -");
		EXPECT_EQ(yaml["mat_name"  ].as<std::string>(), "mat name");
		EXPECT_EQ(yaml["match_table_name" ].as<std::string>(), "");
		ASSERT_TRUE(yaml["next_matches"]);
		ASSERT_TRUE(yaml["next_matches"].IsSequence());
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

		ZED::Core::Pause(1000);

		yaml = YAML::Load( app.Ajax_GetNamesOnMat(HttpServer::Request("id=5")) );

		ASSERT_TRUE(yaml.IsMap());
		ASSERT_TRUE(yaml["white_name"]);
		EXPECT_EQ(yaml["white_name"].as<std::string>(), "A B");
		EXPECT_EQ(yaml["blue_name" ].as<std::string>(), "C D");
		EXPECT_EQ(yaml["mat_name"  ].as<std::string>(), "mat name");
		//EXPECT_EQ(yaml["match_table_name" ].as<std::string>(), "table1 Custom");
		ASSERT_TRUE(yaml["next_matches"].IsSequence());
		EXPECT_EQ(yaml["next_matches"][0]["white_name"].as<std::string>(), "E F");
		EXPECT_EQ(yaml["next_matches"][0]["blue_name" ].as<std::string>(), "G H");
		EXPECT_EQ(yaml["next_matches"][1]["white_name"].as<std::string>(), "I J");
		EXPECT_EQ(yaml["next_matches"][1]["blue_name" ].as<std::string>(), "K L");

		EXPECT_EQ(yaml["next_matches"][0]["uuid"].as<std::string>(), match2->GetUUID());
		EXPECT_EQ(yaml["next_matches"][1]["uuid"].as<std::string>(), match3->GetUUID());

		mat->AddIppon(Fighter::White);
		mat->EndMatch();

		ZED::Core::Pause(1000);

		yaml = YAML::Load( app.Ajax_GetNamesOnMat(HttpServer::Request("id=5")) );

		ASSERT_TRUE(yaml.IsMap());
		ASSERT_TRUE(yaml["white_name"]);
		EXPECT_EQ(yaml["white_name"].as<std::string>(), "- - -");
		EXPECT_EQ(yaml["blue_name" ].as<std::string>(), "- - -");
		EXPECT_EQ(yaml["mat_name"  ].as<std::string>(), "mat name");
		EXPECT_EQ(yaml["match_table_name" ].as<std::string>(), "");
		ASSERT_TRUE(yaml["next_matches"].IsSequence());
		EXPECT_EQ(yaml["next_matches"][0]["white_name"].as<std::string>(), "E F");
		EXPECT_EQ(yaml["next_matches"][0]["blue_name" ].as<std::string>(), "G H");
		EXPECT_EQ(yaml["next_matches"][1]["white_name"].as<std::string>(), "I J");
		EXPECT_EQ(yaml["next_matches"][1]["blue_name" ].as<std::string>(), "K L");

		mat->StartMatch(match2);

		ZED::Core::Pause(1000);

		yaml = YAML::Load( app.Ajax_GetNamesOnMat(HttpServer::Request("id=5")) );

		ASSERT_TRUE(yaml.IsMap());
		ASSERT_TRUE(yaml["white_name"]);
		EXPECT_EQ(yaml["white_name"].as<std::string>(), "E F");
		EXPECT_EQ(yaml["blue_name" ].as<std::string>(), "G H");
		EXPECT_EQ(yaml["mat_name"  ].as<std::string>(), "mat name");
		//EXPECT_EQ(yaml["match_table_name" ].as<std::string>(), "table2 Custom");
		ASSERT_TRUE(yaml["next_matches"].IsSequence());
		EXPECT_EQ(yaml["next_matches"][0]["white_name"].as<std::string>(), "I J");
		EXPECT_EQ(yaml["next_matches"][0]["blue_name" ].as<std::string>(), "K L");

		mat->AddIppon(Fighter::White);
		mat->EndMatch();

		ZED::Core::Pause(1000);

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


		auto inter = new Judoboard::Association("International", nullptr);

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

		auto inter = new Judoboard::Association("International", nullptr);

		auto de = new Judoboard::Association("Deutschland", inter);

		auto dn = new Judoboard::Association("Deutschland-Nord", de);
		auto ds = new Judoboard::Association(u8"Deutschland-S\u00fcd", de);

		auto nord  = new Judoboard::Association("Nord", dn);
		auto west  = new Judoboard::Association("West", dn);
		auto nost  = new Judoboard::Association("Nordost", dn);
		auto sued  = new Judoboard::Association(u8"S\u00fcd", ds);
		auto swest = new Judoboard::Association(u8"S\u00fcdwest", ds);

		auto nieder  = new Judoboard::Association("Niedersachsen", nord);
		auto hamburg = new Judoboard::Association("Hamburg", nord);
		auto berlin  = new Judoboard::Association("Berlin", nost);
		auto nrw     = new Judoboard::Association("Nordrhein-Westfalen", west);

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

		auto inter = new Judoboard::Association("International", nullptr);

		auto de = new Judoboard::Association("Deutschland", inter);

		auto dn = new Judoboard::Association("Deutschland-Nord", de);
		auto ds = new Judoboard::Association(u8"Deutschland-S\u00fcd", de);

		auto nord  = new Judoboard::Association("Nord", dn);
		auto west  = new Judoboard::Association("West", dn);
		auto nost  = new Judoboard::Association("Nordost", dn);
		auto sued  = new Judoboard::Association(u8"S\u00fcd", ds);
		auto swest = new Judoboard::Association(u8"S\u00fcdwest", ds);

		auto nieder  = new Judoboard::Association("Niedersachsen", nord);
		auto hamburg = new Judoboard::Association("Hamburg", nord);
		auto berlin  = new Judoboard::Association("Berlin", nost);
		auto nrw     = new Judoboard::Association("Nordrhein-Westfalen", west);

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

		auto c1 = new Club("Club 1");
		c1->SetShortName("c1");
		app.GetDatabase().AddClub(c1);

		YAML::Node yaml = YAML::Load(app.Ajax_ListClubs(HttpServer::Request("")));

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Club 1");


		app.GetDatabase().AddClub(new Club("Club 2"));

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

		auto c = new Club("Club 1");
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

		auto c = new Club("Club 1");
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

		auto c = new Club("Club 1");
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

		auto c = new Club("Club 1");
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

		auto c = new Club("Club 1");
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

		auto assoc1 = new Association("Assoc 1", nullptr);
		assoc1->SetShortName("assoc1");
		app.GetDatabase().AddAssociation(assoc1);

		YAML::Node yaml = YAML::Load(app.Ajax_ListAssociations(HttpServer::Request("")));

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "assoc1");
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)assoc1->GetUUID());


		auto assoc2 = new Association("Assoc 2", assoc1);
		assoc2->SetShortName("assoc2");
		app.GetDatabase().AddAssociation(assoc2);

		auto club1 = new Club("Club 1", assoc1);
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

			Association* assoc = new Association(GetRandomName(), nullptr);
			Club* c1 = new Club(GetRandomName());
			c1->SetParent(assoc);
			Club* c2 = new Club(GetRandomName());
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

		auto inter = new Judoboard::Association("International", nullptr);

		auto de = new Judoboard::Association("Deutschland", inter);

		auto dn = new Judoboard::Association("Deutschland-Nord", de);
		auto ds = new Judoboard::Association(u8"Deutschland-S\u00fcd", de);

		auto nord  = new Judoboard::Association("Nord", dn);
		auto west  = new Judoboard::Association("West", dn);
		auto nost  = new Judoboard::Association("Nordost", dn);
		auto sued  = new Judoboard::Association(u8"S\u00fcd", ds);
		auto swest = new Judoboard::Association(u8"S\u00fcdwest", ds);

		auto nieder  = new Judoboard::Association("Niedersachsen", nord);
		auto hamburg = new Judoboard::Association("Hamburg", nord);
		auto berlin  = new Judoboard::Association("Berlin", nost);
		auto nrw     = new Judoboard::Association("Nordrhein-Westfalen", west);

		auto detmold = new Judoboard::Association("Detmold", nrw);

		auto biegue = new Judoboard::Association(u8"Bielefeld/G\u00fctersloh", detmold);

		Club* c1 = new Club("club1", biegue);
		Club* c2 = new Club("club2", biegue);

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

		Association* assoc = new Association(GetRandomName(), nullptr);
		Club* c1 = new Club(GetRandomName());
		c1->SetParent(assoc);
		Club* c2 = new Club(GetRandomName());
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

		Match match(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		EXPECT_TRUE(mat->StartMatch(&match));
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

		Match match(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		EXPECT_TRUE(mat->StartMatch(&match));
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
		EXPECT_EQ( ((Weightclass*) tables[1]->GetFilter())->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ( ((Weightclass*) tables[1]->GetFilter())->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ( ((Weightclass*) tables[1]->GetFilter())->GetGender(), Gender::Male);
		EXPECT_EQ(((RoundRobin*)tables[1])->IsBestOfThree(), true);

		EXPECT_TRUE(app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=3&name=Test3&mat=5&minWeight=10,8&maxWeight=20.4&gender=1&bo3=true&mf3=true&mf5=true")));

		ASSERT_EQ(tables.size(), 3);
		ASSERT_EQ(tables[2]->GetType(), MatchTable::Type::SingleElimination);
		EXPECT_EQ(tables[2]->GetName(), "Test3");
		EXPECT_EQ(tables[2]->GetMatID(), 5);
		EXPECT_EQ( ((Weightclass*) tables[2]->GetFilter())->GetMinWeight(), Weight("10,8"));
		EXPECT_EQ( ((Weightclass*) tables[2]->GetFilter())->GetMaxWeight(), Weight("20.4"));
		EXPECT_EQ( ((Weightclass*) tables[2]->GetFilter())->GetGender(), Gender::Female);
		EXPECT_EQ(((SingleElimination*)tables[2])->IsBestOfThree(), true);
		EXPECT_EQ(((SingleElimination*)tables[2])->IsThirdPlaceMatch(), true);
		EXPECT_EQ(((SingleElimination*)tables[2])->IsFifthPlaceMatch(), true);

		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("", "type=1&fight_system=4&name=Test4&mat=5&minWeight=10,9&maxWeight=20.5&gender=1&bo3=true&mf3=true&mf5=true")), "ok");

		ASSERT_EQ(tables.size(), 4);
		ASSERT_EQ(tables[3]->GetType(), MatchTable::Type::Pool);
		EXPECT_EQ(tables[3]->GetName(), "Test4");
		EXPECT_EQ(tables[3]->GetMatID(), 5);
		EXPECT_EQ( ((Weightclass*) tables[3]->GetFilter())->GetMinWeight(), Weight("10,9"));
		EXPECT_EQ( ((Weightclass*) tables[3]->GetFilter())->GetMaxWeight(), Weight("20.5"));
		EXPECT_EQ( ((Weightclass*) tables[3]->GetFilter())->GetGender(), Gender::Female);
		EXPECT_EQ(((Pool*)tables[3])->IsBestOfThree(), true);
		EXPECT_EQ(((Pool*)tables[3])->IsThirdPlaceMatch(), true);
		EXPECT_EQ(((Pool*)tables[3])->IsFifthPlaceMatch(), true);
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
		EXPECT_EQ( ((Weightclass*) tables[0]->GetFilter())->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ( ((Weightclass*) tables[0]->GetFilter())->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ( ((Weightclass*) tables[0]->GetFilter())->GetGender(), Gender::Male);
		EXPECT_EQ(((RoundRobin*)tables[0])->IsBestOfThree(), true);

		tables[0]->SetColor(Color::Name::Purple);
		tables[0]->SetScheduleIndex(10);

		auto old_uuid = tables[0]->GetUUID();

		EXPECT_EQ((std::string)app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID(), "name=Test2&fight_system=3&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")), "ok");

		ASSERT_EQ(tables.size(), 1);
		EXPECT_EQ(tables[0]->GetUUID(), old_uuid);
		EXPECT_EQ(tables[0]->GetColor(), Color::Name::Purple);
		EXPECT_EQ(tables[0]->GetScheduleIndex(), 9);//Gets changed since 9 is unused
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::SingleElimination);
		EXPECT_EQ(tables[0]->GetName(), "Test2");
		EXPECT_EQ(tables[0]->GetMatID(), 5);
		EXPECT_EQ( ((Weightclass*) tables[0]->GetFilter())->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ( ((Weightclass*) tables[0]->GetFilter())->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ( ((Weightclass*) tables[0]->GetFilter())->GetGender(), Gender::Male);
		EXPECT_EQ(((RoundRobin*)tables[0])->IsBestOfThree(), true);


		EXPECT_EQ((std::string)app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID(), "name=Test2&fight_system=4&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")), "ok");

		ASSERT_EQ(tables.size(), 1);
		EXPECT_EQ(tables[0]->GetUUID(), old_uuid);
		EXPECT_EQ(tables[0]->GetColor(), Color::Name::Purple);
		EXPECT_EQ(tables[0]->GetScheduleIndex(), 8);//Gets changed since 8 is unused
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::Pool);
		EXPECT_EQ(tables[0]->GetName(), "Test2");
		EXPECT_EQ(tables[0]->GetMatID(), 5);
		EXPECT_EQ( ((Weightclass*) tables[0]->GetFilter())->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ( ((Weightclass*) tables[0]->GetFilter())->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ( ((Weightclass*) tables[0]->GetFilter())->GetGender(), Gender::Male);
		EXPECT_EQ(((RoundRobin*)tables[0])->IsBestOfThree(), true);

		EXPECT_EQ((std::string)app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)(((Pool*)tables[0])->GetFinals()).GetUUID(), "name=Test4&fight_system=3&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")), "ok");

		auto& finals = ((Pool*)tables[0])->GetFinals();

		ASSERT_EQ(finals.GetName(), "Test4");
		ASSERT_EQ(finals.IsBestOfThree(), true);
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
		Pool* pool = (Pool*)tables[3];

		output = app.Ajax_GetMatchTable(HttpServer::Request("id=" + (std::string)pool->GetFinals().GetUUID()));

		YAML::Emitter yaml5;
		yaml5 << YAML::BeginMap;
		pool->GetFinals().ToString(yaml5);
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

			auto group = new SingleElimination(Weight(0), Weight(200));
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

		Match match(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		mat->StartMatch(&match);

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

		Match match(new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), nullptr, 1);

		EXPECT_TRUE(mat->StartMatch(&match));
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

	auto table = new RoundRobin(Weight(10), Weight(100));
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

	auto table = new RoundRobin(Weight(10), Weight(100));
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

	auto table = new SingleElimination(Weight(10), Weight(100));
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

	auto match1 = new Match(&j1, &j2, tourney, 1);
	auto match2 = new Match(&j1, &j2, tourney, 1);
	auto match3 = new Match(&j1, &j2, tourney, 1);
	auto match4 = new Match(&j1, &j2, tourney, 2);
	auto match5 = new Match(&j1, &j2, tourney, 2);
	auto match6 = new Match(&j1, &j2, tourney, 2);

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
}



TEST(Ajax, Tournament_Add)
{
	initialize();

	Application app;

	auto rules = new RuleSet("Test Rules", 100, 100, 20, 10);
	app.GetDatabase().AddRuleSet(rules);

	auto assoc = new Association("Organizer", nullptr);
	app.GetDatabase().AddAssociation(assoc);

	EXPECT_TRUE(app.Ajax_AddTournament(HttpServer::Request("", "name=test&year=2000&rules=" + (std::string)rules->GetUUID() + "&organizer=" + (std::string)assoc->GetUUID())));

	auto tour = app.FindTournamentByName("test");
	ASSERT_TRUE(tour);
	ASSERT_TRUE(tour->GetDefaultRuleSet());
	EXPECT_EQ(*tour->GetDefaultRuleSet(), *rules);
	EXPECT_EQ(tour->GetDatabase().GetYear(), 2000);
	ASSERT_TRUE(tour->GetOrganizer());
	EXPECT_EQ(*tour->GetOrganizer(), *assoc);
}



TEST(Ajax, Tournament_Get)
{
	initialize();

	Application app;

	auto rules = new RuleSet("Test Rules", 100, 100, 20, 10);
	app.GetDatabase().AddRuleSet(rules);

	auto assoc = new Association("Organizer", nullptr);
	app.GetDatabase().AddAssociation(assoc);

	EXPECT_TRUE(app.Ajax_AddTournament(HttpServer::Request("", "name=test&year=2000&rules=" + (std::string)rules->GetUUID() + "&organizer=" + (std::string)assoc->GetUUID())));

	auto tour = app.FindTournamentByName("test");
	ASSERT_TRUE(tour);

	YAML::Node yaml = YAML::Load( app.Ajax_GetTournament(HttpServer::Request("id=" + (std::string)tour->GetUUID()) ) );

	EXPECT_EQ(yaml["name"].as<std::string>(), tour->GetName());
	EXPECT_EQ(yaml["rule_set_uuid"].as<std::string>(),  *rules);
	EXPECT_EQ(yaml["organizer_uuid"].as<std::string>(), *assoc);
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

		auto rules1 = new RuleSet("Test Rules1", 100, 100, 20, 10);
		auto rules2 = new RuleSet("Test Rules2", 100, 100, 20, 10);
		app.GetDatabase().AddRuleSet(rules1);
		app.GetDatabase().AddRuleSet(rules2);

		auto assoc1 = new Association("Organizer1", nullptr);
		auto assoc2 = new Association("Organizer2", nullptr);
		app.GetDatabase().AddAssociation(assoc1);
		app.GetDatabase().AddAssociation(assoc2);

		EXPECT_TRUE(app.Ajax_AddTournament(HttpServer::Request("", "name=test&year=2000&rules=" + (std::string)rules1->GetUUID() + "&organizer=" + (std::string)assoc1->GetUUID())));

		EXPECT_FALSE(app.Ajax_EditTournament(HttpServer::Request("", "name=test2&year=2001&rules=" + (std::string)rules2->GetUUID() + "&organizer=" + (std::string)assoc2->GetUUID())));

		auto tour1 = app.FindTournamentByName("test");
		ASSERT_TRUE(tour1);
		EXPECT_TRUE(tour1->Save());

		EXPECT_FALSE(app.Ajax_EditTournament(HttpServer::Request("id=" + (std::string)tour1->GetUUID(), "name=test2&year=2001&rules=" + (std::string)rules2->GetUUID() + "&organizer=" + (std::string)assoc2->GetUUID())));

		EXPECT_TRUE(app.CloseTournament());

		EXPECT_TRUE(app.Ajax_EditTournament(HttpServer::Request("id=" + (std::string)tour1->GetUUID(), "name=test2&year=2001&readonly=true&rules=" + (std::string)rules2->GetUUID() + "&organizer=" + (std::string)assoc2->GetUUID())));

		EXPECT_FALSE(app.FindTournamentByName("test"));

		//Check if the old tournament file still exists
		EXPECT_FALSE(ZED::Core::RemoveFile("tournaments/test.yml"));

		auto tour = app.FindTournamentByName("test2");
		ASSERT_TRUE(tour);
		ASSERT_TRUE(tour->GetDefaultRuleSet());
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