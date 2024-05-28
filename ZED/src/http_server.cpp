#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include "../include/http_server.h"
#include "../include/log.h"


using namespace ZED;



void HttpServer::AddListeningPort(Socket* NewSocket, uint16_t Port)
{
	if (NewSocket)
	{
		NewSocket->Listen(Port);
		NewSocket->MakeBlocking(true);
		m_ListeningSockets.emplace_back(NewSocket);
	}
}



bool HttpServer::Start()
{
	//if (!m_ServerSocket.Listen(80))
		//return false;

	//m_ServerSocket.MakeBlocking(true);

	m_Thread = std::thread([this]()
	{
		FD_SET original_fds;
		FD_ZERO(&original_fds);
		for (auto listening_socket : m_ListeningSockets)
			FD_SET(listening_socket->GetSocket(), &original_fds);

		while (m_Running)
		{
			FD_SET read_fds = original_fds;
			if (select(NULL, &read_fds, NULL, NULL, NULL) > 0)
			{
				for (auto listening_socket : m_ListeningSockets)
				{
					if (FD_ISSET(listening_socket->GetSocket(), &read_fds))
					{
						auto new_connection = listening_socket->AcceptClient();
					}
				}
			}
		}
	});
	
	return true;
}