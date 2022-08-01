#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "association.h"



using namespace Judoboard;



Association::Association(const std::string& Name) : m_Name(Name)
{	
}



Association::Association(const YAML::Node& Yaml)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();
}



void Association::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "name" << YAML::Value << m_Name;

	if (m_pParent)
		Yaml << YAML::Key << "parent" << YAML::Value << (std::string)m_pParent->GetUUID();

	Yaml << YAML::EndMap;
}