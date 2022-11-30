#pragma once
#include "matchlog.h"
#include "matchtable.h"



namespace Judoboard
{
	class ITournament;
	class Tournament;
	class RuleSet;
	class Judoka;



	class Match : public ID
	{
		friend class Application;
		friend class Tournament;
		friend class Mat;
		friend class MatchTable;

	public:
		enum class Score
		{
			Ippon = 10, Wazaari = 7, Yuko = 5, Koka = 3, Hantei = 1, Draw = 0
		};

		enum class DependencyType
		{
			None        = 0,
			BestOfThree = 1,//Match is the last in the series of a best of three series
			TakeWinner  = 2,//Take the winner of the match this match depends upon
			TakeLoser   = 3,//Take the loser of the match this match depends upon

			TakeRank1  = 11,//Take the 1st of a match table
			TakeRank2  = 12,//Take the 2nd of a match table
			TakeRank3  = 13,//Take the 3rd of a match table
			TakeRank4  = 14,//Take the 4th of a match table
			TakeRank5  = 15,//Take the 5th of a match table
			TakeRank6  = 16,//Take the 6th of a match table
			TakeRank7  = 17,//Take the 7th of a match table
			TakeRank8  = 18,//Take the 8th of a match table
			TakeRank9  = 19,//Take the 9th of a match table
			TakeRank10 = 20,//Take the 10th of a match table
		};

		struct Result
		{
			Result() = default;
			Result(Winner Winner, Score Score, uint32_t Time = 0)
			{
				m_Winner = Winner;
				m_Score  = Score;
				m_Time   = Time;
			}
			Result(Fighter Winner, Score Score, uint32_t Time = 0)
			{
				m_Winner = Fighter2Winner(Winner);
				m_Score  = Score;
				m_Time   = Time;
			}

			Winner m_Winner = Winner::Draw;
			Score  m_Score  = Score::Draw;
			uint32_t m_Time = 0;//Match time in milliseconds
		};


		Match(const Judoka* White, const Judoka* Blue, const ITournament* Tournament, uint32_t MatID = 0);
		Match(const YAML::Node& Yaml, MatchTable* MatchTable, ITournament* Tournament);
		Match(const Match&) = default;//Deep copy


		void SetMatID(int32_t MatID) { if (m_State == Status::Scheduled) m_MatID = MatID; }
		Status GetStatus() const;

		bool IsScheduled()  const { return m_State == Status::Scheduled; }
		bool IsRunning()    const { return m_State == Status::Running; }
		bool HasConcluded() const;
		bool IsAssociatedWithMat() const { return GetMatID() > 0; }
		MatchLog& GetLog() { return m_Log; }
		const MatchLog& GetLog() const { return m_Log; }

		void StartMatch() { m_State = Status::Running; }
		void EndMatch();

		const MatchTable* GetMatchTable() const { return m_Table; }
		void SetMatchTable(MatchTable* MatchTable) { m_Table = MatchTable; }

		uint32_t GetMatID() const;

		//Judoka* GetFighter(Fighter Fighter);
		//const Judoka* GetFighter(Fighter Fighter) const { return (const_cast<Match*>(this))->GetFighter(Fighter); }
		[[nodiscard]] const Judoka* GetFighter(Fighter Fighter) const;
		void SetFighter(Fighter Fighter, const Judoka* NewFighter) {
			if (Fighter == Fighter::White)
				m_White.m_Judoka = NewFighter;
			else
				m_Blue.m_Judoka  = NewFighter;
		}
		void SwapFighters() { std::swap(m_White, m_Blue); }

		Result GetResult() const { return m_Result; }//Returns the result of the match (if it has concluded)

		bool Contains(const Judoka& Judoka) const;//Returns true if and only if Judoka is one of the two fighters
		Fighter GetColorOfFighter(const Judoka& Judoka) const;//Returns the color (white or blue) of the judoka. 'Judoka' must be on of the two fighters of the match
		const Judoka* GetEnemyOf(const Judoka& Judoka) const;//Returns the enemy of the given fighter. Judoka must be on of the two fighters of this match

		const Judoka* GetWinner() const;
		const Judoka* GetLoser() const;

		//Match dependencies
		void SetDependency(Fighter Fighter, DependencyType Type, Match* Reference);
		void SetDependency(Fighter Fighter, DependencyType Type, const MatchTable* Reference);
		void SetBestOfThree(Match* Reference1, Match* Reference2);
		bool HasUnresolvedDependency() const;
		bool HasDependentMatches() const {//Returns true if and only if this match depends upon other matches (as in the depend matches needs to conclude in order for this match to start)
			return m_White.m_DependentMatch || m_Blue.m_DependentMatch;
		}
		bool HasValidFighters() const { return GetFighter(Fighter::White) && GetFighter(Fighter::Blue); }//Returns true if and only if GetFighter() returns not a null pointer
		const Judoka* GetSingleValidFighters() const {
			if (GetFighter(Fighter::White) && !GetFighter(Fighter::Blue))
				return GetFighter(Fighter::White);
			else if (!GetFighter(Fighter::White) && GetFighter(Fighter::Blue))
				return GetFighter(Fighter::Blue);
			return nullptr;
		}
		bool IsEmptyMatch() const;
		bool IsCompletelyEmptyMatch() const;
		const std::vector<const Match*> GetDependentMatches() const;//Returns a list of matches this match depends upon as in the depend matches need to conclude in order for this match to be scheduled
		auto GetDependencyTypeOf(Fighter Fighter) const {
			if (Fighter == Fighter::White)
				return m_White.m_Dependency;
			return m_Blue.m_Dependency;
		}
		const Match* GetDependentMatchOf(Fighter Fighter) const {
			if (Fighter == Fighter::White)
				return m_White.m_DependentMatch;
			return m_Blue.m_DependentMatch;
		}

		const RuleSet& GetRuleSet() const;
		void SetRuleSet(const RuleSet* NewRuleSet) { m_Rules = NewRuleSet; }

		uint32_t GetCurrentBreaktime() const;

		bool IsBestOfThree()   const { return m_White.m_Dependency == DependencyType::BestOfThree; }
		bool IsAutoGenerated() const { return m_IsAutoGenerated; }

		//Serialize
		void ToString(YAML::Emitter& Yaml) const;

		void operator >>(YAML::Emitter& Yaml) const;

		bool operator ==(const Match& rhs) const {
			return GetUUID() == rhs.GetUUID();
		}

	private:
		const ITournament* GetTournament() const { return m_Tournament; }

		void SetState(Status NewState) { m_State = NewState; }
		void SetResult(const Result& Result) { m_Result = Result; SetState(Status::Concluded); }

		void SetAutoGenerated(bool AutoGenerated = true) { m_IsAutoGenerated = AutoGenerated; }

		struct {
			const Judoka* m_Judoka = nullptr;
			Match* m_DependentMatch = nullptr;
			const MatchTable* m_DependentMatchTable = nullptr;
			DependencyType m_Dependency = DependencyType::None;
		} m_White, m_Blue;

		Status m_State = Status::Scheduled;

		Result m_Result;

		MatchLog m_Log;//Log of the match

		const MatchTable* m_Table = nullptr;//Match table this fight is associated with (if any)
		const RuleSet*    m_Rules = nullptr;//Custom rule set for the match (if available)

		const ITournament* m_Tournament = nullptr;

		uint32_t m_MatID = 0;
		bool m_IsAutoGenerated = false;//Has this matches been created by a match table automatically?
	};



	inline bool operator & (Match::DependencyType lhs, Match::DependencyType rhs) {
		return (int)lhs & (int)rhs;
	}
}