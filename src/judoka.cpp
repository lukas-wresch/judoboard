#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../ZED/include/core.h"
#include "judoka.h"
#include "database.h"



using namespace Judoboard;



bool DependentJudoka::operator == (const Judoka* rhs) const
{
	if (!m_Judoka || !rhs)
		return false;
	return *m_Judoka == *rhs;
}



JudokaData::JudokaData(const Judoka& Judoka)
{
	m_Firstname = Judoka.GetFirstname();
	m_Lastname  = Judoka.GetLastname();
	m_Weight    = Judoka.GetWeight();
	m_Gender    = Judoka.GetGender();
	m_Birthyear = Judoka.GetBirthyear();
	m_Number    = Judoka.GetNumber();
	if (Judoka.GetClub())
		m_ClubName = Judoka.GetClub()->GetName();
}



JudokaData::JudokaData(const MD5::Participant& Judoka)
{
	m_Firstname = Judoka.Firstname;
	m_Lastname  = Judoka.Lastname;
	if (Judoka.WeightInGrams > 0)
		m_Weight.SetWeightInGrams(Judoka.WeightInGrams);
	if (Judoka.AgeGroup)
		m_Gender = Judoka.AgeGroup->Gender;
	if (Judoka.Birthyear >= 0)
		m_Birthyear = Judoka.Birthyear;
	m_ClubName = Judoka.ClubFullname;
}



JudokaData::JudokaData(const DM4::Participant& Judoka)
{
	m_Firstname = Judoka.Firstname;
	m_Lastname  = Judoka.Lastname;
	if (Judoka.WeightInGrams > 0)
		m_Weight.SetWeightInGrams(Judoka.WeightInGrams);
	m_Gender = Judoka.Gender;
	if (Judoka.Birthyear >= 0)
		m_Birthyear = Judoka.Birthyear;
	if (Judoka.Club)
		m_ClubName = Judoka.Club->Name;
}



JudokaData::JudokaData(const DMF::Participant& Judoka)
{
	m_Firstname = Judoka.Firstname;
	m_Lastname  = Judoka.Lastname;
	if (Judoka.WeightInGrams > 0)
		m_Weight.SetWeightInGrams(Judoka.WeightInGrams);
	m_Gender = Judoka.Gender;
	if (Judoka.Birthyear >= 0)
		m_Birthyear = Judoka.Birthyear;
	m_ClubName = Judoka.ClubName;
}



Judoka::Judoka(const std::string& Firstname, const std::string& Lastname, Weight Weight, Gender Gender, uint32_t Birthyear)
	: m_Firstname(Firstname), m_Lastname(Lastname), m_Weight(Weight)
{
	if (Gender == Gender::Male || Gender == Gender::Female)
		m_Gender = Gender;

	m_Birthyear = Birthyear;
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
		m_Weight = Weight(Yaml["weight"]);
	if (Yaml["gender"])
		m_Gender = (Gender)Yaml["gender"].as<int>();
	if (Yaml["birthyear"])
		m_Birthyear = Yaml["birthyear"].as<int>();
	if (Yaml["number"])
		m_Number = Yaml["number"].as<std::string>();
	if (Yaml["club"] && pStandingData)
		m_pClub = pStandingData->FindClub(Yaml["club"].as<std::string>());
}



Judoka::Judoka(const JudokaData& JudokaData, const StandingData* pStandingData)
{
	m_Firstname = JudokaData.m_Firstname;
	m_Lastname  = JudokaData.m_Lastname;
	m_Gender    = JudokaData.m_Gender;
	m_Number    = JudokaData.m_Number;

	if (JudokaData.m_Birthyear > 0)
		m_Birthyear = JudokaData.m_Birthyear;
	if (JudokaData.m_Weight > 0)
		m_Weight = JudokaData.m_Weight;

	if (pStandingData)
		m_pClub = pStandingData->FindClubByName(JudokaData.m_ClubName);
}



Judoka::Judoka(const DM4::Participant& Participant, const StandingData* pStandingData)
{
	m_Firstname = Participant.Firstname;
	m_Lastname  = Participant.Lastname;
	m_Gender    = Participant.Gender;

	if (Participant.Birthyear > 0)
		m_Birthyear = Participant.Birthyear;
	if (Participant.WeightInGrams > 0)
		m_Weight = Participant.WeightInGrams;

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
	if (Participant.WeightInGrams > 0)
		m_Weight = Participant.WeightInGrams;

	if (pStandingData && Participant.Club)
		m_pClub = pStandingData->FindClubByName(Participant.Club->Name);
}



Judoka::Judoka(const DMF::Participant& Participant)
{
	m_Firstname = Participant.Firstname;
	m_Lastname  = Participant.Lastname;

	if (Participant.Birthyear > 0)
		m_Birthyear = Participant.Birthyear;
	if (Participant.WeightInGrams > 0)
		m_Weight = Participant.WeightInGrams;
}



void Judoka::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;
	Yaml << YAML::Key << "uuid"      << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "firstname" << YAML::Value << m_Firstname;
	Yaml << YAML::Key << "lastname"  << YAML::Value << m_Lastname;

	Yaml << YAML::Key << "weight"    << YAML::Value;
	m_Weight >> Yaml;

	Yaml << YAML::Key << "gender"    << YAML::Value << (int)m_Gender;
	Yaml << YAML::Key << "birthyear" << YAML::Value << m_Birthyear;

	if (!m_Number.empty())
		Yaml << YAML::Key << "number" << YAML::Value << m_Number;

	if (m_pClub)
		Yaml << YAML::Key << "club"  << YAML::Value << (std::string)m_pClub->GetUUID();
	Yaml << YAML::EndMap;
}



void Judoka::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;
	Yaml << YAML::Key << "uuid"      << YAML::Value << (std::string)GetUUID();
	Yaml << YAML::Key << "firstname" << YAML::Value << m_Firstname;
	Yaml << YAML::Key << "lastname"  << YAML::Value << m_Lastname;
	Yaml << YAML::Key << "weight"    << YAML::Value << m_Weight.ToString();
	Yaml << YAML::Key << "gender"    << YAML::Value << (int)m_Gender;
	Yaml << YAML::Key << "birthyear" << YAML::Value << m_Birthyear;
	Yaml << YAML::Key << "number"    << YAML::Value << m_Number;

	if (m_pClub)
	{
		Yaml << YAML::Key << "club_uuid" << YAML::Value << (std::string)m_pClub->GetUUID();
		Yaml << YAML::Key << "club_name" << YAML::Value << m_pClub->GetName();
	}
	Yaml << YAML::EndMap;
}



uint16_t Judoka::GetAge() const
{
	auto date = ZED::Core::GetDate();
	return date.year - m_Birthyear;
}



void Judoka::SetWeight(Weight NewWeight)
{
	if (NewWeight < Weight(1000))
		m_Weight = NewWeight;
}