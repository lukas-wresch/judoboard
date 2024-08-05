#pragma once
#include <string>
#include "http_client.h"
#include "socket_ssl.h"



namespace ZED
{
	class HttpsClient : public HttpClient
	{
	public:
		HttpsClient(const char* Host, uint16_t Port = 443) : HttpClient(Host, Port), m_SSL_Socket(m_Socket) {}
		HttpsClient(const std::string& Host, uint16_t Port = 443) : HttpsClient(Host.c_str(), Port) {}

		//HttpsClient(HttpClient&) = delete;
		//HttpsClient(const HttpClient&) = delete;

		//bool IsConnected() const { return m_Socket.IsConnected(); }

		//DLLEXPORT bool SendGETRequest(const char* Path, const char* AdditionalHeaders = nullptr);
		//DLLEXPORT bool SendPOSTRequest(const char* Path, const Blob& Data, const char* AdditionalHeaders = nullptr);
		//DLLEXPORT bool SendFile(const char* Path, const char* Filename);

		//DLLEXPORT Packet RecvResponse();

		Packet GET(const char* Path, const char* AdditionalHeaders = nullptr) { SendGETRequest(Path, AdditionalHeaders); return RecvResponse(); }
		Packet GET(const std::string& Path, const std::string& AdditionalHeaders = "") { SendGETRequest(Path.c_str(), AdditionalHeaders.c_str()); return RecvResponse(); }

		Packet POST(const std::string& Path, const Blob& Data, const std::string& AdditionalHeaders = "") { SendPOSTRequest(Path.c_str(), Data, AdditionalHeaders.c_str()); return RecvResponse(); }
		Packet POST(const std::string& Path, const std::string& Data, const std::string& AdditionalHeaders = "") { SendPOSTRequest(Path.c_str(), Blob(Data), AdditionalHeaders.c_str()); return RecvResponse(); }

	private:
		virtual SocketTCP& GetSocket() override { return m_SSL_Socket; }

		SocketSSL m_SSL_Socket;
	};
}