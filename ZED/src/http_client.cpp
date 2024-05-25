#include <stdio.h>
#include <fstream>
#include "../include/http_client.h"
#include "../include/log.h"


using namespace ZED;



bool HttpClient::SendGETRequest(const char* Path, const char* AdditionalHeaders)
{
	if (!GetSocket().IsConnected())
		return false;
	

	char request[1024] = {};
	strcpy_s(request, sizeof(request), "GET ");
	strcat_s(request, sizeof(request), Path);
	strcat_s(request, sizeof(request), " HTTP/1.1\r\nHost: ");
	strcat_s(request, sizeof(request), m_Hostname.c_str());

	if (AdditionalHeaders)
	{
		strcat_s(request, sizeof(request), "\r\n");
		strcat_s(request, sizeof(request), AdditionalHeaders);
	}

	if (m_KeepAlive)
		strcat_s(request, sizeof(request), "\r\nConnection: keep-alive\r\n\r\n");
	else
		strcat_s(request, sizeof(request), "\r\nConnection: close\r\n\r\n");

	return GetSocket().Send(request, strlen(request));
}



bool HttpClient::SendPOSTRequest(const char* Path, const Blob& Data, const char* AdditionalHeaders)
{
	if (!GetSocket().IsConnected())
		return false;


	char request[1024] = {};
	strcpy_s(request, sizeof(request), "POST ");
	strcat_s(request, sizeof(request), Path);
	strcat_s(request, sizeof(request), " HTTP/1.1\r\nHost: ");
	strcat_s(request, sizeof(request), m_Hostname.c_str());
	strcat_s(request, sizeof(request), "\r\nContent-Length: ");
	strcat_s(request, sizeof(request), std::to_string(Data.GetSize()).c_str());

	if (AdditionalHeaders)
	{
		strcat_s(request, sizeof(request), "\r\n");
		strcat_s(request, sizeof(request), AdditionalHeaders);
	}

	if (m_KeepAlive)
		strcat_s(request, sizeof(request), "\r\nConnection: keep-alive\r\n\r\n");
	else
		strcat_s(request, sizeof(request), "\r\nConnection: close\r\n\r\n");

	if (!GetSocket().Send(request, strlen(request)))
		return false;

	const int packet_size = 1024;
	for (size_t bytes_sent = 0; bytes_sent < Data.GetSize();)
	{
		int bytes_to_send = packet_size;
		if (bytes_sent + packet_size > Data.GetSize())
			bytes_to_send = Data.GetSize() - bytes_sent;

		if (!GetSocket().Send(&Data[bytes_sent], bytes_to_send))
			return false;

		bytes_sent += bytes_to_send;
	}

	return true;
}



bool HttpClient::SendFile(const char* Path, const char* Filename)
{
	std::ifstream file(Filename, std::ios::binary);

	if (!file)
		return false;

	file.seekg(0, std::ios::end);
	auto length = file.tellg();
	file.seekg(0, std::ios::beg);

	char request[1024] = {};
	strcpy_s(request, sizeof(request), "GET ");
	strcat_s(request, sizeof(request), Path);
	strcat_s(request, sizeof(request), " HTTP/1.1\r\nHost: ");
	strcat_s(request, sizeof(request), m_Hostname.c_str());
	strcat_s(request, sizeof(request), "\r\nContent-Length: ");
	strcat_s(request, sizeof(request), std::to_string(length).c_str());

	if (m_KeepAlive)
		strcat_s(request, sizeof(request), "\r\nConnection: keep-alive\r\n\r\n");
	else
		strcat_s(request, sizeof(request), "\r\nConnection: close\r\n\r\n");

	if (!GetSocket().Send(request, strlen(request)))
		return false;

	while (file)
	{
		char buffer[1024];
		//file.read(buffer, 1024);

		auto read = file.readsome(buffer, 1024);

		//if (!GetSocket().Send(buffer, (uint32_t)file.gcount()))
		if (!GetSocket().Send(buffer, (uint32_t)read))
			return false;
	}

	return true;
}



HttpClient::Packet HttpClient::RecvResponse()
{
	auto& socket = GetSocket();

	if (!socket.IsConnected())//Not connected
		return Packet("", "");//Return empty packet

	std::string response;
	int content_length = -1;
	int header_length  = -1;
	uint32_t downloaded = 0;

	while (socket.Recv())
	{
		const char* buffer = socket.GetBuffer();
		int end = socket.GetBufferLength();

		if (end == 0)
			break;

		response   += socket;
		downloaded += end;

		if (content_length == -1)
		{
			auto pos = response.find("Content-Length: ");

			if (pos != std::string::npos)
			{
				pos += sizeof("Content-Length: ") - 1;
				content_length = Core::ToInt(&response[pos]);
			}
		}

		if (header_length == -1)
		{
			auto pos = response.find("\r\n\r\n");

			if (pos != std::string::npos)
				header_length = pos + sizeof("\r\n\r\n") - 1;
		}

		if (header_length != -1 && content_length != -1 &&
			downloaded >= (uint32_t)header_length + (uint32_t)content_length)
			break;
	}

	if (header_length == -1)
		return Packet("", "");

	return Packet(std::string(response.c_str(), header_length), std::string(response.c_str() + header_length));
	//return Packet(std::string(response.c_str(), header_length), std::string(response.c_str() + header_length, content_length));
}