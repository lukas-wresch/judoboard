#pragma once
#include <thread>
#include "core.h"
#include "socket_tcp.h"
#include "thread_pool.h"



namespace ZED
{
	class HttpServer
	{
	public:
		struct HeaderValue
		{
			HeaderValue(const std::string& Name_, const std::string& Value_) : Name(Name_), Value(Value_) {}
			operator const std::string& () const { return Value; }

			std::string Name;
			std::string Value;
		};



		struct RequestHeader
		{
			const HeaderValue* FindHeader(const std::string& Name) const
			{
				for (auto& header : m_Fields)
					if (header.Name == Name)
						return &header;
				return nullptr;
			}

			void Add(const std::string& Key, const std::string& Value)
			{
				m_Fields.emplace_back(Key, Value);
			}

			std::vector<HeaderValue> m_Fields;

			enum class Method {
				Get, Post
			} method;
			std::string url;
			std::string protocol;//HTTP/1.1

			long RemoteIP = 0;
			uint16_t RemotePort = 0;
		};



		struct ResponseHeader
		{
			const HeaderValue* FindHeader(const std::string& Name) const
			{
				for (auto& header : m_Fields)
					if (header.Name == Name)
						return &header;
				return nullptr;
			}

			void Add(const std::string& Key, const std::string& Value)
			{
				m_Fields.emplace_back(Key, Value);
			}

			std::vector<HeaderValue> m_Fields;
		};



		DLLEXPORT HttpServer(size_t WorkerCount);
		DLLEXPORT ~HttpServer();

		DLLEXPORT void AddListeningPort(SocketTCP* NewSocket, uint16_t Port);
		DLLEXPORT bool Start();

	private:
		void SendResponse(SocketTCP* Client, const ResponseHeader& Header, const std::string& Body) const;
		void HandleClient(SocketTCP* Client) const;

		std::vector<SocketTCP*> m_ListeningSockets;
		std::thread m_Thread;

		ThreadPool m_ThreadPool;

		volatile bool m_Running = true;
	};
}