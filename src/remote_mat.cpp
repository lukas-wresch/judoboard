#include <ZED/include/http_client.h>
#include <ZED/include/log.h>
#include <cassert>
#include "remote_mat.h"
#include "app.h"



using namespace Judoboard;



RemoteMat::RemoteMat(uint32_t ID, const std::string& Host, uint16_t Port) : IMat(ID), m_Hostname(Host), m_Port(Port)
{
	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/set_mat_id?id=" + std::to_string(GetMatID()));
}



bool RemoteMat::IsOpen() const
{
	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/is_mat_open?id=" + std::to_string(GetMatID()));
	return response == "ok";
}



bool RemoteMat::Open()
{
	return SendRequest("/ajax/slave/open_mat?id=" + std::to_string(GetMatID()));
}



bool RemoteMat::Close()
{
	return SendRequest("/ajax/slave/close_mat?id=" + std::to_string(GetMatID()));
}



bool RemoteMat::IsConnected() const
{
	return SendRequest("/ajax/slave/get_mat_status?id=" + std::to_string(GetMatID()));
}



bool RemoteMat::StartMatch(Match* NewMatch)
{
	if (!NewMatch)
	{
		ZED::Log::Warn("Invalid match");
		return false;
	}

	ZED::CSV csv;
	*NewMatch >> csv;

	return SendRequest("/ajax/slave/start_match?id=" + std::to_string(GetMatID()) + "&match=" + (std::string)csv);
}



bool RemoteMat::EndMatch()
{
	return SendRequest("/ajax/slave/end_match?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddIppon(Fighter Whom)
{
	SendRequest("/ajax/mat/" + Fighter2String(Whom) + "/+ippon?id=" + std::to_string(GetMatID()));
}



bool RemoteMat::SendRequest(const std::string& URL) const
{
	ZED::HttpClient client(m_Hostname, m_Port);
	char Token[] = "token=test";
	std::string response = client.POST(URL, Token);

	assert(response == "ok");
	return response == "ok";
}