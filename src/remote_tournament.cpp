#include "remote_tournament.h"
#include "../ZED/include/log.h"
#include "../ZED/include/http_client.h"



using namespace Judoboard;



RemoteTournament::RemoteTournament(const std::string& Host, uint16_t Port) : m_Hostname(Host), m_Port(Port)
{
}



std::vector<const Match*> RemoteTournament::GetNextMatches(uint32_t MatID) const
{
	ZED::Log::Error("NOT IMPLEMENTED");
	std::vector<const Match*> ret;
	return ret;
}



bool RemoteTournament::IsParticipant(const Judoka& Judoka) const
{
	ZED::Log::Error("NOT IMPLEMENTED");
	return false;
}



Judoka* RemoteTournament::FindParticipant(const UUID& UUID)
{
	auto response = Request2Master("/ajax/master/find_participant?uuid=" + (std::string)UUID);

	if (response.length() == 0)
		return nullptr;

	ZED::CSV csv(response);
	Judoka* judoka = new Judoka(csv);
	m_StandingData.AddJudoka(judoka);

	return judoka;
}



const Judoka* RemoteTournament::FindParticipant(const UUID& UUID) const
{
	auto response = Request2Master("/ajax/master/find_participant?uuid=" + (std::string)UUID);

	if (response.length() == 0)
		return nullptr;

	ZED::CSV csv(response);
	Judoka* judoka = new Judoka(csv);
	m_StandingData.AddJudoka(judoka);

	return judoka;
}



void RemoteTournament::OnMatchConcluded(const Match& Match) const
{
	ZED::CSV result;
	Match >> result;

	Post2Master("/ajax/master/post_match_result", result);
}



std::string RemoteTournament::Request2Master(const std::string& URL) const
{
	ZED::Log::Debug("Requesting data from master: " + URL);

	ZED::HttpClient client(m_Hostname, m_Port);
	auto response = (std::string)client.GET(URL);

	if (response.length() == 0)
	{
		ZED::Log::Info("Could not connect to master server: " + m_Hostname + ":" + std::to_string(m_Port));
		return "";
	}
	else if (std::count(response.begin(), response.end(), ',') < 3)
	{
		ZED::Log::Warn("Invalid response: " + response);
		return "";
	}

	return response;
}



bool RemoteTournament::Post2Master(const std::string& URL, const ZED::CSV& Data) const
{
	ZED::Log::Debug("Posting data to master: " + URL);

	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.POST(URL, Data, "token=test");

	if (response.length() == 0 || response != "ok")
	{
		ZED::Log::Warn("Invalid response: " + response);
		return false;
	}

	return true;
}