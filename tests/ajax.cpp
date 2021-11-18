#include "tests.h"



TEST(Ajax, GetMats)
{
	initialize();

	Application app;

	auto csv = app.Ajax_GetMats();

	//csv
	int highestID, id, type;
	std::string name;
	bool isOpen;
	int style;

	csv >> highestID;

	EXPECT_TRUE(highestID == 1);

	app.StartLocalMat(1);

	auto csv2 = app.Ajax_GetMats();

	//csv
	csv2 >> highestID >> id >> type >> isOpen >> name >> style;

	EXPECT_TRUE(highestID == 2);
	EXPECT_TRUE(id == 1);
	EXPECT_TRUE(type == (int)Mat::Type::LocalMat);
	EXPECT_TRUE(name == "Mat 1");
	EXPECT_TRUE(style == (int)IMat::IpponDisplayStyle::DoubleDigit);
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

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 1);
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
	}


	{
		Application app;

		app.StartLocalMat(5);
		app.GetDefaultMat()->Close();

		app.Ajax_OpenMat(HttpServer::Request("id=1"));

		EXPECT_FALSE(app.GetDefaultMat()->IsOpen());

		app.Ajax_OpenMat(HttpServer::Request("id=5"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 5);
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

		app.Ajax_UpdateMat(HttpServer::Request("id=1", "id=5&name=Test&ipponStyle=0"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 5);
		EXPECT_TRUE(app.GetDefaultMat()->GetName()  == "Test");
		EXPECT_TRUE((int)app.GetDefaultMat()->GetIpponDisplayStyle() == 0);



		app.Ajax_UpdateMat(HttpServer::Request("id=5", "id=1&name=Test2&ipponStyle=1"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 1);
		EXPECT_TRUE(app.GetDefaultMat()->GetName() == "Test2");
		EXPECT_TRUE((int)app.GetDefaultMat()->GetIpponDisplayStyle() == 1);


		app.Ajax_UpdateMat(HttpServer::Request("id=1", "id=1&name=Test2&ipponStyle=2"));

		EXPECT_TRUE(app.GetDefaultMat());
		EXPECT_TRUE(app.GetDefaultMat()->IsOpen());
		EXPECT_TRUE(app.GetDefaultMat()->GetMatID() == 1);
		EXPECT_TRUE(app.GetDefaultMat()->GetName() == "Test2");
		EXPECT_TRUE((int)app.GetDefaultMat()->GetIpponDisplayStyle() == 2);
	}
}



TEST(Ajax, ListClubs)
{
	initialize();

	{
		Application app;

		app.GetDatabase().AddClub(new Club("Club 1"));

		auto csv = app.Ajax_ListClubs();

		int id1, id2;
		std::string name1, name2;
		csv >> id1 >> name1;

		EXPECT_TRUE(id1 == 2);
		EXPECT_TRUE(name1 == "Club 1");


		app.GetDatabase().AddClub(new Club("Club 2"));

		auto csv2 = app.Ajax_ListClubs();

		csv2 >> id1 >> name1 >> id2 >> name2;

		EXPECT_TRUE(id1 == 2);
		EXPECT_TRUE(name1 == "Club 1");
		EXPECT_TRUE(id2 == 3);
		EXPECT_TRUE(name2 == "Club 2");
	}
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