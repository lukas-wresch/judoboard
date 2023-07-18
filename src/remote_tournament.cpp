#include "remote_tournament.h"
#include "../ZED/include/log.h"
#include "../ZED/include/http_client.h"



using namespace Judoboard;



RemoteTournament::RemoteTournament(const std::string& Host, uint16_t Port) : m_Hostname(Host), m_Port(Port)
{
}



std::vector<Match> RemoteTournament::GetNextMatches(uint32_t MatID) const
{
	auto response = Request2Master("/ajax/master/get_next_matches?id=" + std::to_string(MatID));
	std::vector<Match> ret;

	if (response.length() == 0)
	{
		ZED::Log::Warn("Could not obtain next match data from master server");
		return ret;
	}

	ZED::CSV csv(response);
	int count;
	csv >> count;
	for (int i = 0; i < count; i++)
	{
		ret.emplace_back(Match(csv, (RemoteTournament*)this));
	}
	
	return ret;
}



bool RemoteTournament::AddMatch(Match* NewMatch)
{
	if (!NewMatch)
	{
		ZED::Log::Error("Invalid match");
		return false;
	}

	//TODO: Add match data to cache

	return true;
}



bool RemoteTournament::IsParticipant(const Judoka& Judoka) const
{
	ZED::Log::Error("RemoteTournament::IsParticipant() NOT IMPLEMENTED");
	return false;
}



Judoka* RemoteTournament::FindParticipant(const UUID& UUID)
{
	auto response = Request2Master("/ajax/master/find_judoka?uuid=" + (std::string)UUID);

	if (response.length() == 0)
	{
		ZED::Log::Error("Could not obtain judoka data from master server");
		return nullptr;
	}

	YAML::Node yaml = YAML::Load(response);

	if (!yaml)
		return nullptr;

	Judoka* judoka = new Judoka(yaml, &m_StandingData);//TODO replace by pointer to real standing data
	m_StandingData.AddJudoka(judoka);

	return judoka;
}



const Judoka* RemoteTournament::FindParticipant(const UUID& UUID) const
{
	auto response = Request2Master("/ajax/master/find_judoka?uuid=" + (std::string)UUID);

	if (response.length() == 0)
	{
		ZED::Log::Error("Could not obtain judoka data from master server");
		return nullptr;
	}

	YAML::Node yaml = YAML::Load(response);

	if (!yaml)
		return nullptr;

	Judoka* judoka = new Judoka(yaml, &m_StandingData);//TODO replace by pointer to real standing data
	m_StandingData.AddJudoka(judoka);

	return judoka;
}



const RuleSet* RemoteTournament::FindRuleSet(const UUID& UUID) const
{
	auto response = Request2Master("/ajax/master/find_ruleset?uuid=" + (std::string)UUID);

	if (response.length() == 0)
	{
		ZED::Log::Error("Could not obtain rule set data from master server");
		return nullptr;
	}

	ZED::CSV csv(response);
	RuleSet* rule_set = new RuleSet(csv);
	m_StandingData.AddRuleSet(rule_set);//Add to cache

	return rule_set;
}



RuleSet* RemoteTournament::FindRuleSet(const UUID& UUID)
{
	auto response = Request2Master("/ajax/master/find_ruleset?uuid=" + (std::string)UUID);

	if (response.length() == 0)
	{
		ZED::Log::Error("Could not obtain rule set data from master server");
		return nullptr;
	}

	ZED::CSV csv(response);
	RuleSet* rule_set = new RuleSet(csv);
	m_StandingData.AddRuleSet(rule_set);//Add to cache

	return rule_set;
}



void RemoteTournament::OnMatchConcluded(const Match& Match) const
{
	YAML::Emitter result;
	Match >> result;

	Post2Master("/ajax/master/post_match_result", result);
}



std::string RemoteTournament::Request2Master(const std::string& URL) const
{
	//ZED::Log::Debug("Requesting data from master: " + URL);

	ZED::HttpClient client(m_Hostname, m_Port);
	auto response = (std::string)client.GET(URL);

	if (response.length() == 0)
	{
		ZED::Log::Info("Could not connect to master server: " + m_Hostname + ":" + std::to_string(m_Port));
		return "";
	}
	else if (std::count(response.begin(), response.end(), ':') < 2)
	{
		ZED::Log::Warn("Invalid response: " + response);
		return "";
	}

	return response;
}



bool RemoteTournament::Post2Master(const std::string& URL, const YAML::Emitter& Data) const
{
	ZED::Log::Debug("Posting data to master: " + URL);

	ZED::HttpClient client(m_Hostname, m_Port);
	std::string response = client.POST(URL, Data.c_str(), "token=test");

	if (response.length() == 0 || response != "ok")
	{
		ZED::Log::Warn("Invalid response: " + response);
		return false;
	}

	return true;
}