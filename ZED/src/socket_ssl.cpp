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
#include <openssl/err.h>
#include "../include/socket_ssl.h"
#include "../include/log.h"


#ifdef _WIN32
#pragma comment(lib, "libssl_static.lib")
#pragma comment(lib, "libcrypto_static.lib")
#pragma comment(lib, "Crypt32.lib")
#endif


using namespace ZED;



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

	if (!m_SSL_Context && !m_SSL)
	{
		m_SSL_Context = SSL_CTX_new(TLS_client_method());
		if (!m_SSL_Context)
			return false;
	}

	if (!m_SSL)
	{
		m_SSL = SSL_new(m_SSL_Context);
		if (!m_SSL)
			return false;
	}

	if (SSL_set_fd(m_SSL, m_Socket) == 0)
		return false;

	MakeBlocking(true);

	return true;
}



bool SocketSSL::Listen(uint16_t Port)
{
	m_SSL_Context = SSL_CTX_new(TLS_server_method());

	//if (!CreateSSLContext())
		//return false;

	/* set the local certificate from CertFile */
	if (SSL_CTX_use_certificate_file(m_SSL_Context, "cert.pem", SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	/* set the private key from KeyFile (may be the same as CertFile) */
	if (SSL_CTX_use_PrivateKey_file(m_SSL_Context, "key.pem", SSL_FILETYPE_PEM) <= 0)
	{
		ERR_print_errors_fp(stderr);
		return false;
	}
	/* verify private key */
	if (!SSL_CTX_check_private_key(m_SSL_Context))
	{
		fprintf(stderr, "Private key does not match the public certificate\n");
		return false;
	}

	if (!SocketTCP::Listen(Port))
		return false;

	return true;
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
	sockaddr_in addr;
	int addr_len = sizeof(addr);
	int new_socket = accept(m_Socket, (sockaddr*)&addr, &addr_len);

	if (new_socket == -1)
		return nullptr;

	SocketSSL* ret = new SocketSSL;
	ret->m_Socket  = new_socket;
	ret->m_SSL     = SSL_new(m_SSL_Context);

	if (!ret->CreateSSLContext())
	{
		delete ret;
		return nullptr;
	}

	/*if (SSL_use_certificate_chain_file(ret->m_SSL, "E:\\Workspace\\Judoboard\\bin\\cert.pem") == 0)
	{
		delete ret;
		return nullptr;
	}

	if (SSL_use_PrivateKey_file(ret->m_SSL, "E:\\Workspace\\Judoboard\\bin\\key.pem", SSL_FILETYPE_PEM) == 0)
	{
		delete ret;
		return nullptr;
	}

	//Verify that the private key matches the public certificate
	if (!SSL_check_private_key(ret->m_SSL))
	{
		//fprintf(stderr, "Private key does not match the public certificate\n");
		exit(EXIT_FAILURE);
	}*/

	if (SSL_accept(ret->m_SSL) <= 0)
	{
		int err = SSL_get_error(ret->m_SSL, -1);

		switch (err) {
		case SSL_ERROR_NONE:
			// No error
			break;
		case SSL_ERROR_ZERO_RETURN:
			//std::cerr << "SSL connection closed by peer" << std::endl;
			break;
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			// Non-fatal error, try again later
			//std::cerr << "SSL operation did not complete, try again" << std::endl;
			break;
		case SSL_ERROR_SYSCALL:
			char buffer[1024];
			strerror_s(buffer, errno);
			break;
		case SSL_ERROR_SSL:
			//std::cerr << "SSL library error" << std::endl;
			ERR_print_errors_fp(stderr);
			break;
		default:
			//std::cerr << "Unknown SSL error" << std::endl;
			ERR_print_errors_fp(stderr);
			break;
		}


		//delete ret;
		//return nullptr;
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