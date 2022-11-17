#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "association.h"
#include "standing_data.h"



using namespace Judoboard;



Association::Association(const std::string& Name, const Association* Parent) : m_Name(Name), m_pParent(Parent)
{
	m_ShortName = m_Name.substr(0, 5);
}



Association::Association(const YAML::Node& Yaml, const StandingData* StandingData)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["name"])
		SetName(Yaml["name"].as<std::string>());
	if (Yaml["short_name"])
		SetShortName(Yaml["short_name"].as<std::string>());

	if (Yaml["parent"] && StandingData)
		m_pParent = StandingData->FindAssociation(Yaml["parent"].as<std::string>());
}



void Association::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "name" << YAML::Value << m_Name;
	Yaml << YAML::Key << "short_name" << YAML::Value << m_ShortName;

	if (m_pParent)
		Yaml << YAML::Key << "parent" << YAML::Value << (std::string)m_pParent->GetUUID();

	Yaml << YAML::EndMap;
}



void Association::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	Yaml << YAML::Key << "uuid"  << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "name"  << YAML::Value << m_Name;
	Yaml << YAML::Key << "short_name" << YAML::Value << m_ShortName;
	Yaml << YAML::Key << "level"      << YAML::Value << GetLevel();

	if (m_pParent)
	{
		Yaml << YAML::Key << "parent_uuid" << YAML::Value << (std::string)m_pParent->GetUUID();
		Yaml << YAML::Key << "parent_name" << YAML::Value << m_pParent->GetName();
	}

	Yaml << YAML::EndMap;
}



bool Association::operator < (const Association& rhs) const
{
	if (this->GetLevel() < rhs.GetLevel())
		return true;
	if (this->GetLevel() > rhs.GetLevel())
		return false;

	return (this->GetName() < rhs.GetName());
}