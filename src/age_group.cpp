#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../ZED/include/core.h"
#include "age_group.h"
#include "standing_data.h"
#include "localizer.h"



using namespace Judoboard;



AgeGroup::AgeGroup(const std::string& Name, uint32_t MinAge, uint32_t MaxAge, const RuleSet* Rules, const StandingData& StandingData)
	: m_Name(Name), m_pRules(Rules), m_StandingData(StandingData)
{
	m_MinAge = MinAge;
	m_MaxAge = MaxAge;
}



AgeGroup::AgeGroup(const YAML::Node& Yaml, const StandingData& StandingData)
	: m_StandingData(StandingData)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();
	if (Yaml["min_age"])
		m_MinAge = Yaml["min_age"].as<uint32_t>();
	if (Yaml["max_age"])
		m_MaxAge = Yaml["max_age"].as<uint32_t>();
	if (Yaml["gender"])
		m_Gender = (Gender)Yaml["gender"].as<uint32_t>();

	if (Yaml["rules"])
		m_pRules = StandingData.FindRuleSet(Yaml["rules"].as<std::string>());
}



AgeGroup::AgeGroup(const MD5::AgeGroup& AgeGroup, const StandingData& StandingData)
	: m_StandingData(StandingData)
{
	m_Name   = AgeGroup.Name;
	m_Gender = AgeGroup.Gender;

	auto year = StandingData.GetYear();
	m_MinAge = year - AgeGroup.MaxBirthyear + 1;
	m_MaxAge = year - AgeGroup.MinBirthyear + 1;
}



bool AgeGroup::IsElgiable(const Judoka& Fighter) const
{
	if (m_Gender != Gender::Unknown)
		if (m_Gender != Fighter.GetGender())
			return false;

	uint32_t age = m_StandingData.GetYear() - Fighter.GetBirthyear();

	if (m_MinAge == 0)//No min age
		return age <= m_MaxAge;
	if (m_MaxAge == 0)//No max age
		return m_MinAge <= age;

	return m_MinAge <= age && age <= m_MaxAge;
}



void AgeGroup::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;
	Yaml << YAML::Key << "uuid"    << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "name"    << YAML::Value << m_Name;
	Yaml << YAML::Key << "min_age" << YAML::Value << m_MinAge;
	Yaml << YAML::Key << "max_age" << YAML::Value << m_MaxAge;

	if (m_Gender != Gender::Unknown)
		Yaml << YAML::Key << "gender" << YAML::Value << (int)m_Gender;

	if (m_pRules)
		Yaml << YAML::Key << "rules" << YAML::Value << (std::string)m_pRules->GetUUID();

	Yaml << YAML::EndMap;
}



void AgeGroup::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "uuid"    << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "name"    << YAML::Value << m_Name;
	Yaml << YAML::Key << "min_age" << YAML::Value << m_MinAge;
	Yaml << YAML::Key << "max_age" << YAML::Value << m_MaxAge;

	if (m_Gender != Gender::Unknown)
		Yaml << YAML::Key << "gender" << YAML::Value << (int)m_Gender;

	std::string desc;
	if (m_MinAge == 0)
		desc = "-" + std::to_string(m_MaxAge);
	else if (m_MaxAge == 0)
		desc = std::to_string(m_MinAge) + "+";
	else
		desc = std::to_string(m_MinAge) + " - " + std::to_string(m_MaxAge);

	if (m_Gender != Gender::Unknown)
		desc += " " + Localizer::Gender2ShortForm(m_Gender);

	Yaml << YAML::Key << "desc" << YAML::Value << desc;

	if (m_pRules)
	{
		Yaml << YAML::Key << "rules_name" << YAML::Value << m_pRules->GetName();
		Yaml << YAML::Key << "rules_uuid" << YAML::Value << (std::string)m_pRules->GetUUID();
	}
}