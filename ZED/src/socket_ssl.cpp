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
#include <openssl/ssl.h>
#include "../include/socket_ssl.h"
#include "../include/log.h"


#ifdef _WIN32
#pragma comment(lib, "libssl_static.lib")
#pragma comment(lib, "libcrypto_static.lib")
#pragma comment(lib, "Crypt32.lib")
#endif


using namespace ZED;



/*SocketSSL::SocketSSL()
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



SocketSSL::SocketSSL(int Socket)
{
	m_Socket = Socket;

	u_long on_mode = 1;
#ifdef _WIN32
	ioctlsocket(m_Socket, FIONBIO, &on_mode);
#else		
	ioctl(m_Socket, FIONBIO, &on_mode);
#endif
}*/



SocketSSL::SocketSSL(const SocketTCP& Original)
	: SocketTCP(Original)
{
	if (SocketTCP::IsConnected())
	{
		CreateSSLContext();
		SSL_connect(m_SSL);
	}
}



SocketSSL::~SocketSSL()
{
	if (m_SSL)
	{
		SSL_shutdown(m_SSL);
		SSL_free(m_SSL);
		m_SSL = nullptr;
	}
	if (m_SSL_Context)
	{
		SSL_CTX_free(m_SSL_Context);
		m_SSL_Context = nullptr;
	}
}



bool SocketSSL::Connect(const char* Host, uint16_t Port)
{
	if (!SocketTCP::Connect(Host, Port))
		return false;

	if (!CreateSSLContext())
		return false;

	if (SSL_connect(m_SSL) < 0)
		return false;

	return true;
}



bool SocketSSL::CreateSSLContext()
{
	if (!SocketTCP::IsConnected())
		return false;

	m_SSL_Context = SSL_CTX_new(TLS_client_method());
	if (!m_SSL_Context)
		return false;

	m_SSL = SSL_new(m_SSL_Context);
	if (!m_SSL)
		return false;

	if (SSL_set_fd(m_SSL, m_Socket) == 0)
		return false;

	MakeBlocking(true);

	return true;
}



bool SocketSSL::Listen(uint16_t Port)
{
	if (!SocketTCP::Listen(Port))
		return false;

	return CreateSSLContext();
}



bool SocketSSL::Send(const void* Data, uint32_t Size)
{
	if (Size == 0) return true;

	int ret = SSL_write(m_SSL, Data, Size);

	if (ret <= 0)
		return false;

	return ret == Size;
}



Socket* SocketSSL::AcceptClient() const
{
	Socket* new_socket = SocketTCP::AcceptClient();
	if (!new_socket)
		return nullptr;

	SocketSSL* ret = new SocketSSL;
	ret->m_Socket  = new_socket->GetSocket();
	ret->m_SSL = SSL_new(m_SSL_Context);

	delete new_socket;

	if (ret->m_SSL <= 0)
	{
		delete ret;
		return nullptr;
	}

	return ret;
}



bool SocketSSL::Recv()
{
	int rcv_amt = SSL_read(m_SSL, &m_Buffer[m_BufferPosition], BufferSize - m_BufferPosition);

	if (rcv_amt < 0)//Error?
	{
		return false;//Disconnected
	}
	else
		m_BufferPosition += rcv_amt;

	return true;
}