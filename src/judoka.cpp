#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../ZED/include/core.h"
#include "judoka.h"
#include "database.h"



using namespace Judoboard;



Judoka::Judoka(const std::string& Firstname, const std::string& Lastname, uint32_t Weight, Gender Gender) : m_Firstname(Firstname), m_Lastname(Lastname)
{
	if (Weight < 1000 * 1000)
		m_Weight = Weight;
	if (Gender == Gender::Male || Gender == Gender::Female)
		m_Gender = Gender;
}



Judoka::Judoka(ZED::CSV& Stream, const StandingData* pStandingData)
{
	std::string uuid, club_uuid;
	Stream >> m_Firstname >> m_Lastname >> m_Weight >> m_Gender >> m_Birthyear >> uuid >> club_uuid;
	SetUUID(std::move(uuid));

	if (pStandingData && club_uuid.length() > 1)
		m_pClub = pStandingData->FindClub(UUID(std::move(club_uuid)));
}



Judoka::Judoka(const YAML::Node& Yaml, const StandingData* pStandingData)
{
	if (Yaml["uuid"])
		SetUUID(Yaml["uuid"].as<std::string>());
	if (Yaml["firstname"])
		m_Firstname = Yaml["firstname"].as<std::string>();
	if (Yaml["lastname"])
		m_Lastname = Yaml["lastname"].as<std::string>();
	if (Yaml["weight"])
		m_Weight = Yaml["weight"].as<int>();
	if (Yaml["gender"])
		m_Gender = (Gender)Yaml["gender"].as<int>();
	if (Yaml["birthyear"])
		m_Birthyear = Yaml["birthyear"].as<int>();
	if (Yaml["club"] && pStandingData)
		m_pClub = pStandingData->FindClub(Yaml["club"].as<std::string>());
}



Judoka::Judoka(const DM4::Participant& Participant, const StandingData* pStandingData)
{
	m_Firstname = Participant.Firstname;
	m_Lastname  = Participant.Lastname;
	m_Gender    = Participant.Gender;

	if (Participant.Birthyear > 0)
		m_Birthyear = Participant.Birthyear;
	if (Participant.Weight > 0)
		m_Weight = Participant.Weight;

	if (pStandingData && Participant.Club)
		m_pClub = pStandingData->FindClubByName(Participant.Club->Name);
}



Judoka::Judoka(const MD5::Participant& Participant, const StandingData* pStandingData)
{
	m_Firstname = Participant.Firstname;
	m_Lastname  = Participant.Lastname;
	if (Participant.AgeGroup)//Try to determine the gender
		m_Gender = Participant.AgeGroup->Gender;

	if (Participant.Birthyear > 0)
		m_Birthyear = Participant.Birthyear;
	if (Participant.WeightInGramm > 0)
		m_Weight = Participant.WeightInGramm/1000;//TODO Loss of information!!

	if (pStandingData && Participant.Club)
		m_pClub = pStandingData->FindClubByName(Participant.Club->Name);
}



void Judoka::operator >> (ZED::CSV& Stream) const
{
	Stream << m_Firstname;
	Stream << m_Lastname;
	Stream << m_Weight << m_Gender << m_Birthyear;
	Stream << (std::string)GetUUID();

	if (m_pClub)
		Stream << (std::string)m_pClub->GetUUID();
	else
		Stream << "?";

	Stream.AddNewline();//Also needed to flush the stream
}



void Judoka::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;
	Yaml << YAML::Key << "uuid"      << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "firstname" << YAML::Value << m_Firstname;
	Yaml << YAML::Key << "lastname"  << YAML::Value << m_Lastname;
	Yaml << YAML::Key << "weight"    << YAML::Value << m_Weight;
	Yaml << YAML::Key << "gender"    << YAML::Value << (int)m_Gender;
	Yaml << YAML::Key << "birthyear" << YAML::Value << m_Birthyear;
	if (m_pClub)
		Yaml << YAML::Key << "club"  << YAML::Value << (std::string)m_pClub->GetUUID();
	Yaml << YAML::EndMap;
}



uint16_t Judoka::GetAge() const
{
	auto date = ZED::Core::GetDate();
	return date.year - m_Birthyear;
}



void Judoka::SetWeight(uint32_t NewWeight)
{
	if (NewWeight < 1000 * 1000)
		m_Weight = NewWeight;
}