#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../ZED/include/core.h"
#include "age_group.h"



using namespace Judoboard;



AgeGroup::AgeGroup(const std::string& Name) : m_Name(Name)
{	
}



AgeGroup::AgeGroup(const YAML::Node& Yaml)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();
	if (Yaml["min_age"])
		m_MinAge = Yaml["min_age"].as<uint32_t>();
	if (Yaml["max_age"])
		m_MinAge = Yaml["max_age"].as<uint32_t>();
}



void AgeGroup::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;
	Yaml << YAML::Key << "uuid"    << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "name"    << YAML::Value << m_Name;
	Yaml << YAML::Key << "min_age" << YAML::Value << m_MinAge;
	Yaml << YAML::Key << "max_age" << YAML::Value << m_MaxAge;
	Yaml << YAML::EndMap;
}