#pragma once
#include <ZED/include/csv.h>
#include "id.h"



namespace Judoboard
{
	class Match;
	class Tournament;



	enum class Status
	{
		Scheduled,//Match is scheduled
		Running,//Match is currently underway
		Concluded//Match has concluded, results are available
	};



	class Color
	{
	public:
		enum class Name : unsigned int
		{
			White, Blue, Red, Green, Yellow, Magenta, Brown, Turquoise, Salmon, Purple, Khaki,
			Max
		};


		Color() = default;
		Color(Name Name_) : m_Name(Name_) {}
		Color(int Index) : m_Name((Name)Index) {}

		operator Name () const { return m_Name; }
		operator unsigned int() const { return (unsigned int)m_Name; }

		Color operator + (int v) const
		{
			int self = (int)m_Name;
			return (Color)(Name)(self + v);
		}

		void operator ++ ()
		{
			int temp = (int)m_Name + 1;
			temp %= (int)Name::Max;
			m_Name = (Name)temp;
		}

		void operator ++ (int)
		{
			int temp = (int)m_Name + 1;
			temp %= (int)Name::Max;
			m_Name = (Name)temp;
		}

		void operator << (ZED::CSV& Stream) const { Stream >> m_Name; }
		void operator >> (ZED::CSV& Stream) const { Stream << m_Name; }

		bool operator >  (const Color& rhs) const { return (int)m_Name >  (int)rhs.m_Name; }
		bool operator >= (const Color& rhs) const { return (int)m_Name >= (int)rhs.m_Name; }
		bool operator == (const Color& rhs) const { return (int)m_Name == (int)rhs.m_Name; }
		bool operator != (const Color& rhs) const { return (int)m_Name != (int)rhs.m_Name; }

		bool operator == (const Name& rhs) const { return (int)m_Name == (int)rhs; }

		const std::string ToString()
		{
			switch (m_Name)
			{
			case Name::White:
				return "White";
			case Name::Blue:
				return "Blue";
			case Name::Red:
				return "Red";
			case Name::Green:
				return "Green";
			case Name::Yellow:
				return "Yellow";
			case Name::Magenta:
				return "Magenta";
			case Name::Brown:
				return "Brown";
			case Name::Turquoise:
				return "Turquoise";
			case Name::Salmon:
				return "Salmon";
			case Name::Purple:
				return "Purple";
			case Name::Khaki:
				return "Khaki";
			case Name::Max:
				return "???";
			}

			return "White";
		}

		const std::string ToHexString()
		{
			switch (m_Name)
			{
			case Name::White:
			case Name::Max:
				return "#fff";
			case Name::Blue:
				return "#0bf8";
			case Name::Red:
				return "#e338";
			case Name::Green:
				return "#5d58";
			case Name::Yellow:
				return "#ff68";
			case Name::Magenta:
				return "#d0d8";
			case Name::Brown:
				return "#8658";
			case Name::Turquoise:
				return "#4ed8";
			case Name::Salmon:
				return "#f858";
			case Name::Purple:
				return "#a0a8";
			case Name::Khaki:
				return "#fe88";
			}

			return "#fff";
		}

	private:
		Name m_Name = Name::White;
	};



	class Schedulable : public ID
	{
		friend class Tournament;

	public:
		Schedulable(const Tournament* Tournament) : m_Tournament(Tournament) {}
		Schedulable(ZED::CSV& Stream, const Tournament* Tournament) : m_Tournament(Tournament)
		{
			Stream >> m_ScheduleIndex >> m_MatID;
			m_Color << Stream;
		}

		virtual const std::vector<Match*> GetSchedule() = 0;
		virtual uint32_t GetRecommendedNumMatchesBeforeBreak() const { return 1; }
		virtual const std::string GetName() const = 0;

		virtual uint32_t GetMatID() const { return m_MatID; }
		virtual void SetMatID(int32_t MatID) { m_MatID = MatID; }

		virtual bool IsEditable() const { return false; }
		virtual Status GetStatus() const = 0;

		virtual Color GetColor() const { return m_Color; }
		void  SetColor(Color NewColor) { m_Color = NewColor; }

	protected:
		int32_t GetScheduleIndex() const { return m_ScheduleIndex; }
		const Tournament* GetTournament() const { return m_Tournament; }

		void operator >> (ZED::CSV& Stream) const {
			Stream << m_ScheduleIndex << m_MatID;
			m_Color >> Stream;
		}

	private:
		void SetScheduleIndex(int32_t ScheduleIndex) { m_ScheduleIndex = ScheduleIndex; }

		const Tournament* m_Tournament = nullptr;

		int32_t m_ScheduleIndex = -1;//Index when this entry should be in the schedule
		uint32_t m_MatID = 0;
		Color m_Color;
	};
}