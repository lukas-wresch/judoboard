#pragma once
#include "id.h"
#include "rule_set.h"
#include "judoka.h"
#include "md5.h"


namespace YAML
{
	class Emitter;
	class Node;
}



namespace Judoboard
{
	class StandingData;

	class AgeGroup : public ID
	{
	public:
		AgeGroup(const std::string& Name, uint32_t MinAge, uint32_t MaxAge, const RuleSet* Rules, Gender Gender = Gender::Unknown);
		AgeGroup(const YAML::Node& Yaml, const StandingData& StandingData);
		AgeGroup(const MD5::AgeGroup& AgeGroup, const StandingData& StandingData);

		std::string GetName() const { return m_Name; }
		void SetName(std::string& NewName) { m_Name = NewName; }
		const RuleSet* GetRuleSet() const { return m_pRules; }
		void SetRuleSet(const RuleSet* NewRuleSet) { m_pRules = NewRuleSet; }

		int  GetMinAge() const { return m_MinAge; }
		int  GetMaxAge() const { return m_MaxAge; }
		void SetMinAge(uint32_t Age) { m_MinAge = Age; }
		void SetMaxAge(uint32_t Age) { m_MaxAge = Age; }
		auto GetGender() const { return m_Gender; }
		void SetGender(Gender NewGender) { m_Gender = NewGender; }

		bool IsElgiable(const Judoka& Fighter, const StandingData& StandingData) const;

		void operator >> (YAML::Emitter& Yaml) const;
		void ToString(YAML::Emitter& Yaml) const;

	private:
		std::string m_Name;

		uint32_t m_MinAge = 0;//Age by birthyear
		uint32_t m_MaxAge = 100;

		Gender m_Gender = Gender::Unknown;

		const RuleSet* m_pRules = nullptr;
	};
}