#pragma once
#include <string>
#include <vector>
#include "judoka.h"
#include "schedule_entry.h"
#include "../ZED/include/csv.h"



namespace Judoboard
{
	class ITournament;
	class Tournament;
	class Match;
	class RuleSet;



	class MatchTable : public Schedulable
	{
		friend class Tournament;
		friend class Application;

	public:
		enum class Type
		{
			Unknown, Weightclass, Pause
		};

		MatchTable(const ITournament* Tournament) : Schedulable(Tournament) {}
		MatchTable(ZED::CSV& Stream, const ITournament* Tournament);
		MatchTable(MatchTable&) = delete;
		MatchTable(const MatchTable&) = delete;

		~MatchTable();

		bool IsIncluded(const Judoka& Fighter) const;
		size_t GetNumberOfMatches() const { return m_Schedule.size(); }

		virtual Type GetType() const { return Type::Unknown; }
		virtual bool IsEditable() const override { return true; }

		virtual bool AddMatch(Match* NewMatch);//Add a match manually to the match table. Use only for manual cases

		virtual const std::vector<Match*> GetSchedule() override { return m_Schedule; }
		virtual uint32_t GetRecommendedNumMatchesBeforeBreak() const override { return m_RecommendedNumMatches_Before_Break; }

		virtual bool IsElgiable(const Judoka& Fighter) const = 0;
		virtual void GenerateSchedule() = 0;
		virtual const std::string ToHTML() const = 0;

		virtual void SetMatID(int32_t MatID) override;

		const Match* GetMatch(size_t Index) const { if (Index >= m_Schedule.size()) return nullptr; return m_Schedule[Index]; }

		bool AddParticipant(Judoka* NewParticipant, bool Force = false);
		const std::vector<const Match*> FindMatches(const Judoka& Fighter1, const Judoka& Fighter2) const;//Returns all matches where Fighter1 and Fighter2 fight against each other

		bool Contains(const Judoka* Judoka) const;

		const RuleSet& GetRuleSet() const;
		void SetRuleSet(const RuleSet* NewRuleSet) { m_Rules = NewRuleSet; }

		const std::string GetName() const override { return m_Name; }
		void SetName(const std::string& Name)
		{
			m_Name = Name;
			ReplaceAll(m_Name, ",", "&#44;");//Escape all ','
		}

		//Serialization
		virtual const std::string ToString() const;

		virtual void operator >> (ZED::CSV& Stream) const;

	protected:
		struct FighterScore
		{
			FighterScore() = default;
			FighterScore(FighterScore&) = delete;
			FighterScore(const FighterScore&) = delete;

			void Set(const Judoka* Judoka_, const MatchTable* MatchTable_)
			{
				Judoka = Judoka_;
				MatchTable = MatchTable_;
				Wins = 0;
				Score = 0;
				Time = 0;
				NotSortable = false;
			}

			void Set(const Judoka* Judoka_, const MatchTable* MatchTable_, int Wins_, int Score_, uint32_t Time_)
			{
				Judoka = Judoka_;
				MatchTable = MatchTable_;
				Wins = Wins_;
				Score = Score_;
				Time = Time_;
				NotSortable = false;
			}

			const Judoka* Judoka = nullptr;
			const MatchTable* MatchTable = nullptr;

			unsigned int Wins = 0;
			unsigned int Score = 0;
			uint32_t Time = 0;

			mutable bool NotSortable = false;//Flag to indicate if the element could not be sorted
		};


		const std::vector<Judoka*>& GetParticipants() const { return m_Participants; }
		Judoka* GetParticipant(size_t Index) { if (Index >= m_Participants.size()) return nullptr; return m_Participants[Index]; }
		const Judoka* GetParticipant(size_t Index) const { if (Index >= m_Participants.size()) return nullptr; return m_Participants[Index]; }

		static int CompareFighterScore(const void* a, const void* b);

		std::vector<Match*> m_Schedule;//Set when GenerateSchedule() is called
		std::vector<Match*> m_ManualMatches;//Manually added matches
		uint32_t m_RecommendedNumMatches_Before_Break = 1;//Set when GenerateSchedule() is called

	private:
		std::vector<Judoka*>& SetParticipants() { return m_Participants; }
		std::vector<Match*>& SetSchedule() { return m_Schedule; }

		std::vector<Judoka*> m_Participants;
		const RuleSet* m_Rules = nullptr;//Custom rule set for the match (if available)

		std::string m_Name;
	};
}