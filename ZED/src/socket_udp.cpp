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
#include "../include/socket_udp.h"
#include "../include/log.h"


#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif


using namespace ZED;



SocketUDP::SocketUDP()
{
	if (m_Socket != -1)
	{
		Log::Warn("Socket object has already been created");
		return;
	}

#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(0x202, &wsaData);
#endif

	if ((m_Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		Log::Error("Could not create socket!");
		return;
	}
}



bool SocketUDP::Connect(const char* Host, uint16_t Port)
{
	hostent* host = gethostbyname(Host);

	if (!host)
	{
		m_Socket = -1;
		Log::Warn("Could not resolve host");
		return false;
	}

	m_Addr.sin_family = AF_INET;
	m_Addr.sin_port = htons(Port);
	memcpy((char*)&m_Addr.sin_addr, (char*)host->h_addr, host->h_length);

	u_long on_mode = 1;
#ifdef _WIN32
	ioctlsocket(m_Socket, FIONBIO, &on_mode);
#else		
	ioctl(m_Socket, FIONBIO, &on_mode);
#endif
	return true;
}



bool SocketUDP::Listen(uint16_t Port)
{
	m_Addr.sin_family = AF_INET;
	m_Addr.sin_port = htons(Port);
	m_Addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_Socket, (sockaddr*)&m_Addr, sizeof(m_Addr)) == -1)
	{
		Log::Error("Error can not bind to port " + Port);
		return false;
	}

	u_long on_mode = 1;
#ifdef _WIN32
	ioctlsocket(m_Socket, FIONBIO, &on_mode);
#else		
	ioctl(m_Socket, FIONBIO, &on_mode);
#endif
	return true;
}



bool SocketUDP::Send(const void* Data, uint32_t Size)
{
	if (Size == 0) return true;

	int error_no = sendto(m_Socket, (char*)Data, Size, 0, (sockaddr*)&m_Addr, sizeof(m_Addr));

	if (error_no <= 0)
	{
#ifdef _WIN32
		const int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)//10035
			m_Socket = -1;//Mark socket as invalid
#endif
		return false;
	}

	return true;
}



bool SocketUDP::Recv()
{
	sockaddr_in from;
#ifdef _WIN32
	int from_len = sizeof(from);
#else
	unsigned int from_len = sizeof(from);
#endif

	int rcv_amt = recvfrom(m_Socket, &m_Buffer[m_BufferPosition], BufferSize - m_BufferPosition, 0, (sockaddr*)&from, &from_len);

	if (rcv_amt > 0 && from_len == sizeof(sockaddr))
	{
		if (m_Addr.sin_addr.s_addr == 0)//No connection so far
			m_Addr = from;//Connection established
		else if (from.sin_addr.s_addr != m_Addr.sin_addr.s_addr || from.sin_port != m_Addr.sin_port)
		{
			Log::Warn("Received data from unknown address");
			return false;
		}
	}

#ifdef _WIN32
	if (rcv_amt == SOCKET_ERROR)//Error?
	{
		const int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK && error != WSAECONNRESET)//10035
		{
			m_Socket = -1;
			return false;//Disconnected
		}
	}
#else
	if (rcv_amt == -1)//Error?
	{
		if (errno != EWOULDBLOCK)
		{
			m_Socket = -1;
			return false;//Disconnected
		}
	}
#endif
	else
		m_BufferPosition += rcv_amt;

	return true;
}


bool SocketUDP::RecvFromAny()
{
#ifdef _WIN32
	int from_len = sizeof(m_Addr);
#else
	unsigned int from_len = sizeof(m_Addr);
#endif

	int rcv_amt = recvfrom(m_Socket, &m_Buffer[m_BufferPosition], BufferSize - m_BufferPosition, 0, (sockaddr*)&m_Addr, &from_len);

#ifdef _WIN32
	if (rcv_amt == SOCKET_ERROR)//Error?
	{
		const int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK && error != WSAECONNRESET)//10035
			m_Socket = -1;
		return false;
	}
#else
	if (rcv_amt == -1)//Error?
	{
		if (errno != EWOULDBLOCK)
			m_Socket = -1;
		return false;
	}
#endif
	else
		m_BufferPosition += rcv_amt;

	return true;
}



uint32_t SocketUDP::GetLastAddressRecvFrom() const
{
	return m_Addr.sin_addr.s_addr;
}