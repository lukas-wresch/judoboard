#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "association.h"
#include "standing_data.h"



using namespace Judoboard;



Association::Association(const YAML::Node& Yaml, const StandingData* StandingData)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["name"])
		m_Name = Yaml["name"].as<std::string>();

	if (Yaml["parent"] && StandingData)
		m_pParent = StandingData->FindAssociation(Yaml["parent"].as<std::string>());
}



bool Association::IsChildOf(const UUID& UUID) const
{
	if (*this == UUID)//Are we our own child?
		return true;

	if (m_pParent)
		return m_pParent->IsChildOf(UUID);
	return false;
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



bool Association::operator < (const Association& rhs) const
{
	if (this->GetLevel() < rhs.GetLevel())
		return true;
	if (this->GetLevel() > rhs.GetLevel())
		return false;

	return (this->GetName() < rhs.GetName());
}