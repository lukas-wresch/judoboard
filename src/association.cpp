#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "association.h"
#include "standing_data.h"



using namespace Judoboard;



Association::Association(const std::string& Name, const Association* Parent) : m_Name(Name), m_pParent(Parent)
{
	if (Parent)
		m_Level = m_pParent->GetLevel() + 1;
}



Association::Association(const YAML::Node& Yaml, const StandingData* StandingData)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();

	if (Yaml["parent"] && StandingData)
	{
		m_pParent = StandingData->FindAssociation(Yaml["parent"].as<std::string>());
		if (m_pParent)
			m_Level = m_pParent->GetLevel() + 1;
	}
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



void Association::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	Yaml << YAML::Key << "uuid"  << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "name"  << YAML::Value << m_Name;
	Yaml << YAML::Key << "level" << YAML::Value << GetLevel();

	if (m_pParent)
	{
		Yaml << YAML::Key << "parent_uuid" << YAML::Value << (std::string)m_pParent->GetUUID();
		Yaml << YAML::Key << "parent_name" << YAML::Value << m_pParent->GetName();
	}

	Yaml << YAML::EndMap;
}