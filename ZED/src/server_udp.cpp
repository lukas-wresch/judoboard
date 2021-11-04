#include <stdio.h>
#include "../include/server_udp.h"
#include "../include/log.h"


#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif


using namespace ZED;



/*bool ServerUDP::AcceptClients()
{
	if (!m_ServerSocket.IsConnected())
		m_ServerSocket.Recv();

	if (m_ServerSocket.IsConnected())
	{
		//This also disconnected the current connect client from m_ServerSocket
		SocketUDP new_client(m_ServerSocket);
		//m_Clients.emplace_back(std::move(new_client));
		return true;
	}
	/*auto new_socket = m_ServerSocket.AcceptClients();
	if (new_socket.IsConnected())
	{
		m_Clients.emplace_back(std::move(new_socket));
		return true;
	}*/

	/*return false;
}*/



void ServerUDP::Recv()
{
	//Block until data is ready
	/*fd_set fdset;
	FD_ZERO(&fdset);

	FD_SET(m_ServerSocket.GetSocket(), &fdset);
#ifdef LINUX
	int max = m_ServerSocket.GetSocket();
#endif

	for (auto& client : m_Clients)
	{
		FD_SET(client.GetSocket(), &fdset);

#ifdef LINUX
		if (client.GetSocket() > max)
			max = client.GetSocket();
#endif
	}

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 5 * 1000;//5ms

#ifdef _WIN32
	if (select(NULL, &fdset, NULL, NULL, &timeout) > 0)//Wait for data
#elif LINUX
	if (select(max + 1, &fdset, NULL, NULL, &timeout) > 0)//Wait for data
#endif
	{//At least one socket has data
		if (FD_ISSET(m_ServerSocket.GetSocket(), &fdset))
			AcceptClients();//check for connects

		//Check all clients in fdset for data

		for (auto it = m_Clients.begin(); it != m_Clients.end();)
		{
			if (!FD_ISSET(it->GetSocket(), &fdset))
			{
				++it;
				continue;
			}

			const bool connected = it->Recv();

			if (!connected)
				it = m_Clients.erase(it);
			else
				++it;
		}
	}*/

	/*for (auto it = m_Clients.begin(); it != m_Clients.end();)
	{
		const bool connected = it->Recv();

		if (!connected)
			it = m_Clients.erase(it);
		else
			++it;
	}

	AcceptClients();*/


	while (m_ServerSocket.RecvFromAny())
	{
		auto address = m_ServerSocket.GetLastAddressRecvFrom();
		auto client  = m_Clients.find(address);

		if (client == m_Clients.end())//New client?
			m_Clients.insert({ address, m_ServerSocket });
		else
			client->second << m_ServerSocket;//Move data to client
	}
}