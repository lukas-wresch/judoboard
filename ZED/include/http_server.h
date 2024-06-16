#pragma once
#include <thread>
#include <fstream>
#include <sstream>
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



		struct Response
		{
			enum class MIMEType
			{
				HTML,
				CSS,
				JavaScript,
				Image_JPG,
				Image_PNG,
				Binary
			};


			const HeaderValue* FindHeader(const std::string& Name) const
			{
				for (auto& field : m_Header)
					if (field.Name == Name)
						return &field;
				return nullptr;
			}

			void Add(const std::string& Key, const std::string& Value)
			{
				m_Header.emplace_back(Key, Value);
			}

			void SetMIMEType(MIMEType Type)
			{
				if (Type == MIMEType::HTML)
					Add("Content-Type", "text/html");
				else if (Type == MIMEType::CSS)
					Add("Content-Type", "text/css");
				else if (Type == MIMEType::JavaScript)
					Add("Content-Type", "text/javascript");
				else if (Type == MIMEType::Image_JPG)
					Add("Content-Type", "image/jpeg");
				else if (Type == MIMEType::Image_PNG)
					Add("Content-Type", "image/png");
				else if (Type == MIMEType::Binary)
					Add("Content-Type", "application/octet-stream");
				else
					assert(false);
			}

			void SetCacheControl(uint32_t TimeInSeconds)
			{
				Add("Cache-Control", "max-age="+std::to_string(TimeInSeconds));
			}

			void LoadFile(const std::string& Filename)
			{
				std::ifstream file(Filename, std::ios::binary);//open the input file

				if (file)
				{
					std::stringstream strStream;
					strStream << file.rdbuf();//read the file
					m_Body = std::move(strStream.str());//str holds the content of the file
				}
			}

			std::vector<HeaderValue> m_Header;
			ZED::Blob m_Body;
		};



		DLLEXPORT HttpServer(size_t WorkerCount);
		DLLEXPORT ~HttpServer();

		DLLEXPORT void AddListeningPort(SocketTCP* NewSocket, uint16_t Port);
		DLLEXPORT bool Start();

		void RegisterResource(const std::string& URI, std::function<Response(RequestHeader&)> Callback)
		{
			m_Resources.insert({ URI, Callback });
		}

	private:
		void SendResponse(SocketTCP* Client, const Response& Header) const;
		void HandleClient(SocketTCP* Client) const;

		std::vector<SocketTCP*> m_ListeningSockets;
		std::thread m_Thread;

		ThreadPool m_ThreadPool;

		std::unordered_map<std::string, std::function<Response(RequestHeader&)>> m_Resources;

		volatile bool m_Running = true;
	};
}