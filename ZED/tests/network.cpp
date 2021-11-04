#include "CppUnitTest.h"
#include "../include/core.h"
#include "../include/socket_tcp.h"
#include "../include/socket_udp.h"
#include "../include/server_tcp.h"
#include "../include/server_udp.h"
#include "../include/http_client.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;



namespace UnitTests
{
	TEST_CLASS(Network)
	{
	public:

		TEST_METHOD(SocketFastClose)
		{
			ZED::SocketTCP client;
		}

		TEST_METHOD(SocketTest)
		{
			ZED::SocketTCP client;
			Assert::IsTrue(client.Connect("www.google.com", 80));

			Assert::IsTrue(client.Send("Hello", 5));
		}

		TEST_METHOD(TCPServerTest)
		{
			ZED::ServerTCP server(8000);
			ZED::SocketTCP client;

			Assert::IsTrue(server.IsValid());
			Assert::IsTrue(client.Connect("127.0.0.1", 8000));

			ZED::Core::Pause(100);

			Assert::IsTrue(server.AcceptClients());

			Assert::IsTrue(server.GetClients().size() == 1);

			client.Send("Hello", 5);

			server.Recv();
			Assert::IsTrue(server.GetClients()[0].Buffer2String() == "Hello");
		}



		TEST_METHOD(TCPServerDetectedDisconnect)
		{
			ZED::ServerTCP server(8000);
			ZED::SocketTCP client;

			Assert::IsTrue(server.IsValid());
			Assert::IsTrue(client.Connect("127.0.0.1", 8000));

			ZED::Core::Pause(100);

			//Assert::IsTrue(server.AcceptClients());
			server.Recv();

			Assert::IsTrue(server.GetClients().size() == 1);

			client.Send("Hello", 5);

			server.Recv();
			Assert::IsTrue(server.GetClients()[0].Buffer2String() == "Hello");

			client.Disconnect();

			for (int i = 0; i < 10; i++)
			{
				server.Recv();
				server.Broadcast("Test");
				ZED::Core::Pause(10);
			}

			Assert::IsTrue(server.GetClients().size() == 0);
		}


		TEST_METHOD(ServerFastClose)
		{
			ZED::ServerTCP server(8000);
		}


		TEST_METHOD(UDPServerTest)
		{
			ZED::ServerUDP server(8000);
			ZED::SocketUDP client;

			Assert::IsTrue(server.IsValid());
			Assert::IsTrue(client.Connect("127.0.0.1", 8000));

			ZED::Core::Pause(100);

			//Assert::IsFalse(server.AcceptClients());

			Assert::IsTrue(client.Send("Please Connect"));

			ZED::Core::Pause(100);

			//Assert::IsTrue(server.AcceptClients());
			server.Recv();
			//Assert::IsTrue(server.GetClients()[0].Buffer2String() == "Please Connect");
			server.GetClient(0)->ClearBuffer();

			Assert::IsTrue(server.GetClients().size() == 1);
			server.GetClient(0)->ClearBuffer();

			client.Send("Hello", 5);

			server.Recv();
			Assert::IsTrue(server.GetClient(0)->Buffer2String() == "Hello");
		}


		TEST_METHOD(UDPTest)
		{
			ZED::SocketUDP peer1;
			ZED::SocketUDP peer2;
			
			Assert::IsTrue(peer2.Listen(8000));
			Assert::IsTrue(peer1.Connect("127.0.0.1", 8000));

			Assert::IsTrue(peer1.IsConnected());
			Assert::IsFalse(peer2.IsConnected());

			Assert::IsTrue(peer1.Send("Hello", 5));
			Assert::IsTrue(peer2.Recv());

			Assert::IsTrue(peer2.Buffer2String() == "Hello");

			Assert::IsTrue(peer1.IsConnected());
			Assert::IsTrue(peer2.IsConnected());

			Assert::IsTrue(peer2.Send("Resp", 4));
			Assert::IsTrue(peer1.Recv());

			Assert::IsTrue(peer1.Buffer2String() == "Resp");
		}


		TEST_METHOD(DownloadFromInvalidHost)
		{
			ZED::HttpClient client("https://raw.gfgsfjksnkgldmfl;hmflj;kfl;hl;ercontent.com");
			auto packet = client.GET("/Dafiletch");

			Assert::IsTrue(packet.header.rfind("", 0) == 0);
			Assert::IsTrue(packet.body.rfind("", 0) == 0);
		}

		TEST_METHOD(DownloadFromGithub)
		{
			ZED::HttpClient client("raw.githubusercontent.com");
			auto packet = client.GET("/DaMarkov/ZED-Online/master/client_data/patch");

			Assert::IsTrue(packet.body.rfind("PATCH_FILE", 0) == 0);
		}
	};
}