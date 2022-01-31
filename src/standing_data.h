#pragma once
#include <string>
#include <mutex>
#include <unordered_map>
#include "judoka.h"
#include "club.h"
#include "rule_set.h"
#include "dm4.h"



namespace Judoboard
{
	class StandingData
	{
	public:
		StandingData() = default;
		void Reset();

		void operator << (ZED::CSV& Stream);
		void operator >> (ZED::CSV& Stream) const;

		void AddMD5File(const MD5& File);

		//Judokas
		uint32_t AddJudoka(Judoka&& NewJudoka) { return AddJudoka(new Judoka(NewJudoka)); }//Adds a judoka to the database
		bool AddJudoka(Judoka* NewJudoka);//Adds a judoka to the database by reference
		const std::unordered_map<uint32_t, Judoka*>& GetAllJudokas() const { return m_Judokas; }
		std::unordered_map<uint32_t, Judoka*>& GetAllJudokas() { return m_Judokas; }
		size_t GetNumJudoka() const { return m_Judokas.size(); }
		size_t GetNumClubs()  const { return m_Clubs.size(); }

		Judoka* FindJudoka(uint32_t ID);
		const Judoka* FindJudoka(uint32_t ID) const;
		Judoka* FindJudoka(const UUID& UUID);
		const Judoka* FindJudoka(const UUID& UUID) const;

		Judoka* FindJudoka_DM4_ExactMatch(const DM4::Participant& NewJudoka);
		Judoka* FindJudoka_DM4_SameName(const DM4::Participant& NewJudoka);

		bool DeleteJudoka(uint32_t ID);

		//Clubs
		auto& GetAllClubs() const { return m_Clubs; }
		Club* AddClub(const MD5::Club& NewClub);
		bool  AddClub(Club* NewClub);

		Club* FindClub(uint32_t ID);
		const Club* FindClub(uint32_t ID) const;
		Club* FindClub(const UUID& UUID);
		const Club* FindClub(const UUID& UUID) const;
		Club* FindClubByName(const std::string& Name);
		const Club* FindClubByName(const std::string& Name) const;

		bool DeleteClub(uint32_t ID);

		//Rule sets
		RuleSet* FindRuleSetByName(const std::string& RuleSetName);
		const RuleSet* FindRuleSetByName(const std::string& RuleSetName) const;
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

		mutable  std::mutex m_Mutex;
	};
}