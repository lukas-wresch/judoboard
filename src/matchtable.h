#pragma once
#include <string>
#include <vector>
#include "judoboard.h"
#include "match.h"
#include "judoka.h"



namespace Judoboard
{
	class ITournament;
	class Tournament;
	class IFilter;
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
			Unknown, RoundRobin, Custom, SingleElimination, Pool
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

			unsigned int Wins  = 0;
			unsigned int Score = 0;
			uint32_t Time = 0;

			mutable bool NotSortable = false;//Flag to indicate if the element could not be sorted
		};



		class Results
		{
		public:
			Results(const MatchTable& Table) : m_Results(Table.GetParticipants().size()) {
				size_t i = 0;

				for (auto judoka : Table.GetParticipants())
				{
					m_Results[i].Set(judoka, &Table);
					++i;
				}
			}

			Results() = default;

			Results(size_t Count) : m_Results(Count) {
				for (size_t i = 0; i < Count; ++i)
					m_Results[i].Set(nullptr, nullptr);
			}


			void Add(const Judoka* Judoka, const MatchTable* Table) {
				m_Results.emplace_back(Judoka, Table);
			}

			Result* GetResultsOf(const Judoka* Judoka) {
				for (size_t i = 0; i < m_Results.size(); ++i)
					if (m_Results[i].Judoka && *m_Results[i].Judoka == *Judoka)
						return &m_Results[i];
				return nullptr;
			}

			void Sort() const
			{
				std::sort(m_Results.begin(), m_Results.end());
			}

			size_t GetSize() const { return m_Results.size(); }

			Result& operator [] (size_t Index) {
				assert(Index < m_Results.size());
				if (Index < m_Results.size())
					return m_Results[Index];
				return m_Results[0];
			}


			class Iterator {
			public:
				Iterator(Result* Data) : m_Data(Data) {}
				bool operator != (Iterator rhs) { return m_Data != rhs.m_Data; }
				Result& operator*() { return *m_Data; }
				void operator++() { ++m_Data; }

			private:
				Result* m_Data;
			};

			Iterator begin() { return Iterator(&m_Results[0]); }
			Iterator end()   { return Iterator(&m_Results[0] + GetSize()); }

		private:
			mutable std::vector<Result> m_Results;
		};


		MatchTable() = delete;
		MatchTable(MatchTable&) = delete;
		MatchTable(const MatchTable&) = delete;

		~MatchTable();

		virtual Type GetType() const { return Type::Unknown; }
		virtual bool IsEditable() const { return true; }

		virtual uint32_t GetMatID() const { return m_MatID; }
		virtual void SetMatID(int32_t MatID);

		virtual Status GetStatus() const;

		virtual Results CalculateResults() const = 0;

		virtual bool AddMatch(Match* NewMatch);//Add a match manually to the match table. Use only for manual cases

		virtual const std::vector<Match*> GetSchedule() const { return m_Schedule; }
		virtual uint32_t GetRecommendedNumMatchesBeforeBreak() const { return m_RecommendedNumMatches_Before_Break; }

		virtual const std::string ToHTML() const = 0;

		virtual bool AddParticipant(const Judoka* NewParticipant, bool Force = false);
		virtual bool RemoveParticipant(const Judoka* Participant);

		//Basics
		const Match* GetMatch(size_t Index) const { if (Index >= m_Schedule.size()) return nullptr; return m_Schedule[Index]; }

		virtual std::string GetDescription() const;

		const std::vector<const Match*> FindMatches(const Judoka& Fighter1, const Judoka& Fighter2) const;//Returns all matches where Fighter1 and Fighter2 fight against each other

		bool IsIncluded(const Judoka& Fighter) const;
		size_t GetNumberOfMatches() const { return m_Schedule.size(); }

		std::string GetName() const { return m_Name; }
		void SetName(const std::string& Name) { m_Name = Name; }

		virtual Color GetColor() const { return m_Color; }
		void  SetColor(Color NewColor) { m_Color = NewColor; }

		bool HasConcluded() const;

		Match*  FindMatch(const UUID& UUID) const;
		size_t  FindMatchIndex(const UUID& UUID) const;
		const Judoka* FindParticipant(const UUID& UUID) const;

		bool IsElgiable(const Judoka& Fighter) const;

		const std::vector<const Judoka*> GetParticipants() const;

		const IFilter* GetFilter() const { return m_Filter; }
		IFilter* GetFilter() { return m_Filter; }
		const ITournament* GetTournament() const { return m_Tournament; }

		//Rule sets
		const RuleSet& GetRuleSet() const;
		void SetRuleSet(const RuleSet* NewRuleSet) { m_Rules = NewRuleSet; }
		const RuleSet* GetOwnRuleSet() const { return m_Rules; }

		//Age groups
		const AgeGroup* GetAgeGroup() const;
		void SetAgeGroup(const AgeGroup* NewAgeGroup);

		//Scheduler
		int32_t GetScheduleIndex() const { return m_ScheduleIndex; }
		void SetScheduleIndex(int32_t ScheduleIndex) { m_ScheduleIndex = ScheduleIndex; }

		//Start positions
		virtual size_t GetStartPosition(const Judoka* Judoka) const;
		virtual const Judoka* GetJudokaByStartPosition(size_t StartPosition) const;
		virtual void SetStartPosition(const Judoka* Judoka, size_t NewStartPosition);
		virtual size_t GetMaxStartPositions() const = 0;

		//Best of three
		bool IsBestOfThree() const { return m_BestOfThree; }
		void IsBestOfThree(bool Enable) { m_BestOfThree = Enable; GenerateSchedule(); }

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const;
		virtual void ToString(YAML::Emitter& Yaml) const;

	protected:
		MatchTable(IFilter* Filter, const ITournament* Tournament) : m_Filter(Filter), m_Tournament(Tournament) {}
		MatchTable(const YAML::Node& Yaml, const ITournament* Tournament);

		virtual void GenerateSchedule() = 0;

		static std::string GetHTMLForm();

		Match* AddAutoMatch(size_t WhiteStartPosition, size_t BlueStartPosition);
		Match* CreateAutoMatch(const DependentJudoka& White, const DependentJudoka& Blue);
		Match* AddMatchForWinners(Match* Match1, Match* Match2);

		void AddMatchesForBestOfThree();

		void DeleteSchedule() { m_Schedule.clear(); }

		void SetFilter(IFilter* NewFilter) { m_Filter = NewFilter; }
		void SetTournament(const ITournament* Tournament) { m_Tournament = Tournament; }

		const std::string ResultsToHTML() const;

		std::vector<Match*>&  SetSchedule() { return m_Schedule; }


		std::vector<Match*> m_Schedule;//Set when GenerateSchedule() is called
		uint32_t m_RecommendedNumMatches_Before_Break = 1;//Set when GenerateSchedule() is called

	private:
		const RuleSet* m_Rules = nullptr;//Custom rule set for the matches (if available)

		std::string m_Name;

		IFilter* m_Filter = nullptr;

		const ITournament* m_Tournament = nullptr;

		int32_t m_ScheduleIndex = -1;//Index when this entry should be in the schedule
		uint32_t m_MatID = 0;
		Color m_Color;

		bool m_BestOfThree = false;
	};
}