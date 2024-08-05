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
		void operator =(SocketTCP&& rhs) noexcept
		{
			m_Socket = rhs.m_Socket;
			rhs.m_Socket = -1;
		}

		bool Connect(const char* Host, uint16_t Port) override;
		bool Connect(const std::string& Host, uint16_t Port) { return Connect(Host.c_str(), Port); }

		virtual bool IsConnected() const override { return m_Socket != -1; }

		virtual bool Listen(uint16_t Port) override;
		virtual Socket* AcceptClient() const override;

		const char* GetBuffer() const { return m_Buffer; }
		uint32_t GetBufferLength() const { return m_BufferPosition; }
		void ClearBuffer() { m_BufferPosition = 0; }
		std::string Buffer2String()
		{
			std::string ret(GetBuffer(), GetBufferLength());
			ClearBuffer();
			return ret;
		}

		virtual bool Send(const void* Data, uint32_t Size) override;

		bool Recv();


		operator std::string ()
		{
			return Buffer2String();
		}


		static constexpr uint32_t BufferSize = 4096;

	protected:
		SocketTCP(int Socket);

		char m_Buffer[BufferSize] = {};
		uint32_t m_BufferPosition = 0;
	};
}