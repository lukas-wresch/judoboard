#pragma once
#include "core.h"
#include "socket_tcp.h"
#include "server.h"



namespace ZED
{
	class ServerTCP : public Server<SocketTCP>
	{
	public:
		ServerTCP(uint16_t Port) : Server<SocketTCP>(Port) {}

		/*bool Listen(uint16_t Port) {
			return Socket<SocketTCP>::Listen(Port);
		}*/

		const std::vector<SocketTCP>& GetClients() const { return m_Clients; }
		std::vector<SocketTCP>& GetClients() { return m_Clients; }

		SocketTCP GetClient(uint32_t Index) const
		{
			return m_Clients[Index];
		}

		void Broadcast(const void* Data, uint32_t Size)
		{
			for (auto& client : m_Clients)
				client.Send(Data, Size);
		}
		template <typename T>
		void Broadcast(const T& Data) { return Broadcast((const char*)&Data, sizeof(T)); }

		DLLEXPORT virtual bool AcceptClients();
		DLLEXPORT virtual void Recv() override;

	private:
		std::vector<SocketTCP> m_Clients;
	};
}