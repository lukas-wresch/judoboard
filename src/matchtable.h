#pragma once
#include <string>
#include <vector>
#include "judoka.h"



namespace Judoboard
{
	class ITournament;
	class Tournament;
	class Match;
	class RuleSet;
	class AgeGroup;
	class Judoka;



	class Color
	{
	public:
		enum class Name : uint32_t
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
				return "#fff8";
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



	class MatchTable : public ID
	{
		friend class Tournament;

	public:
		enum class Type
		{
			Unknown, Weightclass, Pause, Custom, SingleElimination
		};

		struct Result
		{
			Result() = default;
			//Result(Result&) = delete;
			//Result(const Result&) = delete;

			Result(const Judoka* Judoka_, const MatchTable* MatchTable_)
			{
				Judoka = Judoka_;
				MatchTable = MatchTable_;
				Wins  = 0;
				Score = 0;
				Time  = 0;
				NotSortable = false;
			}

			void Set(const Judoka* Judoka_, const MatchTable* MatchTable_)
			{
				Judoka = Judoka_;
				MatchTable = MatchTable_;
				Wins  = 0;
				Score = 0;
				Time  = 0;
				NotSortable = false;
			}

			void Set(const Judoka* Judoka_, const MatchTable* MatchTable_, int Wins_, int Score_, uint32_t Time_)
			{
				Judoka = Judoka_;
				MatchTable = MatchTable_;
				Wins  = Wins_;
				Score = Score_;
				Time  = Time_;
				NotSortable = false;
			}

			bool operator < (const Result& rhs) const;

			const Judoka* Judoka = nullptr;
			const MatchTable* MatchTable = nullptr;

			unsigned int Wins = 0;
			unsigned int Score = 0;
			uint32_t Time = 0;

			mutable bool NotSortable = false;//Flag to indicate if the element could not be sorted
		};


		MatchTable(const ITournament* Tournament) : m_Tournament(Tournament) {}
		MatchTable(const YAML::Node& Yaml, ITournament* Tournament);
		MatchTable(MatchTable&) = delete;
		MatchTable(const MatchTable&) = delete;

		~MatchTable();

		bool IsIncluded(const Judoka& Fighter) const;
		size_t GetNumberOfMatches() const { return m_Schedule.size(); }

		virtual Type GetType() const { return Type::Unknown; }
		virtual bool IsEditable() const { return true; }

		virtual std::string GetDescription() const = 0;
		virtual std::vector<Result> CalculateResults() const = 0;

		virtual bool AddMatch(Match* NewMatch);//Add a match manually to the match table. Use only for manual cases

		virtual const std::vector<Match*> GetSchedule() const { return m_Schedule; }
		virtual uint32_t GetRecommendedNumMatchesBeforeBreak() const { return m_RecommendedNumMatches_Before_Break; }

		virtual bool IsElgiable(const Judoka& Fighter) const = 0;
		virtual void GenerateSchedule() = 0;
		virtual const std::string ToHTML() const = 0;

		virtual size_t GetStartingPosition(const Judoka* Judoka) const { return -1; }
		virtual void   SetStartingPosition(const Judoka* Judoka, size_t NewStartingPosition) {}

		virtual bool AddParticipant(Judoka* NewParticipant, bool Force = false);
		virtual void RemoveAllParticipants() {
			m_Participants.clear();
			m_Schedule.clear();
		}

		//Basics
		const Match* GetMatch(size_t Index) const { if (Index >= m_Schedule.size()) return nullptr; return m_Schedule[Index]; }

		const std::vector<const Match*> FindMatches(const Judoka& Fighter1, const Judoka& Fighter2) const;//Returns all matches where Fighter1 and Fighter2 fight against each other

		bool Contains(const Judoka* Judoka) const;

		std::string GetName() const { return m_Name; }
		void SetName(const std::string& Name) { m_Name = Name; }

		virtual uint32_t GetMatID() const { return m_MatID; }
		virtual void SetMatID(int32_t MatID);

		virtual Status GetStatus() const = 0;

		virtual Color GetColor() const { return m_Color; }
		void  SetColor(Color NewColor) { m_Color = NewColor; }

		bool HasConcluded() const;

		Match*  FindMatch(const UUID& UUID) const;
		size_t  FindMatchIndex(const UUID& UUID) const;
		Judoka* FindParticipant(const UUID& UUID) const;

		//Rule sets
		const RuleSet& GetRuleSet() const;
		void SetRuleSet(const RuleSet* NewRuleSet) { m_Rules = NewRuleSet; }
		const RuleSet* GetOwnRuleSet() const { return m_Rules; }

		//Age groups
		const AgeGroup* GetAgeGroup() const { return m_pAgeGroup;}
		void SetAgeGroup(const AgeGroup* NewAgeGroup) { m_pAgeGroup = NewAgeGroup; }

		const std::vector<Judoka*>& GetParticipants() const { return m_Participants; }

		//Scheduler
		int32_t GetScheduleIndex() const { return m_ScheduleIndex; }
		void SetScheduleIndex(int32_t ScheduleIndex) { m_ScheduleIndex = ScheduleIndex; }

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const;
		virtual void ToString(YAML::Emitter& Yaml) const;

	protected:
		Match* AddAutoMatch(size_t WhiteIndex, size_t BlueIndex);
		Match* CreateAutoMatch(const Judoka* White, const Judoka* Blue);
		Match* AddMatchForWinners(Match* Match1, Match* Match2);

		std::pair<size_t, size_t> GetParticipantIndicesOfMatch(const Match* Match) const;

		size_t GetIndexOfParticipant(const Judoka* Participant) const;

		Judoka* GetParticipant(size_t Index) { if (Index >= m_Participants.size()) return nullptr; return m_Participants[Index]; }
		const Judoka* GetParticipant(size_t Index) const { if (Index >= m_Participants.size()) return nullptr; return m_Participants[Index]; }

		const ITournament* GetTournament() const { return m_Tournament; }

		void SortParticipantsByStartingPosition() {
			std::sort(m_Participants.begin(), m_Participants.end(), [this](const Judoka* a, const Judoka* b) {
				return GetStartingPosition(a) < GetStartingPosition(b);
			});
		}

		void DeleteSchedule() { m_Schedule.clear(); }

		virtual const std::string ResultsToHTML() const;


		std::vector<Match*> m_Schedule;//Set when GenerateSchedule() is called
		uint32_t m_RecommendedNumMatches_Before_Break = 1;//Set when GenerateSchedule() is called

	private:
		std::vector<Judoka*>& SetParticipants() { return m_Participants; }
		std::vector<Match*>&  SetSchedule()     { return m_Schedule; }
		
		void SetTournament(const ITournament* Tournament) { m_Tournament = Tournament; }

		std::vector<Judoka*> m_Participants;//List of all participants that are in the match table
		const RuleSet* m_Rules = nullptr;//Custom rule set for the matches (if available)

		const AgeGroup* m_pAgeGroup = nullptr;//Age group for the matches (if available)

		std::string m_Name;

		const ITournament* m_Tournament = nullptr;

		int32_t m_ScheduleIndex = -1;//Index when this entry should be in the schedule
		uint32_t m_MatID = 0;
		Color m_Color;
	};
}