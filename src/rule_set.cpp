#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "rule_set.h"



using namespace Judoboard;



RuleSet::RuleSet(const std::string& Name, uint32_t MatchTime, int GoldenScoreTime, uint32_t OsaeKomiTime, uint32_t OsaeKomiTime_with_Wazaari, bool Yuko, bool Koka, bool Draw, uint32_t BreakTime, bool ExtendBreakTime)
	: m_Name(Name)
{
	m_Yuko = Yuko;
	m_Koka = Koka;
	m_Draw = Draw;

	m_MatchTime = MatchTime;
	m_GoldenScoreTime = GoldenScoreTime;
	if (GoldenScoreTime < 0)
		m_GoldenScoreTime = -1;

	m_OsaeKomiTime = OsaeKomiTime;
	m_OsaeKomiTime_With_WazaAri = OsaeKomiTime_with_Wazaari;

	m_BreakTime = BreakTime;
	m_IsExtendBreakTime = ExtendBreakTime;
}



RuleSet::RuleSet(const YAML::Node& Yaml)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();

	if (Yaml["match_time"])
		m_MatchTime = Yaml["match_time"].as<int>();
	if (Yaml["golden_score_time"])
		m_GoldenScoreTime = Yaml["golden_score_time"].as<int>();
	if (Yaml["osaekomi_time"])
		m_OsaeKomiTime = Yaml["osaekomi_time"].as<int>();
	if (Yaml["osaekomi_with_wazaari_time"])
		m_OsaeKomiTime_With_WazaAri = Yaml["osaekomi_with_wazaari_time"].as<int>();
	if (Yaml["break_time"])
		m_BreakTime = Yaml["break_time"].as<int>();
	if (Yaml["extend_break_time"])
		m_IsExtendBreakTime = Yaml["extend_break_time"].as<bool>();

	if (Yaml["yuko"])
		m_Yuko = Yaml["yuko"].as<bool>();
	if (Yaml["koka"])
		m_Koka = Yaml["koka"].as<bool>();
	if (Yaml["draw"])
		m_Draw = Yaml["draw"].as<bool>();
}



void RuleSet::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;
	Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "name" << YAML::Value << m_Name;

	Yaml << YAML::Key << "match_time"        << YAML::Value << m_MatchTime;
	Yaml << YAML::Key << "golden_score_time" << YAML::Value << m_GoldenScoreTime;

	Yaml << YAML::Key << "osaekomi_time" << YAML::Value << m_OsaeKomiTime;
	Yaml << YAML::Key << "osaekomi_with_wazaari_time" << YAML::Value << m_OsaeKomiTime_With_WazaAri;
	Yaml << YAML::Key << "break_time" << YAML::Value << m_BreakTime;
	Yaml << YAML::Key << "extend_break_time" << YAML::Value << m_IsExtendBreakTime;

	Yaml << YAML::Key << "yuko" << YAML::Value << m_Yuko;
	Yaml << YAML::Key << "koka" << YAML::Value << m_Koka;
	Yaml << YAML::Key << "draw" << YAML::Value << m_Draw;
	Yaml << YAML::EndMap;
}



bool RuleSet::IsOutOfTime(int MatchTime_Millseconds, bool GoldenScore) const
{
	if (!GoldenScore)
	{
		if (m_MatchTime < 0)//Infinite match time?
			return false;
		return MatchTime_Millseconds >= m_MatchTime * 1000;
	}

	if (m_GoldenScoreTime < 0)//Infinite golden score time?
		return false;
	return MatchTime_Millseconds >= m_GoldenScoreTime * 1000;
}



const std::string RuleSet::GetDescription() const
{
	std::string ret;
	if (m_MatchTime < 0)
		ret += "&infin; ";
	else
		ret += Timer::TimestampToString(m_MatchTime * 1000) + " ";

	if (IsGoldenScoreEnabled())
	{
		if (m_GoldenScoreTime < 0)
			ret += "(+&infin;) ";
		else
			ret += "(+" + Timer::TimestampToString(m_GoldenScoreTime * 1000) + ") ";
	}

	return ret + std::to_string(m_OsaeKomiTime) + "|" + std::to_string(m_OsaeKomiTime_With_WazaAri);
}