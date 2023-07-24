#include "remote_tournament.h"
#include "../ZED/include/log.h"
#include "../ZED/include/http_client.h"



using namespace Judoboard;



RemoteTournament::RemoteTournament(const std::string& Host, uint16_t Port) : m_Hostname(Host), m_Port(Port)
{
}



std::vector<Match> RemoteTournament::GetNextMatches(int32_t MatID) const
{
	if (Timer::GetTimestamp() - m_NextMatches_Timestamp < 5000)
		return m_NextMatches;//Return cached value

	auto response = Request2Master("/ajax/master/get_next_matches?id=" + std::to_string(MatID));
	std::vector<Match> ret;

	if (response.length() == 0)
	{
		ZED::Log::Warn("Could not obtain next match data from master server");
		return ret;
	}

	YAML::Node yaml = YAML::Load(response);

	if (!yaml)
		return ret;

	for (const auto& node : yaml)
	{
		if (!node.IsMap() || !node["uuid"])
			continue;

		UUID id(node["uuid"].as<std::string>());

		if (!IsMatchInCache(id))
			m_MatchCache.push_back(new Match(node, nullptr, (RemoteTournament*)this));

		Match match(node, nullptr, (RemoteTournament*)this);
		ret.emplace_back(match);
	}

	auto guard = m_Mutex.LockWriteForScope();
	m_NextMatches = std::move(ret);
	m_NextMatches_Timestamp = Timer::GetTimestamp();

	return m_NextMatches;
}



bool RemoteTournament::IsMatchInCache(const UUID& UUID) const
{
	auto guard = m_Mutex.LockReadForScope();

	for (auto match : m_MatchCache)
		if (match && match->GetUUID() == UUID)
			return true;
	return false;
}



Match* RemoteTournament::FindInCache(const UUID& UUID) const
{
	auto guard = m_Mutex.LockReadForScope();

	for (auto match : m_MatchCache)
		if (match && match->GetUUID() == UUID)
			return match;
	return nullptr;
}



MatchTable* RemoteTournament::FindMatchTableInCache(const UUID& UUID) const
{
	auto guard = m_Mutex.LockReadForScope();

	for (auto table : m_MatchTables)
		if (table && table->GetUUID() == UUID)
			return table;
	return nullptr;
}



bool RemoteTournament::AddMatch(Match* NewMatch)
{
	if (!NewMatch)
	{
		ZED::Log::Error("Invalid match");
		return false;
	}

	auto guard = m_Mutex.LockWriteForScope();

	//Add match data to cache	
	if (!IsMatchInCache(*NewMatch))
		m_MatchCache.push_back(NewMatch);

	return true;
}



bool RemoteTournament::IsParticipant(const Judoka& Judoka) const
{
	ZED::Log::Error("RemoteTournament::IsParticipant() NOT IMPLEMENTED");
	return false;
}



Judoka* RemoteTournament::FindParticipant(const UUID& UUID)
{
	auto judoka = m_StandingData.FindJudoka(UUID);
	if (judoka)
		return judoka;

	auto response = Request2Master("/ajax/master/find_judoka?uuid=" + (std::string)UUID);

	if (response.length() == 0)
	{
		ZED::Log::Error("Could not obtain judoka data from master server");
		return nullptr;
	}

	YAML::Node yaml = YAML::Load(response);

	if (!yaml)
		return nullptr;

	auto guard = m_Mutex.LockWriteForScope();

	judoka = new Judoka(yaml, &m_StandingData);//TODO replace by pointer to real standing data
	m_StandingData.AddJudoka(judoka);

	return judoka;
}



const Judoka* RemoteTournament::FindParticipant(const UUID& UUID) const
{
	auto judoka = m_StandingData.FindJudoka(UUID);
	if (judoka)
		return judoka;

	auto response = Request2Master("/ajax/master/find_judoka?uuid=" + (std::string)UUID);

	if (response.length() == 0)
	{
		ZED::Log::Error("Could not obtain judoka data from master server");
		return nullptr;
	}

	YAML::Node yaml = YAML::Load(response);

	if (!yaml)
		return nullptr;

	auto guard = m_Mutex.LockWriteForScope();

	judoka = new Judoka(yaml, &m_StandingData);//TODO replace by pointer to real standing data
	m_StandingData.AddJudoka(judoka);

	return judoka;
}



MatchTable* RemoteTournament::FindMatchTable(const UUID& ID)
{
	auto response = Request2Master("/ajax/master/find_match_table?uuid=" + (std::string)ID);

	if (response.length() == 0)
	{
		ZED::Log::Error("Could not obtain match table data from master server");
		return nullptr;
	}

	YAML::Node yaml = YAML::Load(response);

	if (!yaml)
		return nullptr;

	auto guard = m_Mutex.LockWriteForScope();

	MatchTable* table = MatchTable::CreateMatchTable(yaml, this);
	m_MatchTables.push_back(table);

	return table;
}



const MatchTable* RemoteTournament::FindMatchTable(const UUID& ID) const
{
	auto response = Request2Master("/ajax/master/find_match_table?uuid=" + (std::string)ID);

	if (response.length() == 0)
	{
		ZED::Log::Error("Could not obtain match table data from master server");
		return nullptr;
	}

	YAML::Node yaml = YAML::Load(response);

	if (!yaml)
		return nullptr;

	auto guard = m_Mutex.LockWriteForScope();

	MatchTable* table = MatchTable::CreateMatchTable(yaml, this);
	m_MatchTables.push_back(table);

	return table;
}



const RuleSet* RemoteTournament::FindRuleSet(const UUID& UUID) const
{
	auto response = Request2Master("/ajax/master/find_ruleset?uuid=" + (std::string)UUID);

	if (response.length() == 0)
	{
		ZED::Log::Error("Could not obtain rule set data from master server");
		return nullptr;
	}

	YAML::Node yaml = YAML::Load(response);

	if (!yaml)
		return nullptr;

	RuleSet* rule_set = new RuleSet(yaml);
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

	YAML::Node yaml = YAML::Load(response);

	if (!yaml)
		return nullptr;

	RuleSet* rule_set = new RuleSet(yaml);
	m_StandingData.AddRuleSet(rule_set);//Add to cache

	return rule_set;
}



void RemoteTournament::OnMatchConcluded(const Match& Match) const
{
	YAML::Emitter result;
	Match >> result;

	if (!Post2Master("/ajax/master/post_match_result", result))
		ZED::Log::Error("Could not post match result to master");
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
	else if (response.length() < 2)
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