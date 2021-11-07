#include "remote_tournament.h"
#include <ZED/include/log.h>
#include <ZED/include/http_client.h>



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
	auto response = Request2Master("/ajax/master/find_participant");

	ZED::CSV csv(response);
	Judoka* judoka = new Judoka(csv);
	m_StandingData.AddJudoka(judoka);

	return judoka;
}



const Judoka* RemoteTournament::FindParticipant(const UUID& UUID) const
{
	auto response = Request2Master("/ajax/master/find_participant");

	ZED::CSV csv(response);
	Judoka* judoka = new Judoka(csv);
	m_StandingData.AddJudoka(judoka);

	return judoka;
}



std::string RemoteTournament::Request2Master(const std::string& URL) const
{
	ZED::HttpClient client(m_Hostname, m_Port);
	auto response = (std::string)client.GET("/ajax/master/find_participant");

	if (response.length() == 0)
		ZED::Log::Info("Could not connect to master server: " + m_Hostname + ":" + std::to_string(m_Port));

	return response;
}