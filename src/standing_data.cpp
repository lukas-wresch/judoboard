#include <fstream>
#include <random>
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../ZED/include/log.h"
#include "../ZED/include/sha512.h"
#include "standing_data.h"
#include "itournament.h"



using namespace Judoboard;



void StandingData::Reset()
{
	for (auto [id, judoka] : m_Judokas)
		delete judoka;
	m_Judokas.clear();

	for (auto rule : m_RuleSets)
		delete rule;
	m_RuleSets.clear();

	for (auto age_group : m_AgeGroups)
		delete age_group;
	m_AgeGroups.clear();

	m_Year = 0;
}



void StandingData::operator << (const YAML::Node& Yaml)
{
	if (Yaml["year"])
		m_Year = Yaml["year"].as<uint32_t>();


	if (Yaml["associations"] && Yaml["associations"].IsSequence())
	{
		m_Associations.clear();

		for (const auto& node : Yaml["associations"])
		{
			Association* newAssoc = new Association(node, this);
			m_Associations.emplace_back(newAssoc);
		}
	}


	if (Yaml["clubs"] && Yaml["clubs"].IsSequence())
	{
		m_Clubs.clear();

		for (const auto& node : Yaml["clubs"])
		{
			Club* newClub = new Club(node, this);
			m_Clubs.emplace_back(newClub);
		}
	}


	if (Yaml["judoka"] && Yaml["judoka"].IsSequence())
	{
		m_Judokas.clear();

		for (const auto& node : Yaml["judoka"])
		{
			Judoka* newJudoka = new Judoka(node, this);
			m_Judokas.insert({ (std::string)newJudoka->GetUUID(), newJudoka });
		}
	}


	if (Yaml["rule_sets"] && Yaml["rule_sets"].IsSequence())
	{
		m_RuleSets.clear();

		for (const auto& node : Yaml["rule_sets"])
		{
			auto new_rule_set = new RuleSet(node);
			m_RuleSets.emplace_back(new_rule_set);
		}
	}


	if (Yaml["age_groups"] && Yaml["age_groups"].IsSequence())
	{
		m_AgeGroups.clear();

		for (const auto& node : Yaml["age_groups"])
		{
			auto new_age_group = new AgeGroup(node, *this);
			m_AgeGroups.emplace_back(new_age_group);
		}
	}
}



void StandingData::operator >> (YAML::Emitter& Yaml) const
{
	if (m_Year > 0)//No the default (current) year?
		Yaml << YAML::Key << "year" << YAML::Value << m_Year;

	Yaml << YAML::Key << "associations";
	Yaml << YAML::Value;
	Yaml << YAML::BeginSeq;

	for (auto assoc : m_Associations)
	{
		if (assoc)
			*assoc >> Yaml;
	}

	Yaml << YAML::EndSeq;

	Yaml << YAML::Key << "clubs";
	Yaml << YAML::Value;
	Yaml << YAML::BeginSeq;

	for (auto club : m_Clubs)
	{
		if (club)
			*club >> Yaml;
	}

	Yaml << YAML::EndSeq;

	Yaml << YAML::Key << "judoka";
	Yaml << YAML::Value;
	Yaml << YAML::BeginSeq;

	for (auto [id, judoka] : m_Judokas)
	{
		if (judoka)
			*judoka >> Yaml;
	}

	Yaml << YAML::EndSeq;
	
	//Export rule sets
	Yaml << YAML::Key << "rule_sets";
	Yaml << YAML::Value;
	Yaml << YAML::BeginSeq;

	for (auto rule : m_RuleSets)
		if (rule)
			*rule >> Yaml;

	Yaml << YAML::EndSeq;

	//Export age groups
	Yaml << YAML::Key << "age_groups";
	Yaml << YAML::Value;
	Yaml << YAML::BeginSeq;

	for (auto age_group : m_AgeGroups)
		if (age_group)
			*age_group >> Yaml;

	Yaml << YAML::EndSeq;
}



void StandingData::AddMD5File(const MD5& File)
{
	for (auto club : File.GetClubs())
		AddClub(new Club(*club));

	for (auto judoka : File.GetParticipants())
		AddJudoka(new Judoka(JudokaData(*judoka), this));
}



uint32_t StandingData::GetYear() const
{
	if (m_Year != 0)
		return m_Year;

	auto date = ZED::Core::GetDate();
	return date.year;
}



bool StandingData::AddJudoka(Judoka* NewJudoka)
{
	if (!NewJudoka)
		return false;

	m_Judokas.insert({ NewJudoka->GetUUID(), NewJudoka });
	return true;
}



bool StandingData::DeleteJudoka(const UUID& UUID)
{
	for (auto it = m_Judokas.begin(); it != m_Judokas.end(); ++it)
	{
		if (it->second && it->second->GetUUID() == UUID)
		{
			m_Judokas.erase(it);
			return true;
		}
	}

	return false;
}



const std::string StandingData::Judoka2String(std::string SearchString, const ITournament* Tournament) const
{
	if (!Tournament)
		return "";
	
	std::transform(SearchString.begin(), SearchString.end(), SearchString.begin(),
		[](unsigned char c){ return std::tolower(c); });

	YAML::Emitter ret;
	ret << YAML::BeginSeq;

	for (auto [id, judoka] : m_Judokas)
	{
		auto name = judoka->GetName(NameStyle::GivenName);
		std::transform(name.begin(), name.end(), name.begin(),
			[](unsigned char c){ return std::tolower(c); });

		if (name.find(SearchString) == std::string::npos)
			continue;

		ret << YAML::BeginMap;

		ret << YAML::Key << "uuid" << YAML::Value << (std::string)judoka->GetUUID();
		ret << YAML::Key << "name" << YAML::Value << judoka->GetName(NameStyle::GivenName);
		ret << YAML::Key << "weight" << YAML::Value << judoka->GetWeight().ToString();
		ret << YAML::Key << "birthyear" << YAML::Value << judoka->GetBirthyear();

		if (judoka->GetClub())
			ret << YAML::Key << "club" << YAML::Value << judoka->GetClub()->GetName();

		ret << YAML::Key << "is_participant" << YAML::Value << Tournament->IsParticipant(*judoka);

		auto judoka_age_group = Tournament->GetAgeGroupOfJudoka(judoka);
		if (judoka_age_group)
			ret << YAML::Key << "age_group_uuid" << YAML::Value << (std::string)judoka_age_group->GetUUID();

		//Calculate eligable age groups
		ret << YAML::Key << "age_groups" << YAML::Value;
		ret << YAML::BeginSeq;

		auto age_groups = Tournament->GetEligableAgeGroupsOfJudoka(judoka);
		for (auto age_group : age_groups)
		{
			ret << YAML::BeginMap;
			ret << YAML::Key << "uuid" << YAML::Value << (std::string)age_group->GetUUID();
			ret << YAML::Key << "name" << YAML::Value << age_group->GetName();
			ret << YAML::EndMap;
		}

		ret << YAML::EndSeq;

		ret << YAML::EndMap;
	}

	ret << YAML::EndSeq;
	return ret.c_str();
}



Club* StandingData::AddClub(const MD5::Club& NewClub)
{
	auto club_to_update = FindClubByName(NewClub.Name);
	if (club_to_update)
	{
		//TODO update addtional club information
		return club_to_update;
	}

	//Add the club to the database
	auto new_club = new Club(NewClub);
	if (!AddClub(new_club))
	{
		ZED::Log::Error("Could not add MD5 club to database");
		delete new_club;
		new_club = nullptr;
	}

	return new_club;
}



bool StandingData::AddClub(Club* NewClub)
{
	if (!NewClub)
		return false;

	if (FindClub(NewClub->GetUUID()))
		return false;

	AddAssociation(const_cast<Association*>(NewClub->GetParent()));

	m_Clubs.emplace_back(NewClub);
	return true;
}



Club* StandingData::FindClub(const UUID& UUID)
{
	for (auto club : m_Clubs)
		if (club && club->GetUUID() == UUID)
			return club;

	return nullptr;
}



const Club* StandingData::FindClub(const UUID& UUID) const
{
	for (auto club : m_Clubs)
		if (club && club->GetUUID() == UUID)
			return club;

	return nullptr;
}



Club* StandingData::FindClubByName(const std::string& Name)
{
	for (auto club : m_Clubs)
		if (club && club->GetName() == Name)
			return club;

	return nullptr;
}



const Club* StandingData::FindClubByName(const std::string& Name) const
{
	for (auto club : m_Clubs)
		if (club && club->GetName() == Name)
			return club;

	return nullptr;
}



bool StandingData::DeleteClub(const UUID& UUID)
{
	for (auto club = m_Clubs.begin(); club != m_Clubs.end(); ++club)
		if (*club && (*club)->GetUUID() == UUID)
		{
			//Check if there is any judoka associated to this club
			for (auto [id, judoka] : m_Judokas)
			{
				if (judoka->GetClub() && *judoka->GetClub() == UUID)
					return false;
			}

			m_Clubs.erase(club);
			return true;
		}

	return false;
}



bool StandingData::AddAssociation(Association* NewAssociation)
{
	if (!NewAssociation)
		return false;

	if (FindAssociation(NewAssociation->GetUUID()))
		return false;

	//Add recursively
	AddAssociation(const_cast<Association*>(NewAssociation->GetParent()));

	m_Associations.emplace_back(NewAssociation);

	//std::sort would normally order the vector by memory address :-(
	std::sort(m_Associations.begin(), m_Associations.end(), [](auto a, auto b) { return *a < *b; });

	return true;
}



Association* StandingData::FindAssociation(const UUID& UUID)
{
	for (auto assoc : m_Associations)
		if (assoc && assoc->GetUUID() == UUID)
			return assoc;

	return nullptr;
}



const Association* StandingData::FindAssociation(const UUID& UUID) const
{
	for (auto assoc : m_Associations)
		if (assoc && assoc->GetUUID() == UUID)
			return assoc;

	return nullptr;
}



const Association* StandingData::FindAssociationByName(const std::string& Name) const
{
	for (auto assoc : m_Associations)
		if (assoc && assoc->GetName() == Name)
			return assoc;

	return nullptr;
}



bool StandingData::DeleteAssociation(const UUID& UUID)
{
	for (auto assoc = m_Associations.begin(); assoc != m_Associations.end(); ++assoc)
		if (*assoc && (*assoc)->GetUUID() == UUID)
		{
			//Check if this association has childen
			for (auto possible_child : m_Associations)
				if (possible_child->GetParent() && *possible_child->GetParent() == UUID)
					return false;

			m_Associations.erase(assoc);
			return true;
		}

	return false;
}



bool StandingData::AssociationHasChildren(const Association* Association) const
{
	if (!Association)
		return false;

	for (auto assoc : m_Associations)
	{
		if (assoc && assoc->GetParent() && *assoc->GetParent() == *Association)
			return true;
	}

	return false;
}



RuleSet* StandingData::FindRuleSetByName(const std::string& RuleSetName)
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetName() == RuleSetName)
			return rule;

	return nullptr;
}



const RuleSet* StandingData::FindRuleSetByName(const std::string& RuleSetName) const
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetName() == RuleSetName)
			return rule;

	return nullptr;
}



RuleSet* StandingData::FindRuleSet(const UUID& UUID)
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetUUID() == UUID)
			return rule;

	return nullptr;
}



const RuleSet* StandingData::FindRuleSet(const UUID& UUID) const
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetUUID() == UUID)
			return rule;

	return nullptr;
}



bool StandingData::DeleteRuleSet(const UUID& UUID)
{
	for (auto it = m_RuleSets.begin(); it != m_RuleSets.end(); ++it)
	{
		if ((*it)->GetUUID() == UUID)
		{
			delete *it;
			m_RuleSets.erase(it);
			return true;
		}
	}

	return false;
}



bool StandingData::AddRuleSet(RuleSet* NewRuleSet)
{
	if (!NewRuleSet || FindRuleSet(NewRuleSet->GetUUID()))
		return false;

	m_RuleSets.emplace_back(NewRuleSet);
	return true;
}



AgeGroup* StandingData::FindAgeGroupByName(const std::string& AgeGroupName)
{
	for (auto age_group : m_AgeGroups)
		if (age_group && age_group->GetName() == AgeGroupName)
			return age_group;

	return nullptr;
}



const AgeGroup* StandingData::FindAgeGroupByName(const std::string& AgeGroupName) const
{
	for (auto age_group : m_AgeGroups)
		if (age_group && age_group->GetName() == AgeGroupName)
			return age_group;

	return nullptr;
}



AgeGroup* StandingData::FindAgeGroup(const UUID& UUID)
{
	for (auto age_group : m_AgeGroups)
		if (age_group && age_group->GetUUID() == UUID)
			return age_group;

	return nullptr;
}



const AgeGroup* StandingData::FindAgeGroup(const UUID& UUID) const
{
	for (auto age_group : m_AgeGroups)
		if (age_group && age_group->GetUUID() == UUID)
			return age_group;

	return nullptr;
}



bool StandingData::AddAgeGroup(AgeGroup* NewAgeGroup)
{
	if (!NewAgeGroup || FindAgeGroup(NewAgeGroup->GetUUID()))
		return false;

	m_AgeGroups.emplace_back(NewAgeGroup);
	return true;
}



bool StandingData::RemoveAgeGroup(const UUID& UUID)
{
	for (auto it = m_AgeGroups.begin(); it != m_AgeGroups.end(); ++it)
	{
		if ((*it)->GetUUID() == UUID)
		{
			delete *it;
			m_AgeGroups.erase(it);
			return true;
		}
	}

	return false;
}



Judoka* StandingData::FindJudoka(const UUID& UUID)
{
	for (auto [id, judoka] : m_Judokas)
	{
		if (judoka && judoka->GetUUID() == UUID)
			return judoka;
	}

	return nullptr;
}



const Judoka* StandingData::FindJudoka(const UUID& UUID) const
{
	for (auto [id, judoka] : m_Judokas)
	{
		if (judoka && judoka->GetUUID() == UUID)
			return judoka;
	}

	return nullptr;
}



const Judoka* StandingData::FindJudokaByName(const std::string& Name) const
{
	for (auto [id, judoka] : m_Judokas)
	{
		if (judoka && judoka->GetName(NameStyle::GivenName) == Name)
			return judoka;
	}

	return nullptr;
}



Judoka* StandingData::FindJudoka_ExactMatch(const JudokaData& NewJudoka)
{
	Judoka* ret = nullptr;

	for (auto [id, judoka] : m_Judokas)
	{
		if (judoka && judoka->GetFirstname()  == NewJudoka.m_Firstname
			&& judoka->GetLastname()   == NewJudoka.m_Lastname
			&& judoka->GetGender()     == NewJudoka.m_Gender
			&& (judoka->GetBirthyear() == NewJudoka.m_Birthyear || judoka->GetBirthyear() == 0 || NewJudoka.m_Birthyear == 0)
			&& (!judoka->GetClub() || judoka->GetClub()->GetName() == NewJudoka.m_ClubName) )
		{
			if (ret)//Have we found one?
				return nullptr;//Then this is not an exact match

			ret = judoka;
		}
	}

	return ret;
}



Judoka* StandingData::FindJudoka_SameName(const JudokaData& NewJudoka)
{
	Judoka* ret = nullptr;

	for (auto [id, judoka] : m_Judokas)
	{
		if (judoka && judoka->GetFirstname() == NewJudoka.m_Firstname
				   && judoka->GetLastname()  == NewJudoka.m_Lastname
				   && judoka->GetGender()    == NewJudoka.m_Gender)
		{
			if (ret)//Have we found one?
				return nullptr;//Then this is not an exact match

			ret = judoka;
		}
	}

	return ret;
}