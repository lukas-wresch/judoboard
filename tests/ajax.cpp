#include "tests.h"



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

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		mat->StartMatch(&match);
		mat->AddHansokuMake(f);
		
		YAML::Node ret2 = YAML::Load(app.Ajax_GetHansokumake());

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

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		mat->StartMatch(&match);
		mat->AddHansokuMake(f);
		mat->AddDisqualification(f);

		YAML::Node ret2 = YAML::Load(app.Ajax_GetHansokumake());

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

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		mat->StartMatch(&match);
		mat->AddHansokuMake(f);
		mat->AddNoDisqualification(f);

		YAML::Node ret2 = YAML::Load(app.Ajax_GetHansokumake());

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

		Match match(nullptr, new Judoka(GetRandomName(), GetRandomName()), new Judoka(GetRandomName(), GetRandomName()), 1);

		auto ret = app.Ajax_GetHansokumake();
		EXPECT_EQ(ret, "[]");

		mat->StartMatch(&match);
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
		auto judoka = judokas.begin()->second;

		EXPECT_EQ(judoka->GetFirstname(), "first");
		EXPECT_EQ(judoka->GetLastname(),  "last");
		EXPECT_EQ(judoka->GetWeight(),  Weight(10));
		EXPECT_EQ(judoka->GetGender(),  Gender::Male);
		EXPECT_EQ(judoka->GetBirthyear(), 2000);
		EXPECT_EQ(judoka->GetNumber(), "A123");
	}

	ZED::Core::RemoveFile("database.yml");
}



TEST(Ajax, Judoka_Edit)
{
	initialize();

	{
		Application app;

		EXPECT_EQ((std::string)app.Ajax_AddJudoka(HttpServer::Request("", "firstname=first&lastname=last&weight=10&gender=0&birthyear=2000&number=A123")), "ok");

		auto judokas = app.GetDatabase().GetAllJudokas();

		ASSERT_EQ(judokas.size(), 1);
		auto judoka = judokas.begin()->second;

		EXPECT_EQ((std::string)app.Ajax_EditJudoka(HttpServer::Request("id="+(std::string)judoka->GetUUID(), "firstname=first2&lastname=last2&weight=12,5&gender=1&birthyear=2001&number=A1234")), "ok");

		EXPECT_EQ(judoka->GetFirstname(), "first2");
		EXPECT_EQ(judoka->GetLastname(),  "last2");
		EXPECT_EQ(judoka->GetWeight(),  Weight("12,5"));
		EXPECT_EQ(judoka->GetGender(),  Gender::Female);
		EXPECT_EQ(judoka->GetBirthyear(), 2001);
		EXPECT_EQ(judoka->GetNumber(), "A1234");
	}

	ZED::Core::RemoveFile("database.yml");
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

	ZED::Core::RemoveFile("database.yml");
}



TEST(Ajax, Clubs_List)
{
	initialize();

	{
		ZED::Core::RemoveFile("database.yml");
		Application app;

		auto c1 = new Club("Club 1");
		c1->SetShortName("c1");
		app.GetDatabase().AddClub(c1);

		YAML::Node yaml = YAML::Load(app.Ajax_ListClubs());

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Club 1");


		app.GetDatabase().AddClub(new Club("Club 2"));

		yaml = YAML::Load(app.Ajax_ListClubs());

		ASSERT_EQ(yaml.size(), 2);
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "Club 1");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "c1");
		EXPECT_EQ(yaml[1]["name"].as<std::string>(), "Club 2");
	}

	ZED::Core::RemoveFile("database.yml");
}



TEST(Ajax, Clubs_Get)
{
	initialize();

	{
		ZED::Core::RemoveFile("database.yml");
		Application app;

		auto c = new Club("Club 1");
		c->SetShortName("c");
		app.GetDatabase().AddClub(c);

		YAML::Node yaml = YAML::Load(app.Ajax_GetClub(HttpServer::Request("id=" + (std::string)c->GetUUID())));

		EXPECT_EQ(yaml["uuid"].as<std::string>(), c->GetUUID());
		EXPECT_EQ(yaml["name"].as<std::string>(), c->GetName());
		EXPECT_EQ(yaml["short_name"].as<std::string>(), c->GetShortName());
	}

	ZED::Core::RemoveFile("database.yml");
}



TEST(Ajax, Clubs_Edit)
{
	initialize();

	{
		ZED::Core::RemoveFile("database.yml");
		Application app;

		auto c = new Club("Club 1");
		app.GetDatabase().AddClub(c);

		EXPECT_EQ((std::string)app.Ajax_EditClub(HttpServer::Request("id=" + (std::string)c->GetUUID(), "name=NewName&shortname=c")), "ok");


		auto yaml = YAML::Load(app.Ajax_ListClubs());

		ASSERT_EQ(yaml.size(), 1);
		EXPECT_EQ(yaml[0]["uuid"].as<std::string>(), c->GetUUID());
		EXPECT_EQ(yaml[0]["name"].as<std::string>(), "NewName");
		EXPECT_EQ(yaml[0]["short_name"].as<std::string>(), "c");
	}

	ZED::Core::RemoveFile("database.yml");
}



TEST(Ajax, Clubs_Delete)
{
	initialize();

	{
		ZED::Core::RemoveFile("database.yml");
		Application app;

		auto c = new Club("Club 1");
		app.GetDatabase().AddClub(c);

		EXPECT_EQ((std::string)app.Ajax_DeleteClub(HttpServer::Request("id=" + (std::string)c->GetUUID())), "ok");

		auto yaml = YAML::Load(app.Ajax_ListClubs());

		ASSERT_EQ(yaml.size(), 0);
	}

	ZED::Core::RemoveFile("database.yml");
}



TEST(Ajax, ListAssociations)
{
	initialize();

	{
		ZED::Core::RemoveFile("database.yml");
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

	ZED::Core::RemoveFile("database.yml");
}



TEST(Ajax, Status)
{
	initialize();

	{
		Application app;

		{
			YAML::Node yaml = YAML::Load(app.Ajax_Status());
			uint32_t uptime = yaml["uptime"].as<uint32_t>();
			EXPECT_TRUE(uptime < 100);

			auto version = yaml["version"].as<std::string>();
			EXPECT_EQ(version, Application::Version);
		}
		
		ZED::Core::Pause(1000);

		{
			YAML::Node yaml = YAML::Load(app.Ajax_Status());
			uint32_t uptime = yaml["uptime"].as<uint32_t>();
			EXPECT_TRUE(uptime < 1100);
		}

		ZED::Core::Pause(1000);

		{
			YAML::Node yaml = YAML::Load(app.Ajax_Status());
			uint32_t uptime = yaml["uptime"].as<uint32_t>();
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



TEST(Ajax, MatchTable_Add)
{
	initialize();

	{
		Application app;

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("type=1", "name=Test&mat=7")), "ok");

		ASSERT_EQ(tables.size(), 1);
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::Weightclass);
		EXPECT_EQ(tables[0]->GetName(), "Test");
		EXPECT_EQ(tables[0]->GetMatID(), 7);


		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("type=1", "name=Test2&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")), "ok");

		ASSERT_EQ(tables.size(), 2);
		ASSERT_EQ(tables[1]->GetType(), MatchTable::Type::Weightclass);
		EXPECT_EQ(tables[1]->GetName(), "Test2");
		EXPECT_EQ(tables[1]->GetMatID(), 5);
		EXPECT_EQ(((Weightclass*)tables[1])->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ(((Weightclass*)tables[1])->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ(((Weightclass*)tables[1])->GetGender(), Gender::Male);
		EXPECT_EQ(((Weightclass*)tables[1])->IsBestOfThree(), true);

		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("type=4", "name=Test3&mat=5&minWeight=10,7&maxWeight=20.3&gender=1&bo3=true&mf3=true&mf5=true")), "ok");

		ASSERT_EQ(tables.size(), 3);
		ASSERT_EQ(tables[2]->GetType(), MatchTable::Type::SingleElimination);
		EXPECT_EQ(tables[2]->GetName(), "Test3");
		EXPECT_EQ(tables[2]->GetMatID(), 5);
		EXPECT_EQ(((SingleElimination*)tables[1])->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ(((SingleElimination*)tables[1])->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ(((SingleElimination*)tables[1])->GetGender(), Gender::Female);
		EXPECT_EQ(((SingleElimination*)tables[1])->IsBestOfThree(), true);
		EXPECT_EQ(((SingleElimination*)tables[1])->IsThirdPlaceMatch(), true);
		EXPECT_EQ(((SingleElimination*)tables[1])->IsFifthPlaceMatch(), true);
	}
}



TEST(Ajax, MatchTable_Edit)
{
	initialize();

	{
		Application app;

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("type=1", "name=Test&mat=7")), "ok");

		ASSERT_EQ(tables.size(), 1);
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::Weightclass);
		EXPECT_EQ(tables[0]->GetName(), "Test");
		EXPECT_EQ(tables[0]->GetMatID(), 7);


		EXPECT_EQ((std::string)app.Ajax_EditMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID(), "name=Test2&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")), "ok");

		ASSERT_EQ(tables.size(), 1);
		ASSERT_EQ(tables[0]->GetType(), MatchTable::Type::Weightclass);
		EXPECT_EQ(tables[0]->GetName(), "Test2");
		EXPECT_EQ(tables[0]->GetMatID(), 5);
		EXPECT_EQ(((Weightclass*)tables[0])->GetMinWeight(), Weight("10,7"));
		EXPECT_EQ(((Weightclass*)tables[0])->GetMaxWeight(), Weight("20.3"));
		EXPECT_EQ(((Weightclass*)tables[0])->GetGender(), Gender::Male);
		EXPECT_EQ(((Weightclass*)tables[0])->IsBestOfThree(), true);
	}
}



TEST(Ajax, MatchTable_Get)
{
	initialize();

	{
		Application app;

		auto& tables = app.GetTournament()->GetMatchTables();

		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("type=1", "name=Test&mat=7")), "ok");

		ASSERT_EQ(tables.size(), 1);

		auto output = app.Ajax_GetMatchTable(HttpServer::Request("id=" + (std::string)tables[0]->GetUUID()));

		YAML::Emitter yaml1;
		yaml1 << YAML::BeginMap;
		tables[0]->ToString(yaml1);
		yaml1 << YAML::EndMap;
		EXPECT_EQ(yaml1.c_str(), output);


		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("type=1", "name=Test2&mat=5&minWeight=10,7&maxWeight=20.3&gender=0&bo3=true")), "ok");

		ASSERT_EQ(tables.size(), 2);

		output = app.Ajax_GetMatchTable(HttpServer::Request("id=" + (std::string)tables[1]->GetUUID()));

		YAML::Emitter yaml2;
		yaml2 << YAML::BeginMap;
		tables[1]->ToString(yaml2);
		yaml2 << YAML::EndMap;
		EXPECT_EQ(yaml2.c_str(), output);

		EXPECT_EQ((std::string)app.Ajax_AddMatchTable(HttpServer::Request("type=4", "name=Test3&mat=5&minWeight=10,7&maxWeight=20.3&gender=1&bo3=true&mf3=true&mf5=true")), "ok");

		ASSERT_EQ(tables.size(), 3);

		output = app.Ajax_GetMatchTable(HttpServer::Request("id=" + (std::string)tables[2]->GetUUID()));

		YAML::Emitter yaml3;
		yaml3 << YAML::BeginMap;
		tables[2]->ToString(yaml3);
		yaml3 << YAML::EndMap;
		EXPECT_EQ(yaml3.c_str(), output);

		auto node = YAML::Load(output);

		ASSERT_TRUE(node["best_of_three"].IsDefined());
		ASSERT_TRUE(node["third_place"].IsDefined());
		ASSERT_TRUE(node["fifth_place"].IsDefined());

		EXPECT_EQ(node["best_of_three"].as<bool>(), true);
		EXPECT_EQ(node["third_place"].as<bool>(), true);
		EXPECT_EQ(node["fifth_place"].as<bool>(), true);
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

	auto table = new Weightclass(10, 100);
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

	auto table = new Weightclass(10, 100);
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



TEST(Ajax, SetStartingPosition)
{
	initialize();

	Application app;

	auto j1 = new Judoka(GetRandomName(), GetRandomName(), 50);
	auto j2 = new Judoka(GetRandomName(), GetRandomName(), 50);
	auto j3 = new Judoka(GetRandomName(), GetRandomName(), 50);

	app.GetTournament()->AddParticipant(j1);
	app.GetTournament()->AddParticipant(j2);
	app.GetTournament()->AddParticipant(j3);

	auto table = new SingleElimination(10, 100);
	app.GetTournament()->AddMatchTable(table);

	for (int i = 0; i < 100; ++i)
	{
		int startpos = rand() % 4;
		EXPECT_EQ((std::string)app.Ajax_SetStartingPosition(HttpServer::Request( "id=" + (std::string)table->GetUUID() + "&judoka=" + (std::string)j1->GetUUID() + "&startpos=" + std::to_string(startpos) )), "ok");

		EXPECT_EQ(table->GetStartingPosition(j1), startpos);
	}

	for (int i = 0; i < 100; ++i)
	{
		int startpos = rand() % 4;
		EXPECT_EQ((std::string)app.Ajax_SetStartingPosition(HttpServer::Request( "id=" + (std::string)table->GetUUID() + "&judoka=" + (std::string)j2->GetUUID() + "&startpos=" + std::to_string(startpos) )), "ok");

		EXPECT_EQ(table->GetStartingPosition(j2), startpos);
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



TEST(Ajax, AddTournament)
{
	initialize();

	ZED::Core::RemoveFile("database.yml");
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



TEST(Ajax, EditTournament)
{
	initialize();

	ZED::Core::RemoveFile("database.yml");
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

	EXPECT_FALSE(app.Ajax_EditTournament(HttpServer::Request("id=" + (std::string)tour1->GetUUID(), "name=test2&year=2001&rules=" + (std::string)rules2->GetUUID() + "&organizer=" + (std::string)assoc2->GetUUID())));

	EXPECT_TRUE(app.CloseTournament());

	EXPECT_TRUE(app.Ajax_EditTournament(HttpServer::Request("id=" + (std::string)tour1->GetUUID(), "name=test2&year=2001&rules=" + (std::string)rules2->GetUUID() + "&organizer=" + (std::string)assoc2->GetUUID())));

	auto tour = app.FindTournamentByName("test2");
	ASSERT_TRUE(tour);
	ASSERT_TRUE(tour->GetDefaultRuleSet());
	EXPECT_EQ(*tour->GetDefaultRuleSet(), *rules2);
	EXPECT_EQ(tour->GetDatabase().GetYear(), 2001);
	ASSERT_TRUE(tour->GetOrganizer());
	EXPECT_EQ(*tour->GetOrganizer(), *assoc2);
}