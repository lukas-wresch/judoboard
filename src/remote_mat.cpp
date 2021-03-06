#include "../ZED/include/http_client.h"
#include "../ZED/include/log.h"
#include "remote_mat.h"
#include "app.h"



using namespace Judoboard;



RemoteMat::RemoteMat(uint32_t ID, const std::string& Host, uint16_t Port) : IMat(ID), m_Hostname(Host), m_Port(Port)
{
	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/set_mat_id?id=" + std::to_string(GetMatID()));
	if (response != "ok")
		ZED::Log::Error("Could not connect to remote mat");
}



bool RemoteMat::IsOpen() const
{
	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.GET("/ajax/slave/is_mat_open?id=" + std::to_string(GetMatID()));

	if (response != "ok")
	{
		ZED::Log::Error("Could not open remote mat");
		return false;
	}

	return true;
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

	YAML::Emitter yaml;
	*NewMatch >> yaml;

	return PostData("/ajax/slave/start_match?id=" + std::to_string(GetMatID()), yaml);
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
	return SendCommand("/ajax/mat/end_match?id=" + std::to_string(GetMatID()));
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



ZED::Blob RemoteMat::RequestScreenshot() const
{
	ZED::Blob data(SendRequest("/ajax/mat/screenshot?id=" + std::to_string(GetMatID())).body);
	//return ZED::PNG(std::move(data));
	return data;
}



bool RemoteMat::SendCommand(const std::string& URL) const
{
	std::string response = SendRequest(URL);
	return response == "ok";
}



ZED::HttpClient::Packet RemoteMat::SendRequest(const std::string& URL) const
{
	ZED::Log::Debug("Sending request: " + URL);

	ZED::HttpClient client(m_Hostname, m_Port);
	return client.GET(URL, "Cookie: token=test");
}



bool RemoteMat::PostData(const std::string& URL, const YAML::Emitter& Data) const
{
	ZED::Log::Debug("Posting data: " + URL);

	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.POST(URL, Data.c_str(), "Cookie: token=test");
	return response == "ok";
}



RemoteMat::InternalState RemoteMat::GetState(bool& Success) const
{
	std::string response = SendRequest("/ajax/mat/get_score?id=" + std::to_string(GetMatID()));
	InternalState internalState = {};

	if (std::count(response.begin(), response.end(), ',') < 10)
	{
		ZED::Log::Warn("Invalid response: " + response);
		Success = false;
		return internalState;
	}

	Success = true;

	ZED::CSV state(response);

	state >> SetScoreboard(Fighter::White).m_Ippon >> SetScoreboard(Fighter::White).m_WazaAri;
	state >> SetScoreboard(Fighter::Blue).m_Ippon  >> SetScoreboard(Fighter::Blue).m_WazaAri;

	state >> SetScoreboard(Fighter::White).m_Shido >> SetScoreboard(Fighter::Blue).m_Shido;
	state >> SetScoreboard(Fighter::White).m_MedicalExamination >> SetScoreboard(Fighter::Blue).m_MedicalExamination;

	state >> internalState.display_time >> internalState.hajime;

	state >> internalState.white_osaekomi_time >> internalState.white_osaekomi;
	state >> internalState.blue_osaekomi_time  >> internalState.blue_osaekomi;

	state >> internalState.cannextmatchstart >> internalState.hasconcluded >> internalState.isoutoftime >> internalState.NoWinnerYet  >> internalState.isgoldenscore >> internalState.arefightersonmat;

	return internalState;
}