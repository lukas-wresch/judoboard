#pragma once
#include <unordered_map>
#include "core.h"
#include "server.h"
#include "socket_udp.h"



namespace ZED
{
	class ServerUDP : public Server<SocketUDP>
	{
	public:
		ServerUDP(uint16_t Port) : Server<SocketUDP>(Port) {}

		std::unordered_map<uint32_t, SocketUDP>& GetClients() { return m_Clients; }
		SocketUDP* GetClient(uint32_t Index)
		{
			for (auto& client : m_Clients)
			{
				if (Index == 0)
					return &client.second;
				Index--;
			}
			return nullptr;
		}

		void Broadcast(const void* Data, uint32_t Size)
		{
			for (auto& client : m_Clients)
				client.second.Send(Data, Size);
		}
		template <typename T>
		void Broadcast(const T& Data) { return Broadcast((const char*)&Data, sizeof(T)); }

		DLLEXPORT virtual void Recv() override;

	private:
		std::unordered_map<uint32_t, SocketUDP> m_Clients;
	};
}