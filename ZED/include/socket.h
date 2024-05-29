#pragma once
#include "core.h"



namespace ZED
{
	class DLLEXPORT Socket
	{
	public:
		Socket(Socket&) = delete;
		Socket(const Socket&) = delete;
		virtual ~Socket() { Disconnect(); }

		int GetSocket() { return m_Socket; }

		void MakeBlocking(bool Enable = true);
		void SetTimeout(int Seconds);//Sets timeout for send and receive operations for blocking sockets

		virtual bool Connect(const char* Host, uint16_t Port) = 0;
		bool Connect(const std::string& Host, uint16_t Port) { return Connect(Host.c_str(), Port); }

		virtual bool IsConnected() const = 0;

		virtual bool Listen(uint16_t Port) = 0;

		virtual Socket* AcceptClient() const = 0;

		bool IsValid() const { return m_Socket != -1; }

		virtual bool Send(const void* Data, uint32_t Size) = 0;
		bool SendText(const std::string& Data) { return Send(Data.c_str(), Data.length()); }
		template <typename T>
		bool Send(const T& Data) { return Send((const char*)&Data, sizeof(T)); }

		virtual bool Recv() = 0;

		void Disconnect();

	protected:
		Socket() = default;

		int m_Socket = -1;
	};
}