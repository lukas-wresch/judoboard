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
	auto state = GetState(&success);

	if (!success)
		return true;

	return state.arefightersonmat;
}



bool RemoteMat::CanNextMatchStart() const
{
	bool success;
	auto state = GetState(&success);

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

	const bool ret = PostData("/ajax/slave/start_match?id=" + std::to_string(GetMatID()), csv);

	if (ret)
		m_pMatch = NewMatch;

	return ret;
}



bool RemoteMat::HasConcluded() const
{
	bool success;
	auto state = GetState(&success);

	if (!success)
		return false;

	return state.hasconcluded;
}



bool RemoteMat::EndMatch()
{
	//Used to keep the judoka data in sync
	if (m_pMatch->GetFighter(Fighter::White))
		m_pMatch->GetFighter(Fighter::White)->StartBreak();
	if (m_pMatch->GetFighter(Fighter::Blue))
		m_pMatch->GetFighter(Fighter::Blue)->StartBreak();

	const bool ret = SendCommand("/ajax/mat/end_match?id=" + std::to_string(GetMatID()));

	if (ret)
		m_pMatch = nullptr;

	return ret;
}



bool RemoteMat::EnableGoldenScore(bool GoldenScore)
{
	if (GoldenScore)
		return SendCommand("/ajax/mat/+golden_score?id=" + std::to_string(GetMatID()));
	else
		return SendCommand("/ajax/mat/-golden_score?id=" + std::to_string(GetMatID()));
}



void RemoteMat::Hajime()
{
	SendCommand("/ajax/mat/hajime?id=" + std::to_string(GetMatID()));
}



void RemoteMat::Mate()
{
	SendCommand("/ajax/mat/mate?id=" + std::to_string(GetMatID()));
}



void RemoteMat::Sonomama()
{
	SendCommand("/ajax/mat/sonomama?id=" + std::to_string(GetMatID()));
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



void RemoteMat::AddYuko(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+yuko?id=" + std::to_string(GetMatID()));
}



void RemoteMat::RemoveYuko(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-yuko?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddKoka(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+koka?id=" + std::to_string(GetMatID()));
}



void RemoteMat::RemoveKoka(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-koka?id=" + std::to_string(GetMatID()));
}



void RemoteMat::Hantei(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/hantei?id=" + std::to_string(GetMatID()));
}



void RemoteMat::SetAsDraw(bool Enable)
{
	if (Enable)
		SendCommand("/ajax/mat/+draw?id=" + std::to_string(GetMatID()));
	else
		SendCommand("/ajax/mat/-draw?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddShido(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+shido?id=" + std::to_string(GetMatID()));
}



void RemoteMat::RemoveShido(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-shido?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddHansokuMake(Fighter Whom, bool Direct)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+hansokumake?id=" + std::to_string(GetMatID()));
	//TODO send disqualification command in case of a direct hansokumake
}



void RemoteMat::RemoveHansokuMake(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-hansokumake?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddDisqualification(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+disqualification?id=" + std::to_string(GetMatID()));
}



void RemoteMat::RemoveDisqualification(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-disqualification?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddNoDisqualification(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+nodisqualification?id=" + std::to_string(GetMatID()));
}



void RemoteMat::RemoveNoDisqualification(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-nodisqualification?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddMedicalExamination(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+medic?id=" + std::to_string(GetMatID()));
}



void RemoteMat::RemoveMedicalExamination(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-medic?id=" + std::to_string(GetMatID()));
}



void RemoteMat::AddGachi(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/+gachi?id=" + std::to_string(GetMatID()));
}



void RemoteMat::RemoveGachi(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/-gachi?id=" + std::to_string(GetMatID()));
}



void RemoteMat::Osaekomi(Fighter Whom)
{
	SendCommand("/ajax/mat/" + Fighter2String(Whom) + "/osaekomi?id=" + std::to_string(GetMatID()));
}



void RemoteMat::Tokeda()
{
	SendCommand("/ajax/mat/tokeda?id=" + std::to_string(GetMatID()));
}



ZED::Blob RemoteMat::RequestScreenshot() const
{
	ZED::Blob data(SendRequest("/ajax/mat/screenshot?id=" + std::to_string(GetMatID())).body);
	return data;
}



bool RemoteMat::SendCommand(const std::string& URL) const
{
	std::string response = SendRequest(URL);
	return response == "ok";
}



ZED::HttpClient::Packet RemoteMat::SendRequest(const std::string& URL) const
{
	//ZED::Log::Debug("Sending request: " + URL);

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



RemoteMat::InternalState RemoteMat::GetState(bool* pSuccess) const
{
	std::string response = SendRequest("/ajax/mat/get_score?id=" + std::to_string(GetMatID()));
	InternalState internalState = {};

	if (std::count(response.begin(), response.end(), ',') < 10)
	{
		ZED::Log::Warn("Invalid response: " + response);
		if (pSuccess)
			*pSuccess = false;
		return internalState;
	}

	if (pSuccess)
		*pSuccess = true;

	ZED::CSV state(response);

	state >> SetScoreboard(Fighter::White).m_Ippon >> SetScoreboard(Fighter::White).m_WazaAri;
	state >> SetScoreboard(Fighter::Blue).m_Ippon  >> SetScoreboard(Fighter::Blue).m_WazaAri;

	state >> SetScoreboard(Fighter::White).m_Yuko >> SetScoreboard(Fighter::Blue).m_Yuko;
	state >> SetScoreboard(Fighter::White).m_Koka >> SetScoreboard(Fighter::Blue).m_Koka;

	state >> SetScoreboard(Fighter::White).m_Shido >> SetScoreboard(Fighter::Blue).m_Shido;
	state >> SetScoreboard(Fighter::White).m_MedicalExamination >> SetScoreboard(Fighter::Blue).m_MedicalExamination;

	for (Fighter f = Fighter::White; f <= Fighter::Blue; ++f)
		SetScoreboard(f).m_HansokuMake = m_Scoreboards[(int)f].m_Shido >= 3;

	state >> internalState.display_time >> internalState.hajime;

	state >> internalState.white_osaekomi_time >> internalState.white_osaekomi;
	state >> internalState.blue_osaekomi_time  >> internalState.blue_osaekomi;

	state >> internalState.cannextmatchstart >> internalState.hasconcluded >> internalState.isoutoftime >> internalState.NoWinnerYet  >> internalState.isgoldenscore >> internalState.arefightersonmat;

	return internalState;
}