#include "tests.h"



TEST(Ajax, GetMats)
{
	initialize();

	Application app;

	auto yaml = YAML::Load(app.Ajax_GetMats());

	EXPECT_TRUE(yaml["highest_mat_id"].as<int>() == 1);

	app.StartLocalMat(1);

	yaml = YAML::Load(app.Ajax_GetMats());

	EXPECT_EQ(yaml["highest_mat_id"].as<int>(), 2);
	EXPECT_EQ(yaml["mats"][0]["type"].as<int>(), (int)Mat::Type::LocalMat);
	EXPECT_EQ(yaml["mats"][0]["name"].as<std::string>(), "Mat 1");
	EXPECT_EQ(yaml["mats"][0]["ippon_style"].as<int>(), (int)IMat::IpponStyle::DoubleDigit);
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

		EXPECT_TRUE(app.GetDefaultMat());
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

		app.Ajax_UpdateMat(HttpServer::Request("id=1", "id=5&name=Test&ipponStyle=0&timerStyle=1"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 5);
		EXPECT_TRUE(app.GetDefaultMat()->GetName()  == "Test");
		EXPECT_TRUE((int)app.GetDefaultMat()->GetIpponStyle() == 0);
		EXPECT_TRUE((int)app.GetDefaultMat()->GetTimerStyle() == 1);


		app.Ajax_UpdateMat(HttpServer::Request("id=5", "id=1&name=Test2&ipponStyle=1&timerStyle=2"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 1);
		EXPECT_TRUE(app.GetDefaultMat()->GetName() == "Test2");
		EXPECT_TRUE((int)app.GetDefaultMat()->GetIpponStyle() == 1);
		EXPECT_TRUE((int)app.GetDefaultMat()->GetTimerStyle() == 2);


		app.Ajax_UpdateMat(HttpServer::Request("id=1", "id=1&name=Test2&ipponStyle=2&timerStyle=0"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 1);
		EXPECT_TRUE(app.GetDefaultMat()->GetName() == "Test2");
		EXPECT_TRUE((int)app.GetDefaultMat()->GetIpponStyle() == 2);
		EXPECT_TRUE((int)app.GetDefaultMat()->GetTimerStyle() == 0);
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
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 1);


		app.Ajax_SetFullscreen(false, HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_FALSE(app.GetDefaultMat()->IsFullscreen());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 1);


		app.Ajax_SetFullscreen(true, HttpServer::Request("id=1"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_TRUE(app.GetDefaultMat()->IsFullscreen());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 1);
	}
}



TEST(Ajax, Ajax_GetHansokumake)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret.length(), 0);

		mat->StartMatch(&match);
		mat->AddHansokuMake(f);
		
		ZED::CSV ret2 = app.Ajax_GetHansokumake();

		std::string id;
		int mat_id, state, matchtable, hansokumake_fighter, hansokumake_state;
		std::string white_name, blue_name, color, matchtable_name;
		ret2 >> id >> white_name >> blue_name >> mat_id >> state >> color >> matchtable >> matchtable_name;
		ret2 >> hansokumake_fighter >> hansokumake_state;

		EXPECT_EQ(id, (std::string)match.GetUUID());
		EXPECT_EQ(white_name, match.GetFighter(Fighter::White)->GetName());
		EXPECT_EQ(blue_name,  match.GetFighter(Fighter::Blue )->GetName());
		EXPECT_EQ(hansokumake_fighter, (int)f);
		EXPECT_EQ(hansokumake_state, (int)IMat::Scoreboard::DisqualificationState::Unknown);
	}


	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret.length(), 0);

		mat->StartMatch(&match);
		mat->AddHansokuMake(f);
		mat->AddDisqualification(f);

		ZED::CSV ret2 = app.Ajax_GetHansokumake();

		std::string id;
		int mat_id, state, matchtable, hansokumake_fighter, hansokumake_state;
		std::string white_name, blue_name, color, matchtable_name;
		ret2 >> id >> white_name >> blue_name >> mat_id >> state >> color >> matchtable >> matchtable_name;
		ret2 >> hansokumake_fighter >> hansokumake_state;

		EXPECT_EQ(id, (std::string)match.GetUUID());
		EXPECT_EQ(white_name, match.GetFighter(Fighter::White)->GetName());
		EXPECT_EQ(blue_name,  match.GetFighter(Fighter::Blue)->GetName());
		EXPECT_EQ(hansokumake_fighter, (int)f);
		EXPECT_EQ(hansokumake_state, (int)IMat::Scoreboard::DisqualificationState::Disqualified);
	}


	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret.length(), 0);

		mat->StartMatch(&match);
		mat->AddHansokuMake(f);
		mat->AddNoDisqualification(f);

		ZED::CSV ret2 = app.Ajax_GetHansokumake();

		std::string id;
		int mat_id, state, matchtable, hansokumake_fighter, hansokumake_state;
		std::string white_name, blue_name, color, matchtable_name;
		ret2 >> id >> white_name >> blue_name >> mat_id >> state >> color >> matchtable >> matchtable_name;
		ret2 >> hansokumake_fighter >> hansokumake_state;

		EXPECT_EQ(id, (std::string)match.GetUUID());
		EXPECT_EQ(white_name, match.GetFighter(Fighter::White)->GetName());
		EXPECT_EQ(blue_name,  match.GetFighter(Fighter::Blue)->GetName());
		EXPECT_EQ(hansokumake_fighter, (int)f);
		EXPECT_EQ(hansokumake_state, (int)IMat::Scoreboard::DisqualificationState::NotDisqualified);
	}
}



TEST(Ajax, Ajax_GetHansokumake2)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret.length(), 0);

		mat->StartMatch(&match);
		for (int i = 0;i < 5; i++)
			mat->AddShido(f);

		auto ret2 = app.Ajax_GetHansokumake();

		EXPECT_EQ(ret2.length(), 0);
	}
}



TEST(Ajax, AddClub)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddClub(HttpServer::Request("", "name=Test Club")), "ok");

		auto clubs = app.GetDatabase().GetAllClubs();

		ASSERT_EQ(clubs.size(), 1);
		EXPECT_EQ(clubs[0]->GetName(), "Test Club");
	}
}



TEST(Ajax, GetClub)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddClub(HttpServer::Request("", "name=Test Club")), "ok");

		auto clubs = app.GetDatabase().GetAllClubs();

		ASSERT_EQ(clubs.size(), 1);
		EXPECT_EQ(clubs[0]->GetName(), "Test Club");

		Club club2(YAML::Load(app.Ajax_GetClub(HttpServer::Request("id="+(std::string)clubs[0]->GetUUID()))));

		EXPECT_EQ(clubs[0]->GetName(),   club2.GetName());
		EXPECT_EQ(clubs[0]->GetUUID(),   club2.GetUUID());
		EXPECT_EQ(clubs[0]->GetParent(), club2.GetParent());
		EXPECT_EQ(clubs[0]->GetLevel(),  club2.GetLevel());
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

		EXPECT_EQ((std::string)app.Ajax_EditClub(HttpServer::Request("id="+(std::string)clubs[0]->GetUUID(), "name=Test Club 2")), "ok");

		EXPECT_EQ(clubs[0]->GetName(), "Test Club 2");
	}
}



TEST(Ajax, DeleteClub)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddClub(HttpServer::Request("", "name=Test Club")), "ok");

		auto clubs = app.GetDatabase().GetAllClubs();

		ASSERT_EQ(clubs.size(), 1);

		EXPECT_EQ((std::string)app.Ajax_DeleteClub(HttpServer::Request("id="+(std::string)clubs[0]->GetUUID(), "name=Test Club 2")), "ok");

		EXPECT_EQ(clubs.size(), 0);
	}
}



TEST(Ajax, DeleteAssociation)
{
	initialize();

	{
		Application app;

		auto inter = new Judoboard::Association("International");

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

		auto inter = new Judoboard::Association("International");

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


		EXPECT_EQ((std::string)app.Ajax_EditClub(HttpServer::Request("id=" + (std::string)nieder->GetUUID(), "name=Niedersachen 2&parent=" + (std::string)de->GetUUID())), "ok");

		EXPECT_EQ(nieder->GetName(), "Niedersachen 2");
		EXPECT_EQ(nieder->GetLevel(), 2);
		EXPECT_EQ(nieder->GetParent()->GetUUID(), de->GetUUID());
	}
}



TEST(Ajax, ListClubs)
{
	initialize();

	{
		ZED::Core::RemoveFile("database.yml");
		Application app;

		app.GetDatabase().AddClub(new Club("Club 1"));

		YAML::Node yaml = YAML::Load(app.Ajax_ListClubs());

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Club 1");


		app.GetDatabase().AddClub(new Club("Club 2"));

		yaml = YAML::Load(app.Ajax_ListClubs());

		ASSERT_EQ(yaml.size(), 2);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Club 1");
		EXPECT_EQ(yaml[1]["name"].as<std::string>(), "Club 2");
	}
}



TEST(Ajax, ListAssociations)
{
	initialize();

	{
		ZED::Core::RemoveFile("database.yml");
		Application app;

		auto assoc1 = new Association("Assoc 1");
		app.GetDatabase().AddAssociation(assoc1);

		YAML::Node yaml = YAML::Load(app.Ajax_ListAssociations());

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)assoc1->GetUUID());


		auto assoc2 = new Association("Assoc 2", assoc1);
		app.GetDatabase().AddAssociation(assoc2);

		yaml = YAML::Load(app.Ajax_ListAssociations());

		ASSERT_EQ(yaml.size(), 2);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), (std::string)assoc1->GetUUID());

		EXPECT_EQ(yaml[1]["name"].as<std::string>(), "Assoc 2");
		EXPECT_EQ(yaml[1]["uuid"].as<std::string>(), (std::string)assoc2->GetUUID());
		EXPECT_EQ(yaml[1]["parent_name"].as<std::string>(), "Assoc 1");
		EXPECT_EQ(yaml[1]["parent_uuid"].as<std::string>(), (std::string)assoc1->GetUUID());
	}

	ZED::Core::RemoveFile("database.yml");
}



TEST(Ajax, Uptime)
{
	initialize();

	{
		Application app;

		{
			auto csv = app.Ajax_Uptime();
			uint32_t uptime;
			csv >> uptime;
			EXPECT_TRUE(uptime < 100);
		}
		
		ZED::Core::Pause(1000);

		{
			auto csv = app.Ajax_Uptime();
			uint32_t uptime;
			csv >> uptime;
			EXPECT_TRUE(uptime < 1100);
		}

		ZED::Core::Pause(1000);

		{
			auto csv = app.Ajax_Uptime();
			uint32_t uptime;
			csv >> uptime;
			EXPECT_TRUE(uptime < 2100);
		}
	}
}



TEST(Ajax, AddDisqualification)
{
	initialize();

	for (Fighter f = Fighter::White; f <= Fighter::Blue; f++)
	{
		Application app;

		app.StartLocalMat(1);
		IMat* mat = app.FindMat(1);

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		mat->StartMatch(&match);
		mat->AddHansokuMake(f);

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsUnknownDisqualification());

		app.Ajax_AddDisqualification(f, HttpServer::Request("id=1"));

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

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		mat->StartMatch(&match);
		mat->AddHansokuMake(f);

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsUnknownDisqualification());

		app.Ajax_AddDisqualification(f, HttpServer::Request("id=1"));

		EXPECT_TRUE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsUnknownDisqualification());

		app.Ajax_RemoveDisqualification(f, HttpServer::Request("id=2"));

		EXPECT_TRUE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsUnknownDisqualification());

		app.Ajax_RemoveDisqualification(f, HttpServer::Request("id=1"));

		EXPECT_FALSE(mat->GetScoreboard(f).IsDisqualified());
		EXPECT_FALSE(mat->GetScoreboard(f).IsNotDisqualified());
		EXPECT_TRUE(mat->GetScoreboard(f).IsUnknownDisqualification());
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

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		mat->StartMatch(&match);
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

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		mat->StartMatch(&match);
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

	auto table = new Weightclass(app.GetTournament(), 10, 100);
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

	auto table = new Weightclass(app.GetTournament(), 10, 100);
	app.GetTournament()->AddMatchTable(table);

	YAML::Node yaml = YAML::Load(app.Ajax_GetMatchesFromMatchTable(HttpServer::Request("id=" + (std::string)table->GetUUID())));

	ASSERT_EQ(yaml.size(), 3);

	int i = 0;
	for (const auto& node : yaml)
	{
		EXPECT_EQ(node["uuid"].as<std::string>(),       (std::string)table->GetSchedule()[i]->GetUUID());
		EXPECT_EQ(node["white_name"].as<std::string>(), table->GetSchedule()[i]->GetFighter(Fighter::White)->GetName());
		EXPECT_EQ(node["blue_name"].as<std::string>(),  table->GetSchedule()[i]->GetFighter(Fighter::Blue )->GetName());
		EXPECT_EQ(node["mat_id"].as<int>(),             table->GetSchedule()[i]->GetMatID());
		i++;
	}
}



TEST(Ajax, ListAgeGroups)
{
	initialize();

	ZED::Core::RemoveFile("database.yml");
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