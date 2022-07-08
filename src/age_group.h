#pragma once
#include "id.h"
#include "rule_set.h"
#include "judoka.h"


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
		AgeGroup(const std::string& Name, uint32_t MinAge, uint32_t MaxAge, const RuleSet* Rules, const StandingData& StandingData);
		AgeGroup(const YAML::Node& Yaml, const StandingData& StandingData);

		std::string GetName() const { return m_Name; }
		const RuleSet* GetRuleSet() const { return m_pRules; }

		void SetMinAge(uint32_t Age) { m_MinAge = Age; }
		void SetMaxAge(uint32_t Age) { m_MaxAge = Age; }

		bool IsElgiable(const Judoka& Fighter) const;

		void operator >> (YAML::Emitter& Yaml) const;

	private:
		std::string m_Name;

		uint32_t m_MinAge = 0;//Age by birthyear
		uint32_t m_MaxAge = 100;

		const RuleSet* m_pRules = nullptr;

		const StandingData& m_StandingData;
	};
}