#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include "../include/server.h"
#include "../include/log.h"


#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif


using namespace ZED;



template <class  T>
Server<T>::Server(uint16_t Port)
{
	m_ServerSocket.Listen(Port);
}



/*template <class  T>
void Server<T>::Disconnect()
{
	for (auto& client : m_Clients)
		client.Disconnect();

	m_ServerSocket.Disconnect();
}*/



template class Server<SocketTCP>;
template class Server<SocketUDP>;