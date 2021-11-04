#pragma once
#include <string>
#include <unordered_map>
#include "judoka.h"
#include "club.h"
#include "rule_set.h"



namespace Judoboard
{
	class StandingData
	{
	public:
		StandingData() = default;
		void Reset();

		void operator << (ZED::CSV& Stream);
		void operator >> (ZED::CSV& Stream) const;

		//Judokas
		uint32_t AddJudoka(Judoka&& NewJudoka) { return AddJudoka(new Judoka(NewJudoka)); }//Adds a judoka to the database
		bool AddJudoka(Judoka* NewJudoka);//Adds a judoka to the database by reference
		const std::unordered_map<uint32_t, Judoka*>& GetAllJudokas() const { return m_Judokas; }
		std::unordered_map<uint32_t, Judoka*>& GetAllJudokas() { return m_Judokas; }
		size_t GetNumJudoka() const { return m_Judokas.size(); }

		Judoka* FindJudoka(uint32_t ID);
		const Judoka* FindJudoka(uint32_t ID) const;
		Judoka* FindJudoka(const UUID& UUID);
		const Judoka* FindJudoka(const UUID& UUID) const;

		bool DeleteJudoka(uint32_t ID);

		//Clubs
		auto& GetAllClubs() const { return m_Clubs; }
		bool AddClub(Club* NewClub);

		Club* FindClub(uint32_t ID);
		const Club* FindClub(uint32_t ID) const;
		Club* FindClub(const UUID& UUID);
		const Club* FindClub(const UUID& UUID) const;

		bool DeleteClub(uint32_t ID);

		//Rule sets
		RuleSet* FindRuleSet(const std::string& RuleSetName);
		const RuleSet* FindRuleSet(const std::string& RuleSetName) const;
		RuleSet* FindRuleSet(const UUID& UUID);
		const RuleSet* FindRuleSet(const UUID& UUID) const;
		RuleSet* FindRuleSet(uint32_t ID);
		const RuleSet* FindRuleSet(uint32_t ID) const;

		bool  AddRuleSet(RuleSet* NewRuleSet);
		std::vector<RuleSet*>& GetRuleSets() { return m_RuleSets; }
		const std::vector<RuleSet*>& GetRuleSets() const { return m_RuleSets; }

		//Serialization
		const std::string JudokaToJSON() const;

	protected:
		std::unordered_map<uint32_t, Judoka*> m_Judokas;

		std::vector<Club*> m_Clubs;

		std::vector<RuleSet*> m_RuleSets;
	};
}