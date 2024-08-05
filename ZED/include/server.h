#pragma once
#include <cassert>
#include "core.h"
#include "socket_tcp.h"
#include "socket_udp.h"



namespace ZED
{
	template <class  T>
	class Server
	{
	public:
		DLLEXPORT Server(uint16_t Port);
		~Server() { Disconnect(); }

		bool Listen(uint16_t Port) { return m_ServerSocket.Listen(Port); }

		void MakeBlocking(bool Enable = true) { m_ServerSocket.MakeBlocking(Enable); }

		bool IsValid() const { return m_ServerSocket.IsValid(); }

		//const std::vector<T>& GetClients() const { return m_Clients; }
		//std::vector<T>& GetClients() { return m_Clients; }

		/*T GetClient(uint32_t Index) const
		{
			return m_Clients[Index];
		}*/

		DLLEXPORT virtual void Recv() = 0;

		DLLEXPORT void Disconnect() { assert(false); }//NOT IMPLEMENTED

	protected:

		T m_ServerSocket;
	};
}