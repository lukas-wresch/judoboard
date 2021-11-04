#pragma once
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
#include "core.h"
#include "socket.h"



namespace ZED
{
	class DLLEXPORT SocketUDP : public Socket
	{
	public:
		SocketUDP();
		SocketUDP(SocketUDP& Original)
		{
			m_Socket = Original.m_Socket;
			m_Addr   = Original.m_Addr;
			Original.m_Addr.sin_addr.s_addr = 0;

			memcpy(&m_Buffer, &Original.m_Buffer, Original.m_BufferPosition);
			m_BufferPosition = Original.m_BufferPosition;

			Original.m_BufferPosition = 0;
		}
		SocketUDP(const SocketUDP& Original)
		{
			m_Socket = Original.m_Socket;
			m_Addr   = Original.m_Addr;
			memcpy(&m_Buffer, &Original.m_Buffer, Original.m_BufferPosition);
			m_BufferPosition = Original.m_BufferPosition;
		}
		SocketUDP(SocketUDP&& Original) noexcept : m_Addr(Original.m_Addr)
		{
			m_Socket = Original.m_Socket;
			m_Addr   = Original.m_Addr;
			Original.m_Addr.sin_addr.s_addr = 0;

			memcpy(&m_Buffer, &Original.m_Buffer, Original.m_BufferPosition);
			m_BufferPosition = Original.m_BufferPosition;

			Original.m_Socket = -1;
			Original.m_BufferPosition = 0;
		}

		void operator =(const SocketUDP& rhs)
		{
			m_Socket = rhs.m_Socket;
			m_Addr   = rhs.m_Addr;
			memcpy(&m_Buffer, &rhs.m_Buffer, rhs.m_BufferPosition);
			m_BufferPosition = rhs.m_BufferPosition;
		}

		bool operator <<(SocketUDP& rhs)
		{
			m_Addr = rhs.m_Addr;
			if (m_BufferPosition + rhs.m_BufferPosition < BufferSize)//Is memory is available
			{
				memcpy(&m_Buffer[m_BufferPosition], &rhs.m_Buffer, rhs.m_BufferPosition);
				m_BufferPosition += rhs.m_BufferPosition;
				rhs.m_BufferPosition = 0;
				return true;
			}
			return false;
		}

		virtual bool Connect(const char* Host, uint16_t Port) override;
		bool Connect(const std::string& Host, uint16_t Port) { return Connect(Host.c_str(), Port); }

		virtual bool Listen(uint16_t Port) override;		

		bool IsConnected() const { return m_Socket != -1 && m_Addr.sin_addr.s_addr != 0; }

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
		template <typename T>
		bool Send(const T& Data) { return Send((const char*)&Data, sizeof(T)); }

		bool Recv();
		bool RecvFromAny();

		uint32_t GetLastAddressRecvFrom() const;

		static constexpr uint32_t BufferSize = 1024;

	private:
		char m_Buffer[BufferSize] = {};
		uint32_t m_BufferPosition = 0;

		sockaddr_in m_Addr = {};
	};
}