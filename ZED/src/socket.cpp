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



void Socket::MakeBlocking(bool Enable)
{
	u_long on_mode = Enable ? 0 : 1;
#ifdef _WIN32
	ioctlsocket(m_Socket, FIONBIO, &on_mode);
#else		
	ioctl(m_Socket, FIONBIO, &on_mode);
#endif
}



void Socket::SetTimeout(int Seconds)
{
	MakeBlocking(true);//Doesn't make send for non-blocking sockets

	struct timeval timeout;
	timeout.tv_sec  = Seconds;
	timeout.tv_usec = 0;
	setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	setsockopt(m_Socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));
}



void Socket::Disconnect()
{
	if (!IsValid())
		return;

#ifdef _WIN32
	closesocket(m_Socket);
#else
	close(m_Socket);
#endif

	m_Socket = -1;
	//Log::Debug("Socket disconnected");
}