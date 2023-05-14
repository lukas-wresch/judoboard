#pragma once
#include "timer.h"
#include "id.h"
#include "../ZED/include/csv.h"


namespace YAML
{
	class Emitter;
	class Node;
}



namespace Judoboard
{
	class RuleSet : public ID
	{
		friend class StandingData;

	public:
		RuleSet() = default;
		RuleSet(const std::string& Name, uint32_t MatchTime, int GoldenScoreTime, uint32_t OsaeKomiTime, uint32_t OsaeKomiTime_with_Wazaari, bool Yuko = false, bool Koka = false, bool Draw = false, uint32_t BreakTime = 0);
		RuleSet(const YAML::Node& Yaml);

		const std::string& GetName() const { return m_Name; }
		bool IsYukoEnabled() const { return m_Yuko; }
		bool IsKokaEnabled() const { return m_Koka; }

		bool IsGoldenScoreEnabled() const { return m_GoldenScoreTime != 0; }
		bool IsDrawAllowed() const { return m_Draw; }

		int32_t GetOsaeKomiTime(bool HasWazaari) const
		{
			if (HasWazaari)
				return m_OsaeKomiTime_With_WazaAri;
			return m_OsaeKomiTime;
		}

		bool IsOutOfTime(int MatchTime_Millseconds, bool GoldenScore) const;

		int GetMatchTime() const { return m_MatchTime; }
		int GetGoldenScoreTime() const { return m_GoldenScoreTime; }
		uint32_t GetBreakTime() const { return m_BreakTime; }

		const std::string GetDescription() const;

		void operator >> (YAML::Emitter& Yaml) const;

	private:

		std::string m_Name = "Default";//User-friendly name of the rule set

		bool m_Yuko = false;//Is Yuko enabled?
		bool m_Koka = false;//Is Koka enabled?
		bool m_Draw = true;//Is a draw allowed?

		int32_t  m_MatchTime = 3 * 60;//Match time in seconds. Can be -1 for infinite matches
		int32_t  m_GoldenScoreTime = 3 * 60;//Match time when golden score is active. Can be -1 for infinite matches
		uint32_t m_OsaeKomiTime = 20;//Seconds to achieve an ippon during osaekomi
		uint32_t m_OsaeKomiTime_With_WazaAri = 10;//Seconds to achieve a wazaari during osaekomi

		uint32_t m_BreakTime = 0;//Break time in seconds between matches
	};
}