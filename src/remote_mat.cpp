#include "../ZED/include/http_client.h"
#include "../ZED/include/log.h"
#include "remote_mat.h"
#include "matchlog.h"
#include "app.h"



using namespace Judoboard;



RemoteMat::RemoteMat(uint32_t ID, const std::string& Host, uint16_t Port, const std::string& Token)
	: IMat(ID), m_Hostname(Host), m_Port(Port), m_SecurityToken(Token)
{
	if (!SendCommand("/ajax/slave/set_mat_id?id=" + std::to_string(GetMatID())))
		ZED::Log::Error("Could not connect to remote mat");
}



bool RemoteMat::IsOpen() const
{
	if (!SendCommand("/ajax/slave/is_mat_open?id=" + std::to_string(GetMatID())))
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



std::vector<IMat::OsaekomiEntry> RemoteMat::GetOsaekomiList() const
{
	std::string response = SendRequest("/ajax/mat/get_osaekomilist?id=" + std::to_string(GetMatID()));

	auto count = std::count(response.begin(), response.end(), ',');

	m_OsaekomiList.clear();

	if (count > 0)
	{
		ZED::CSV data(response);

		for (int i = 0; i < count; i+=2)
		{
			Fighter who;
			uint32_t time;
			data >> who >> time;

			m_OsaekomiList.emplace_back(OsaekomiEntry(who, time));
		}
	}

	return m_OsaekomiList;
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



bool RemoteMat::StartMatch(Match* NewMatch, bool UseForce)
{
	if (!NewMatch)
	{
		ZED::Log::Warn("Invalid match");
		return false;
	}

	YAML::Emitter yaml;
	*NewMatch >> yaml;

	const bool ret = PostData("/ajax/slave/start_match?id=" + std::to_string(GetMatID()), yaml);

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
	return client.GET(URL + "&token=" + m_SecurityToken);
	//return client.GET(URL, "Cookie: token=test");
}



bool RemoteMat::PostData(const std::string& URL, const YAML::Emitter& Data) const
{
	ZED::Log::Debug("Posting data: " + URL);

	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.POST(URL, Data.c_str(), "Cookie: token=test");
	//assert(response == "ok");
	return response == "ok";
}



RemoteMat::InternalState RemoteMat::GetState(bool* pSuccess) const
{
	std::string response = SendRequest("/ajax/mat/get_score?id=" + std::to_string(GetMatID()));
	InternalState internalState = {};

	auto yaml = YAML::Load(response);

	if (!yaml)
	{
		if (pSuccess)
			*pSuccess = false;
		return internalState;
	}

	if (pSuccess)
		*pSuccess = true;

	//White
	auto white = yaml["white"];
	SetScoreboard(Fighter::White).m_Ippon   = white["ippon"].as<uint32_t>();
	SetScoreboard(Fighter::White).m_WazaAri = white["wazaari"].as<uint32_t>();
	if (white["yuko"])
		SetScoreboard(Fighter::White).m_Yuko = white["yuko"].as<int32_t>();
	else
		SetScoreboard(Fighter::White).m_Yuko = -1;
	if (white["koka"])
		SetScoreboard(Fighter::White).m_Koka = white["koka"].as<int32_t>();
	else
		SetScoreboard(Fighter::White).m_Koka = -1;
	SetScoreboard(Fighter::White).m_Shido = white["shido"].as<uint32_t>();
	SetScoreboard(Fighter::White).m_MedicalExamination = white["medical"].as<uint32_t>();

	//Blue
	auto blue = yaml["blue"];
	SetScoreboard(Fighter::Blue).m_Ippon   = blue["ippon"].as<uint32_t>();
	SetScoreboard(Fighter::Blue).m_WazaAri = blue["wazaari"].as<uint32_t>();
	if (blue["yuko"])
		SetScoreboard(Fighter::Blue).m_Yuko = blue["yuko"].as<int32_t>();
	else
		SetScoreboard(Fighter::Blue).m_Yuko = -1;
	if (blue["koka"])
		SetScoreboard(Fighter::Blue).m_Koka = blue["koka"].as<int32_t>();
	else
		SetScoreboard(Fighter::Blue).m_Koka = -1;
	SetScoreboard(Fighter::Blue).m_Shido = blue["shido"].as<uint32_t>();
	SetScoreboard(Fighter::Blue).m_MedicalExamination = blue["medical"].as<uint32_t>();

	//Conversion
	for (Fighter f = Fighter::White; f <= Fighter::Blue; ++f)
	{
		//if (SetScoreboard(f).m_Ippon == 1)
			//SetScoreboard(f).m_WazaAri = 2;
		SetScoreboard(f).m_HansokuMake = m_Scoreboards[(int)f].m_Shido >= 3;
	}

	//Timers
	internalState.display_time        =  yaml["hajime_time"].as<uint32_t>();
	internalState.white_osaekomi_time = white["osaekomi_time"].as<uint32_t>();
	internalState.blue_osaekomi_time  =  blue["osaekomi_time"].as<uint32_t>();

	//Extra states
	internalState.hajime            = yaml["is_hajime"].as<bool>();
	internalState.IsOsaekomi        = yaml["is_osaekomi"].as<bool>();
	internalState.white_osaekomi    = white["is_osaekomi_running"].as<bool>();
	internalState.blue_osaekomi     = blue["is_osaekomi_running"].as<bool>();
	internalState.cannextmatchstart = yaml["can_next_match_start"].as<bool>();
	internalState.hasconcluded      = yaml["has_concluded"].as<bool>();
	internalState.isoutoftime       = yaml["is_out_of_time"].as<bool>();
	internalState.NoWinnerYet       = yaml["no_winner_yet"].as<bool>();
	internalState.isgoldenscore     = yaml["is_goldenscore"].as<bool>();
	internalState.arefightersonmat  = yaml["are_fighters_on_mat"].as<bool>();

	return internalState;
}