#pragma once
#include <string>
#include <unordered_map>
#include "judoka.h"
#include "club.h"
#include "rule_set.h"
#include "age_group.h"
#include "dm4.h"


namespace YAML
{
	class Emitter;
	class Node;
}



namespace Judoboard
{
	class StandingData
	{
	public:
		StandingData() = default;
		void Reset();

		void operator << (YAML::Node& Yaml);
		void operator >> (YAML::Emitter& Yaml) const;

		void AddMD5File(const MD5& File);

		uint32_t GetYear() const;
		void SetYear(uint32_t NewYear) { m_Year = NewYear; }

		//Judokas
		uint32_t AddJudoka(Judoka&& NewJudoka) { return AddJudoka(new Judoka(NewJudoka)); }//Adds a judoka to the database
		bool AddJudoka(Judoka* NewJudoka);//Adds a judoka to the database by reference
		const std::unordered_map<UUID, Judoka*>& GetAllJudokas() const { return m_Judokas; }
		std::unordered_map<UUID, Judoka*>& GetAllJudokas() { return m_Judokas; }
		size_t GetNumJudoka() const { return m_Judokas.size(); }
		size_t GetNumClubs()  const { return m_Clubs.size(); }

		Judoka* FindJudoka(const UUID& UUID);
		const Judoka* FindJudoka(const UUID& UUID) const;

		Judoka* FindJudoka_ExactMatch(const Judoka& NewJudoka);
		Judoka* FindJudoka_SameName(const Judoka& NewJudoka);

		Judoka* FindJudoka_DM4_ExactMatch(const DM4::Participant& NewJudoka);
		Judoka* FindJudoka_DM4_SameName(const DM4::Participant& NewJudoka);

		bool DeleteJudoka(const UUID& UUID);

		//Clubs
		auto& GetAllClubs() const { return m_Clubs; }
		Club* AddClub(const MD5::Club& NewClub);
		bool  AddClub(Club* NewClub);

		Club* FindClub(const UUID& UUID);
		const Club* FindClub(const UUID& UUID) const;
		Club* FindClubByName(const std::string& Name);
		const Club* FindClubByName(const std::string& Name) const;

		bool DeleteClub(const UUID& UUID);

		//Associations
		auto& GetAllAssociations() const { return m_Associations; }
		bool AddAssociation(Association* NewAssociation);
		Association* FindAssociation(const UUID& UUID);
		const Association* FindAssociation(const UUID& UUID) const;
		bool DeleteAssociation(const UUID& UUID);

		//Rule sets
		RuleSet* FindRuleSetByName(const std::string& RuleSetName);
		const RuleSet* FindRuleSetByName(const std::string& RuleSetName) const;
		RuleSet* FindRuleSet(const UUID& UUID);
		const RuleSet* FindRuleSet(const UUID& UUID) const;

		bool AddRuleSet(RuleSet* NewRuleSet);
		std::vector<RuleSet*>& GetRuleSets() { return m_RuleSets; }
		const std::vector<RuleSet*>& GetRuleSets() const { return m_RuleSets; }

		//Age groups
		AgeGroup* FindAgeGroupByName(const std::string& AgeGroupName);
		const AgeGroup* FindAgeGroupByName(const std::string& AgeGroupName) const;
		AgeGroup* FindAgeGroup(const UUID& UUID);
		const AgeGroup* FindAgeGroup(const UUID& UUID) const;

		bool AddAgeGroup(AgeGroup* NewAgeGroup);
		bool RemoveAgeGroup(const UUID& UUID);
		std::vector<AgeGroup*>& GetAgeGroups() { return m_AgeGroups; }
		const std::vector<AgeGroup*>& GetAgeGroups() const { return m_AgeGroups; }

		//Serialization
		const std::string Judoka2String(std::string SearchString, const ITournament* Tournament) const;

	protected:
		std::unordered_map<UUID, Judoka*> m_Judokas;

		std::vector<Club*> m_Clubs;
		std::vector<Association*> m_Associations;

		std::vector<RuleSet*> m_RuleSets;
		std::vector<AgeGroup*> m_AgeGroups;

	private:
		uint32_t m_Year = 0;//Relative year for calculating the age groups. Set to 0 to use the current year
	};
}