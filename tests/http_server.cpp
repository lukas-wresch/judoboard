#include "tests.h"



TEST(HttpServer, SimpleRequest)
{
	initialize();

	//Block server with semi open connections

	int port = rand()%5000 + 2000;
	Application app(port);

	ZED::HttpClient client("localhost", port);
	ASSERT_TRUE(client.IsConnected());

	auto time = ZED::Core::CurrentTimestamp();
	auto packet = client.GET("/");
	EXPECT_GE(packet.body.size(), 1024);
	EXPECT_LE(ZED::Core::CurrentTimestamp() - time, 10u);
}



TEST(HttpServer, MultipleRequests)
{
	initialize();

	//Block server with semi open connections

	int port = rand()%5000 + 2000;
	Application app(port);


	{
		auto time = ZED::Core::CurrentTimestamp();
		ZED::HttpClient client("localhost", port);
		client.GET("/");
		EXPECT_LE(ZED::Core::CurrentTimestamp() - time, 10u);
	}

	auto connection = [=]() {
		ZED::HttpClient client = ZED::HttpClient("localhost", port);
		client.SendGETRequest("/");
	};

	for (int i = 0; i < 50; i++)
	{
		std::thread thread(connection);
		thread.detach();
	}

	{
		auto time = ZED::Core::CurrentTimestamp();
		ZED::HttpClient client("localhost", port);
		client.GET("/");
		EXPECT_LE(ZED::Core::CurrentTimestamp() - time, 20u);
	}
}



TEST(HttpServer, Block_SemiOpen)
{
	initialize();

	//Block server with semi open connections

	int port = rand()%5000 + 2000;
	Application app(port);

	std::thread app_thread([&]() {
		app.Run();
	});

	{
		auto time = ZED::Core::CurrentTimestamp();
		ZED::HttpClient client("localhost", port);
		client.GET("/");
		time = ZED::Core::CurrentTimestamp() - time;
		EXPECT_LE(time, 10u);
	}

	auto connection_keep_alive = [=]() {
		ZED::HttpClient* client = new ZED::HttpClient("localhost", port);
		client->SetKeepAlive();
		client->SendGETRequest("/");

		ZED::Core::Pause(80 * 1000);

		delete client;
	};

#ifdef _DEBUG
	for (int i = 0; i < 15; i++)
#else
	for (int i = 0; i < 50; i++)
#endif
	{
		std::thread thread(connection_keep_alive);
		thread.detach();
	}

	ZED::Core::Pause(40 * 1000);

	{
		auto time = ZED::Core::CurrentTimestamp();
		ZED::HttpClient client("localhost", port);
		client.GET("/");
		time = ZED::Core::CurrentTimestamp() - time;
		EXPECT_LE(time, 30u);
	}

	app.Shutdown();
	app_thread.join();
}