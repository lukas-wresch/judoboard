#include <ZED/include/http_client.h>
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
	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/open_mat?id=" + std::to_string(GetMatID()));
	return response == "ok";
}



bool RemoteMat::Close()
{
	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/close_mat?id=" + std::to_string(GetMatID()));
	return response == "ok";
}



bool RemoteMat::IsConnected() const
{
	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/get_mat_status?id=" + std::to_string(GetMatID()));
	return response == "ok";
}



bool RemoteMat::StartMatch(Match& NewMatch)
{
	ZED::CSV csv;
	NewMatch >> csv;

	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/start_match?id=" + std::to_string(GetMatID()) + "&match=" + (std::string)csv);
	return response == "ok";
}