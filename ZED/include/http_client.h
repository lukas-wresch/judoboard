#pragma once
#include <string>
#include "socket_tcp.h"
#include "blob.h"



namespace ZED
{
	class HttpClient
	{
	public:
		struct Packet
		{
			Packet(const std::string& Header_, const std::string& Body_) : header(Header_), body(Body_) {}

			operator const std::string& () const { return body; }

			std::string header;
			std::string body;
		};

		HttpClient(const char* Host, uint16_t Port = 80) : m_Socket(), m_Hostname(Host) { m_Socket.Connect(Host, Port); }
		HttpClient(const std::string& Host, uint16_t Port = 80) : HttpClient(Host.c_str(), Port) {}

		HttpClient(HttpClient&) = delete;
		HttpClient(const HttpClient&) = delete;

		bool IsConnected() const { return m_Socket.IsConnected(); }
		const std::string& GetHostname() const { return m_Hostname; }

		DLLEXPORT bool SendGETRequest(const char* Path, const char* AdditionalHeaders = nullptr);
		DLLEXPORT bool SendPOSTRequest(const char* Path, const Blob& Data, const char* AdditionalHeaders = nullptr);
		DLLEXPORT bool SendFile(const char* Path, const char* Filename);

		DLLEXPORT Packet RecvResponse();

		Packet GET(const char* Path, const char* AdditionalHeaders = nullptr) { SendGETRequest(Path, AdditionalHeaders); return RecvResponse(); }
		Packet GET(const std::string& Path, const std::string& AdditionalHeaders = "") { SendGETRequest(Path.c_str(), AdditionalHeaders.c_str()); return RecvResponse(); }

		Packet POST(const std::string& Path, const Blob& Data, const std::string& AdditionalHeaders = "") { SendPOSTRequest(Path.c_str(), Data, AdditionalHeaders.c_str()); return RecvResponse(); }
		Packet POST(const std::string& Path, const std::string& Data, const std::string& AdditionalHeaders = "") { SendPOSTRequest(Path.c_str(), Blob(Data), AdditionalHeaders.c_str()); return RecvResponse(); }

		void SetKeepAlive(bool Enable = true) { m_KeepAlive = Enable; }

	private:
		SocketTCP m_Socket;

		std::string m_Hostname;

		bool m_KeepAlive = false;
	};
}