#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>
#include "../../ZED/include/blob.h"



struct mg_context;
struct mg_connection;
enum mg_event : uint32_t;



class HttpServer
{
public:
	enum class ResourceType
	{
		Plain,
		HTML,
		CSS,
		JavaScript,
		Image_JPG,
		Image_PNG,
		Binary//Arbitrary binary data (to force downloads)
	};

	struct RequestHeader
	{
		RequestHeader(const std::string& Name_, const std::string& Value_) : Name(Name_), Value(Value_) {}
		operator const std::string& () const { return Value; }

		std::string Name;
		std::string Value;
	};

	struct RequestInfo
	{
		const RequestHeader* FindHeader(const std::string& Name) const
		{
			for (auto& header : m_Headers)
				if (header.Name == Name)
					return &header;
			return nullptr;
		}

		long RemoteIP = 0;
		uint16_t RemotePort = 0;

		std::vector<RequestHeader> m_Headers;
	};

	struct Request
	{
		Request(const Request& Org)//Deep copy
			: m_Query(Org.m_Query), m_Body(Org.m_Body, Org.m_Body.GetSize()), m_RequestInfo(Org.m_RequestInfo), m_ResponseHeader(Org.m_ResponseHeader) {
		}

		Request(const std::string& Query) : m_Query(Query) {}
		Request(const std::string& Query, ZED::Blob&& Body) : m_Query(Query), m_Body(std::move(Body)) {}
		Request(const std::string& Query, const std::string& Body) : Request(Query, ZED::Blob(Body)) {}
		Request(const std::string& Query, ZED::Blob&& Body,  const RequestInfo& RequestInfo) : m_Query(Query), m_Body(std::move(Body)), m_RequestInfo(RequestInfo) {}
		Request(const std::string& Query, std::string& Body, const RequestInfo& RequestInfo) : Request(Query, ZED::Blob(Body), RequestInfo) {}

		std::string m_Query;
		ZED::Blob m_Body;
		const RequestInfo m_RequestInfo;

		mutable std::string m_ResponseHeader;
	};

	HttpServer() : m_Port(0) {}
	HttpServer(HttpServer&) = delete;
	HttpServer(const HttpServer&) = delete;
	HttpServer(HttpServer&& rhs) = delete;
	~HttpServer();

	void Start(uint16_t Port);

	[[nodiscard]]
	uint16_t GetPort() const { return m_Port; }

	[[nodiscard]]
	static const std::string LoadFile(const std::string& Filename);

	[[nodiscard]]
	static std::string DecodeURLEncoded(const ZED::Blob& Input, const char* VariableName);
	[[nodiscard]]
	static std::string DecodeURLEncoded(const ZED::Blob& Input, const std::string& VariableName) {
		return DecodeURLEncoded(Input, VariableName.c_str());
	}

	[[nodiscard]]
	static std::string DecodeURLEncoded(const std::string& Input, const char* VariableName);
	[[nodiscard]]
	static std::string DecodeURLEncoded(const std::string& Input, const std::string& VariableName) {
		return DecodeURLEncoded(Input, VariableName.c_str());
	}

	[[nodiscard]]
	static const std::string CookieHeader(const std::string& Name, const std::string& Value, const std::string& Location = "/");

	[[nodiscard]]
	bool IsRunning() const { return m_Context; }

	[[nodiscard]]
	uint32_t GetWorkerCount() const;

	[[nodiscard]]
	uint32_t GetFreeWorkerCount() const;

	void IncreaseWorkerCount(uint32_t Amount = 1);
		
	//void RegisterResource(const std::string& URI, std::string (*Callback)(const std::string Query), ResourceType Type = ResourceType::HTML);
	//void RegisterResource(const std::string& URI, std::function<std::string(Request&)> Callback, ResourceType Type = ResourceType::HTML);
	void RegisterResource(const std::string& URI, std::function<ZED::Blob(Request&)> Callback, ResourceType Type = ResourceType::HTML, uint32_t Cache = 0);

private:

	struct Resource
	{
		Resource(ResourceType Type, std::function<ZED::Blob(Request&)> Callback, uint32_t Cache = 0) : m_Callback(Callback)
		{
			m_Type = Type;
			m_CacheAgeInSeconds = Cache;
		}

		ResourceType m_Type;
		uint32_t m_CacheAgeInSeconds = 0;
		std::function<ZED::Blob(Request&)> m_Callback;
	};

	void* Callback(mg_event Event, mg_connection* Connection);


	mg_context* m_Context = nullptr;
	uint16_t m_Port;

	std::unordered_map<std::string, Resource> m_Resources;
};