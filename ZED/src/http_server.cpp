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
#include <cassert>
#include "../include/http_server.h"
#include "../include/log.h"


using namespace ZED;



HttpServer::HttpServer(size_t WorkerCount) : m_ThreadPool(1)
{
}



void HttpServer::AddListeningPort(SocketTCP* NewSocket, uint16_t Port)
{
	if (NewSocket)
	{
		NewSocket->Listen(Port);
		NewSocket->MakeBlocking(true);
		m_ListeningSockets.emplace_back(NewSocket);
	}
}



void HttpServer::SendResponse(SocketTCP* Client, const Response& Response) const
{
	assert(!Response.m_Header.empty());

	std::string data = "HTTP/1.1 200 OK\r\n";
	for (auto& field : Response.m_Header)
	{
		data += field.Name + ": " + field.Value + "\r\n";
	}

	data += "\r\n";
	Client->SendText(data);

	Client->Send(Response.m_Body, Response.m_Body.GetSize());
}



void HttpServer::HandleClient(SocketTCP* Client) const
{
	std::string request;

	int content_length  = -1;
	int header_length   = -1;
	uint32_t downloaded = 0;

	while (Client->Recv())
	{
		const char* buffer = Client->GetBuffer();
		int end = Client->GetBufferLength();

		request    += *Client;
		downloaded += end;

		if (content_length == -1)
		{
			auto pos = request.find("Content-Length: ");

			if (pos != std::string::npos)
			{
				pos += sizeof("Content-Length: ") - 1;
				content_length = Core::ToInt(&request[pos]);
			}
		}

		if (header_length == -1)
		{
			auto pos = request.find("\r\n\r\n");

			if (pos != std::string::npos)
				header_length = pos + sizeof("\r\n\r\n") - 1;
		}

		if (header_length != -1 && content_length != -1 &&
			downloaded >= (uint32_t)header_length + (uint32_t)content_length)
			break;
		if (header_length != -1 && content_length == -1 &&
			downloaded >= (uint32_t)header_length)
			break;
	}


	RequestHeader request_header;

	//Parse request
	const char* req = request.c_str();
	char value[256];
	int  value_index = 0;

	size_t i = 0;
	if (req[0] == 'G' && req[1] == 'E' && req[2] == 'T')
	{
		request_header.method = RequestHeader::Method::Get;
		i += 4;
	}
	else if (req[0] == 'P' && req[1] == 'O' && req[2] == 'S' && req[3] == 'T')
	{
		request_header.method = RequestHeader::Method::Post;
		i += 5;
	}

	for (; req[i]; ++i)
	{
		if (req[i] == ' ')
		{
			value[value_index%256] = '\0';
			request_header.url = std::string(value);
			i++;
			break;
		}
		else
			value[value_index++%256] = req[i];
	}

	value_index = 0;
	for (; req[i]; ++i)
	{
		if (req[i] == '\r' && req[i+1] == '\n')
			break;
		else
			value[value_index++%256] = req[i];
	}

	value[value_index%256] = '\0';
	request_header.protocol = std::string(value);
	value_index = 0;
	i += 2;
	char key[256];
	int  key_index = 0;
	int  mode = 1;//0 none, 1 at key, 2 at value

	for (; req[i]; ++i)
	{
		if (req[i] == '\r' && req[++i] == '\n')
		{
			//End of field
			key[key_index%256] = '\0';
			value[value_index%256] = '\0';
			request_header.Add(key, value);
			key_index = value_index = 0;

			if (req[i+1] == '\r' && req[i+2] == '\n')
				break;

			mode = 1;
			i++;
		}

		if (mode == 1)//At key
		{
			if (req[i] == ':')
			{
				i++;
				mode = 2;
			}
			else
				key[key_index++%256] = req[i];
		}

		else//At value
			value[value_index++%256] = req[i];
	}	


	//Parsing complete, searching for endpoint

	auto item = m_Resources.find(request_header.url);

	if (item != m_Resources.end())
	{
		if (item->second)
		{
			auto content = item->second(request_header);//Execute endpoint
			SendResponse(Client, content);//Return content
		}
	}

	else//No endpoint found
	{
		Response response;
		response.Add("Content-Type", "text/plain");
		response.Add("Connection", "close");
		response.Add("Content-Length", "12");
		response.m_Body = std::string("Hello World!");

		SendResponse(Client, response);
	}

	delete Client;
}



bool HttpServer::Start()
{
	m_Thread = std::thread([this]()
	{
		FD_SET original_fds;
		FD_ZERO(&original_fds);
		for (auto listening_socket : m_ListeningSockets)
			FD_SET(listening_socket->GetSocket(), &original_fds);

		while (m_Running)
		{
			FD_SET read_fds = original_fds;
			if (select(NULL, &read_fds, NULL, NULL, NULL) > 0)
			{
				for (auto listening_socket : m_ListeningSockets)
				{
					if (FD_ISSET(listening_socket->GetSocket(), &read_fds))
					{
						SocketTCP* new_connection = (SocketTCP*)listening_socket->AcceptClient();

						//assert(new_connection);

						if (new_connection)
							m_ThreadPool.Enqueue([this, new_connection]() {
								HandleClient(new_connection); });
					}
				}
			}
		}
	});
	
	return true;
}



HttpServer::~HttpServer()
{
	m_Running = false;

	if (m_Thread.joinable())
		m_Thread.join();
}