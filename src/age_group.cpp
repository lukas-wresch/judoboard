#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../ZED/include/core.h"
#include "age_group.h"
#include "standing_data.h"



using namespace Judoboard;



AgeGroup::AgeGroup(const std::string& Name, uint32_t MinAge, uint32_t MaxAge, const RuleSet* Rules) : m_Name(Name), m_pRules(Rules)
{
	m_MinAge = MinAge;
	m_MaxAge = MaxAge;
}



AgeGroup::AgeGroup(const YAML::Node& Yaml, const StandingData* pStandingData)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();
	if (Yaml["min_age"])
		m_MinAge = Yaml["min_age"].as<uint32_t>();
	if (Yaml["max_age"])
		m_MinAge = Yaml["max_age"].as<uint32_t>();

	if (Yaml["rules"] && pStandingData)
		m_pRules = pStandingData->FindRuleSet(Yaml["rules"].as<std::string>());
}



void AgeGroup::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;
	Yaml << YAML::Key << "uuid"    << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "name"    << YAML::Value << m_Name;
	Yaml << YAML::Key << "min_age" << YAML::Value << m_MinAge;
	Yaml << YAML::Key << "max_age" << YAML::Value << m_MaxAge;

	if (m_pRules)
		Yaml << YAML::Key << "rules" << YAML::Value << (std::string)m_pRules->GetUUID();

	Yaml << YAML::EndMap;
}