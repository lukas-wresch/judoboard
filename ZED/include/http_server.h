#pragma once
#include <thread>
#include "core.h"
#include "socket.h"



namespace ZED
{
	class HttpServer
	{
	public:
		//ServerTCP(uint16_t Port) : Server<SocketTCP>(Port) {}

		void AddListeningPort(Socket* NewSocket, uint16_t Port);
		bool Start();

	private:
		std::vector<Socket*> m_ListeningSockets;
		std::thread m_Thread;

		volatile bool m_Running = true;
	};
}