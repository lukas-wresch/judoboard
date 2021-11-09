#include "../ZED/include/http_client.h"
#include "../ZED/include/log.h"
#include <cassert>
#include "remote_mat.h"
#include "app.h"



using namespace Judoboard;



RemoteMat::RemoteMat(uint32_t ID, const std::string& Host, uint16_t Port) : IMat(ID), m_Hostname(Host), m_Port(Port)
{
	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/set_mat_id?id=" + std::to_string(GetMatID()));
	assert(response == "ok");
}



bool RemoteMat::IsOpen() const
{
	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/is_mat_open?id=" + std::to_string(GetMatID()));
	return response == "ok";
}



bool RemoteMat::Open()
{
	return SendCommand("/ajax/slave/open_mat?id=" + std::to_string(GetMatID()));
}



bool RemoteMat::Close()
{
	return SendCommand("/ajax/slave/close_mat?id=" + std::to_string(GetMatID()));
}



bool RemoteMat::IsConnected() const
{
	return SendCommand("/ajax/slave/get_mat_status?id=" + std::to_string(GetMatID()));
}



bool RemoteMat::AreFightersOnMat() const
{
	bool success;
	auto state = GetState(success);

	if (!success)
		return true;

	return state.arefightersonmat;
}



bool RemoteMat::CanNextMatchStart() const
{
	bool success;
	auto state = GetState(success);

	if (!success)
		return false;

	return state.cannextmatchstart;
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

	return PostData("/ajax/slave/start_match?id=" + std::to_string(GetMatID()), csv);
}



bool RemoteMat::HasConcluded() const
{
	bool success;
	auto state = GetState(success);

	if (!success)
		return false;

	return state.hasconcluded;
}



bool RemoteMat::EndMatch()
{
	return SendCommand("/ajax/slave/end_match?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddIppon(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+ippon?id=" + std::to_string(GetMatID()));
}



void RemoteMat::RemoveIppon(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-ippon?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddWazaAri(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+wazaari?id=" + std::to_string(GetMatID()));
}



void RemoteMat::RemoveWazaAri(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-wazaari?id=" + std::to_string(GetMatID()));
}



bool RemoteMat::SendCommand(const std::string& URL) const
{
	std::string response = SendRequest(URL);

	assert(response == "ok");
	return response == "ok";
}



std::string RemoteMat::SendRequest(const std::string& URL) const
{
	ZED::Log::Debug("Sending request: " + URL);

	ZED::HttpClient client(m_Hostname, m_Port);
	return client.GET(URL, "Cookie: token=test");
}



bool RemoteMat::PostData(const std::string& URL, const ZED::CSV& Data) const
{
	ZED::Log::Debug("Posting data: " + URL);

	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.POST(URL, Data, "Cookie: token=test");
	return response == "ok";
}



RemoteMat::InternalState RemoteMat::GetState(bool& Success) const
{
	auto response = SendRequest("/ajax/mat/get_score?id=" + std::to_string(GetMatID()));
	InternalState internalState = {};

	if (std::count(response.begin(), response.end(), ',') < 10)
	{
		ZED::Log::Warn("Invalid response: " + response);
		Success = false;
		assert(false);
		return internalState;
	}

	Success = true;

	ZED::CSV state(response);

	state >> internalState.white_ippon >> internalState.white_wazaari;
	state >> internalState.blue_ippon  >> internalState.blue_wazaari;

	state >> internalState.white_shidos  >> internalState.blue_shidos;
	state >> internalState.white_medical >> internalState.blue_medical;

	state >> internalState.display_time >> internalState.hajime;

	state >> internalState.white_osaekomi_time >> internalState.white_osaekomi;
	state >> internalState.blue_osaekomi_time  >> internalState.blue_osaekomi;

	state >> internalState.cannextmatchstart >> internalState.hasconcluded >> internalState.isoutoftime_and_draw >> internalState.isgoldenscore >> internalState.arefightersonmat;

	return internalState;
}