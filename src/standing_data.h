#pragma once
#include <string>
#include <mutex>
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

		void operator << (const YAML::Node& Yaml);
		void operator >> (YAML::Emitter& Yaml) const;

		void AddMD5File(const MD5& File);

		[[nodiscard]]
		uint32_t GetYear() const;
		void SetYear(uint32_t NewYear) { m_Year = NewYear; }

		//Judokas
		bool AddJudoka(Judoka&& NewJudoka) { return AddJudoka(new Judoka(NewJudoka)); }//Adds a judoka to the database
		bool AddJudoka(Judoka* NewJudoka);//Adds a judoka to the database by reference
		[[nodiscard]] const std::vector<Judoka*>& GetAllJudokas() const { return m_Judokas; }
		[[nodiscard]] std::vector<Judoka*>& GetAllJudokas() { return m_Judokas; }
		[[nodiscard]] size_t GetNumJudoka() const { return m_Judokas.size(); }
		[[nodiscard]] size_t GetNumClubs()  const { return m_Clubs.size(); }

		[[nodiscard]] Judoka* FindJudoka(const UUID& UUID);
		[[nodiscard]] const Judoka* FindJudoka(const UUID& UUID) const;
		[[nodiscard]] const Judoka* FindJudokaByName(const std::string& Name) const;

		[[nodiscard]] Judoka* FindJudoka_ExactMatch(const JudokaData& NewJudoka);
		[[nodiscard]] Judoka* FindJudoka_SameName(const JudokaData& NewJudoka);

		bool DeleteJudoka(const UUID& UUID);

		//Clubs
		[[nodiscard]]
		auto& GetAllClubs() const { return m_Clubs; }
		std::shared_ptr<Club> AddClub(const MD5::Club& NewClub);
		bool AddClub(std::shared_ptr<Club> NewClub);

		[[nodiscard]]
		std::shared_ptr<Club> FindClub(const UUID& UUID);
		[[nodiscard]]
		std::shared_ptr<const Club> FindClub(const UUID& UUID) const;
		[[nodiscard]]
		std::shared_ptr<Club> FindClubByName(const std::string& Name);
		[[nodiscard]]
		std::shared_ptr<const Club> FindClubByName(const std::string& Name) const;

		bool DeleteClub(const UUID& UUID);

		//Associations
		[[nodiscard]]
		auto& GetAllAssociations() const { return m_Associations; }
		bool AddAssociation(std::shared_ptr<Association> NewAssociation);
		[[nodiscard]]
		std::shared_ptr<Association> FindAssociation(const UUID& UUID);
		[[nodiscard]]
		std::shared_ptr<const Association> FindAssociation(const UUID& UUID) const;
		[[nodiscard]]
		std::shared_ptr<const Association> FindAssociationByName(const std::string& Name) const;
		bool DeleteAssociation(const UUID& UUID);
		bool AssociationHasChildren(std::shared_ptr<const Association> Association) const;

		//Rule sets
		bool AddRuleSet(std::shared_ptr<RuleSet> NewRuleSet);
		[[nodiscard]]

		[[nodiscard]]
		std::shared_ptr<RuleSet> FindRuleSetByName(const std::string& RuleSetName);
		[[nodiscard]]
		std::shared_ptr<const RuleSet> FindRuleSetByName(const std::string& RuleSetName) const;
		[[nodiscard]]
		std::shared_ptr<RuleSet> FindRuleSet(const UUID& UUID);
		[[nodiscard]]
		std::shared_ptr<const RuleSet> FindRuleSet(const UUID& UUID) const;

		std::vector<std::shared_ptr<RuleSet>>& GetRuleSets() { return m_RuleSets; }
		[[nodiscard]]
		const std::vector<std::shared_ptr<RuleSet>>& GetRuleSets() const { return m_RuleSets; }

		[[nodiscard]]
		bool DeleteRuleSet(const UUID& UUID);

		//Age groups
		bool AddAgeGroup(std::shared_ptr<AgeGroup> NewAgeGroup);
		[[nodiscard]]
		std::shared_ptr<AgeGroup> FindAgeGroupByName(const std::string& AgeGroupName);
		[[nodiscard]]
		std::shared_ptr<const AgeGroup> FindAgeGroupByName(const std::string& AgeGroupName) const;
		[[nodiscard]]
		std::shared_ptr<AgeGroup> FindAgeGroup(const UUID& UUID);
		[[nodiscard]]
		std::shared_ptr<const AgeGroup> FindAgeGroup(const UUID& UUID) const;

		bool RemoveAgeGroup(const UUID& UUID);
		[[nodiscard]]
		std::vector<std::shared_ptr<AgeGroup>>& GetAgeGroups() { return m_AgeGroups; }
		[[nodiscard]]
		const std::vector<std::shared_ptr<AgeGroup>>& GetAgeGroups() const { return m_AgeGroups; }

	protected:
		std::vector<Judoka*> m_Judokas;

		std::vector<std::shared_ptr<Club>> m_Clubs;
		std::vector<std::shared_ptr<Association>> m_Associations;

		std::vector<std::shared_ptr<RuleSet>>  m_RuleSets;
		std::vector<std::shared_ptr<AgeGroup>> m_AgeGroups;

	private:
		uint32_t m_Year = 0;//Relative year for calculating the age groups. Set to 0 to use the current year
	};
}