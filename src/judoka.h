#pragma once
#include <string>
#include "../ZED/include/csv.h"
#include "timer.h"
#include "club.h"
#include "judoboard.h"
#include "dm4.h"



namespace Judoboard
{
	class StandingData;



	class Judoka : public ID
	{
		friend class StandingData;
		friend class Database;
		friend class Mat;
		friend class Match;

	public:
		Judoka(const std::string& Firstname, const std::string& Lastname, uint16_t Weight = 0, Gender Gender = Gender::Male);
		Judoka(ZED::CSV& Stream, const StandingData* pStandingData = nullptr);//Load judoka from file
		Judoka(const DM4::Participant& Participant, const StandingData* pStandingData = nullptr);//Load judoka from DM4 data

		Gender GetGender()   const { return m_Gender; }
		uint16_t GetWeight() const { return m_Weight; }
		uint16_t GetAge() const;
		const std::string GetName() const { return m_Firstname + " " + m_Lastname; }

		//[[deprecated]]
		//const std::string GetShortName() const;
		//const std::string ToHexString() const;

		const std::string GetFirstname() const { return m_Firstname; }
		const std::string GetLastname()  const { return m_Lastname;  }
		auto GetBirthyear() const { return m_Birthyear; }
		const Club* GetClub() const { return m_pClub; }

		void SetFirstname(const std::string& Firstname) { m_Firstname = Firstname; }
		void SetLastname( const std::string& Lastname)  { m_Lastname  = Lastname; }
		void SetGender(Gender NewGender) { m_Gender = NewGender; }
		void SetWeight(uint16_t NewWight);
		void SetBirthyear(uint16_t NewBirthyear) { m_Birthyear = NewBirthyear; }
		void SetClub(const Club* NewClub) { m_pClub = NewClub; }

		void StartBreak() const { m_LastMatch_Timestamp = Timer::GetTimestamp(); }
		uint32_t GetLengthOfBreak()  const { return (Timer::GetTimestamp() - m_LastMatch_Timestamp) / 1000; }//Returns the number of seconds this judoka had a break

		const std::string ToString() const;

		void operator >> (ZED::CSV& Stream) const;

		bool operator ==(const Judoka& cmp) const { return GetID() == cmp.GetID(); }

	private:
		Judoka() = default;//Create empty judoka (for class Mat)

		std::string m_Firstname;
		std::string m_Lastname;

		uint16_t m_Weight = 0;
		Gender m_Gender = Gender::Male;

		uint16_t m_Birthyear = 0;

		const Club* m_pClub = nullptr;//Club the judoka belongs to

		mutable uint32_t m_LastMatch_Timestamp = 0;//Timestamp when the last match for this judoka ended to determine break between matches
	};
}