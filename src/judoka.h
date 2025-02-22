#pragma once
#include <string>
#include "timer.h"
#include "club.h"
#include "judoboard.h"
#include "dm4.h"
#include "md5.h"
#include "dmf.h"



namespace Judoboard
{
	class StandingData;
	class Judoka;
	class Match;
	class MatchTable;



	class DependentJudoka
	{
		friend class Match;

	public:
		DependentJudoka(const Judoka* Judoka) {
			m_Judoka = Judoka;
			m_Type   = DependencyType::None;
		}
		DependentJudoka(DependencyType DependencyType, Match& DependentMatch) {
			m_Type = DependencyType;
			m_DependentMatch = &DependentMatch;
		}
		DependentJudoka(DependencyType DependencyType, const MatchTable& DependentMatchTable) {
			m_Type = DependencyType;
			m_DependentMatchTable = &DependentMatchTable;
		}

		const Judoka* GetJudoka() const;
		auto GetDependency() const { return m_Type; }
		const Match* GetDependentMatch() const { return m_DependentMatch; }
		auto GetDependentMatchTable() const { return m_DependentMatchTable; }

		bool operator == (const Judoka* rhs) const;
		bool operator == (const DependentJudoka& rhs) const;

		operator bool () const {
			return m_Judoka || m_Type != DependencyType::None;
		}

	private:
		DependentJudoka() = default;

		const Judoka* m_Judoka  = nullptr;
		DependencyType m_Type   = DependencyType::None;
		Match* m_DependentMatch = nullptr;
		const MatchTable* m_DependentMatchTable = nullptr;
	};



	struct JudokaData
	{
		JudokaData(const Judoka& Judoka);
		JudokaData(const MD5::Participant& Judoka);
		JudokaData(const DM4::Participant& Judoka);
		JudokaData(const DMF::Participant& Judoka);

		std::string m_Firstname;
		std::string m_Lastname;

		Weight m_Weight = 0;//Weight in grams
		Gender m_Gender = Gender::Unknown;

		uint16_t m_Birthyear = 0;

		std::string m_ClubName;

		std::string m_Number;//Official number of this judoka according to his "judo passport"
	};



	class Judoka : public ID
	{
		friend class StandingData;
		friend class Database;
		friend class Mat;
		friend class Match;

	public:
		Judoka(const std::string& Firstname, const std::string& Lastname, Weight Weight = 0, Gender Gender = Gender::Male, uint32_t Birthyear = 0);
		Judoka(const YAML::Node& Yaml, const StandingData* pStandingData);//Load judoka from file
		Judoka(const JudokaData& JudokaData, const StandingData* pStandingData);
		[[deprecated]] Judoka(const DM4::Participant& Participant, const StandingData* pStandingData = nullptr);//Load judoka from DM4 data
		[[deprecated]] Judoka(const MD5::Participant& Participant, const StandingData* pStandingData = nullptr);//Load judoka from MD5 data
		[[deprecated]] Judoka(const DMF::Participant& Participant);//Load judoka from DMF data

		virtual bool IsValid() const { return true; }

		auto GetGender() const { return m_Gender; }
		auto GetWeight() const { return m_Weight; }
		uint16_t GetAge() const;
		const std::string GetName(NameStyle Style) const {
			switch (Style)
			{
			case NameStyle::GivenName:
				return m_Firstname + " " + m_Lastname;
			case NameStyle::FamilyName:
				return m_Lastname + ", " + m_Firstname;
			}
			return "";
		}

		const std::string GetFirstname() const { return m_Firstname; }
		const std::string GetLastname()  const { return m_Lastname;  }
		auto GetBirthyear() const { return m_Birthyear; }
		const Club* GetClub() const { return m_pClub; }
		auto GetNumber() const { return m_Number; }

		void SetFirstname(const std::string& Firstname) { m_Firstname = Firstname; }
		void SetLastname( const std::string& Lastname)  { m_Lastname  = Lastname; }
		void SetGender(Gender NewGender) { m_Gender = NewGender; }
		void SetWeight(Weight NewWeight);
		void SetBirthyear(uint16_t NewBirthyear) { m_Birthyear = NewBirthyear; }
		void SetClub(const Club* NewClub) { m_pClub = NewClub; }
		void SetNumber(const std::string& NewNumber) { m_Number = NewNumber; }

		void StartBreak(uint32_t BreakTime) const {//Start break for 'BreakTime' seconds
			m_LastMatch_Timestamp = Timer::GetTimestamp();
			m_RequiredBreakTime   = BreakTime;
		}
		//Returns the number of seconds this judoka had a break
		uint32_t GetLengthOfBreak()  const { return (Timer::GetTimestamp() - m_LastMatch_Timestamp) / 1000; }
		bool NeedsBreak()  const {
			return GetLengthOfBreak() < m_RequiredBreakTime;
		}
		//Skips the current break
		void SkipBreak() {
			assert(NeedsBreak());
			if (NeedsBreak())
				m_LastMatch_Timestamp = Timer::GetTimestamp() - m_RequiredBreakTime*1000;
			assert(!NeedsBreak());
		}

		void operator >> (YAML::Emitter& Yaml) const;
		void ToString(YAML::Emitter& Yaml) const;

		bool operator ==(const Judoka& rhs) const { return GetUUID() == rhs.GetUUID(); }
		bool operator ==(const Judoka* rhs) const {
			if (!rhs)
				return false;
			return GetUUID() == rhs->GetUUID();
		}
		bool operator ==(const UUID& rhs) const {
			return GetUUID() == rhs;
		}

	private:
		Judoka() = default;//Create empty judoka (for class Mat)

		std::string m_Firstname;
		std::string m_Lastname;

		Weight m_Weight = 0;//Weight in grams
		Gender m_Gender = Gender::Unknown;

		uint16_t m_Birthyear = 0;

		std::string m_Number;//Official number of this judoka according to his "judo passport"

		const Club* m_pClub = nullptr;//Club the judoka belongs to

		mutable uint32_t m_LastMatch_Timestamp = 0;//Timestamp when the last match for this judoka ended to determine break between matches
		mutable uint32_t m_RequiredBreakTime = 0;//Required break time for this judoka in seconds
	};
}