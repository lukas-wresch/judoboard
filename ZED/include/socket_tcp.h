#pragma once
#include "core.h"
#include "socket.h"



namespace ZED
{
	class DLLEXPORT SocketTCP : public Socket
	{
	public:
		SocketTCP();
		SocketTCP(const SocketTCP& Original)
		{
			m_Socket = Original.m_Socket;
		}
		SocketTCP(SocketTCP&& Original) noexcept
		{
			m_Socket = Original.m_Socket;
			Original.m_Socket = -1;
		}

		void operator =(const SocketTCP& rhs)
		{
			m_Socket = rhs.m_Socket;
		}

		bool Connect(const char* Host, uint16_t Port) override;
		bool Connect(const std::string& Host, uint16_t Port) { return Connect(Host.c_str(), Port); }

		virtual bool Listen(uint16_t Port) override;

		bool IsConnected() const { return m_Socket != -1; }

		const char* GetBuffer() const { return m_Buffer; }
		uint32_t GetBufferLength() const { return m_BufferPosition; }
		void ClearBuffer() { m_BufferPosition = 0; }
		std::string Buffer2String()
		{
			std::string ret(GetBuffer(), GetBufferLength());
			ClearBuffer();
			return ret;
		}

		bool Send(const void* Data, uint32_t Size);
		template <typename T>
		bool Send(const T& Data) { return Send((const char*)&Data, sizeof(T)); }

		SocketTCP AcceptClients();
		bool Recv();


		operator std::string ()
		{
			return Buffer2String();
		}


		static constexpr uint32_t BufferSize = 1024*4;

	private:
		SocketTCP(int Socket);

		char m_Buffer[BufferSize] = {};
		uint32_t m_BufferPosition = 0;
	};
}