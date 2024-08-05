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
#include "../include/socket_tcp.h"
#include "../include/log.h"


#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif


using namespace ZED;



SocketTCP::SocketTCP()
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

	if ((m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		Log::Error("Could not create socket!");
		return;
	}
}



SocketTCP::SocketTCP(int Socket)
{
	m_Socket = Socket;
}



bool SocketTCP::Connect(const char* Host, uint16_t Port)
{
	hostent* host = gethostbyname(Host);

	if (!host)
	{
		m_Socket = -1;
		Log::Warn("Could not resolve host");
		return false;
	}

	sockaddr_in c_in;
	memcpy((char*)&c_in.sin_addr, (char*)host->h_addr, host->h_length);
	c_in.sin_family = AF_INET;
	c_in.sin_port = htons(Port);

	if (connect(m_Socket, (sockaddr*)&c_in, sizeof c_in) < 0)
	{
		Log::Warn("Could not connect to host");
		m_Socket = -1;
		return false;
	}

	return true;
}



bool SocketTCP::Listen(uint16_t Port)
{
	sockaddr_in si_me;
	memset((char*)&si_me, 0x00, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(Port);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(m_Socket, (sockaddr*)&si_me, sizeof(si_me)) == -1)
	{
		m_Socket = -1;
		Log::Error("Error can not bind to port " + Port);
		return false;
	}

	if (listen(m_Socket, 16) == -1)//Backlog
	{
		m_Socket = -1;
		Log::Error("Error can not listen to port " + Port);
		return false;
	}

	return true;
}



bool SocketTCP::Send(const void* Data, uint32_t Size)
{
	if (Size == 0) return true;

	int error_no = send(m_Socket, (char*)Data, Size, 0);

	if (error_no <= 0)
	{
#ifdef _WIN32
		const int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)//10035
			m_Socket = -1;//Mark socket as invalid
#else
		//if (errno != EWOULDBLOCK)
#endif
		
		return false;
	}

	return error_no == Size;
}



Socket* SocketTCP::AcceptClient() const
{
	sockaddr_in addr;
	int addr_len = sizeof(addr);
	int new_socket = accept(m_Socket, (sockaddr*)&addr, &addr_len);

	if (new_socket == -1)
		return nullptr;

	//accept
	if (new_socket != -1)//got one, so lets put him in
	{
		/*int so_sndbuf_amount = 200000;
#ifdef _WIN32
		int c_temp_addrlen;
		int tst = sizeof(so_sndbuf_amount);
#else
		socklen_t c_temp_addrlen;
		socklen_t tst = sizeof(so_sndbuf_amount);
#endif
		if (setsockopt(temp_socket, SOL_SOCKET, SO_SNDBUF, (char*)&so_sndbuf_amount, sizeof(so_sndbuf_amount)) == -1)
			Log::Warn("Error setting SO_SNDBUF");*/
	}

	return new SocketTCP(new_socket);
}



bool SocketTCP::Recv()
{
	int rcv_amt = recv(m_Socket, &m_Buffer[m_BufferPosition], BufferSize - m_BufferPosition, 0);

#ifdef _WIN32
	if (rcv_amt == SOCKET_ERROR)//Error?
	{
		const int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)//10035
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