#pragma once
#include "judoka.h"
#include "matchlog.h"
#include "matchtable.h"
#include "schedule_entry.h"
#include <ZED/include/csv.h>



namespace Judoboard
{
	class Tournament;
	class RuleSet;



	class Match : public Schedulable
	{
		friend class Tournament;
		friend class Mat;
		friend class MatchTable;

	public:
		enum class Score
		{
			Ippon = 10, Wazaari = 7, Yuko = 5, Koka = 3, Hantei = 1, Draw = 0
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
				m_Score = Score;
				m_Time = Time;
			}

			Winner m_Winner = Winner::Draw;
			Score  m_Score = Score::Draw;
			uint32_t m_Time = 0;//Match time in milliseconds
		};


		Match(const Tournament* Tournament, Judoka* White, Judoka* Blue, uint32_t MatID = 0);
		Match(ZED::CSV& Stream, Tournament* Tournament);

		//Virtuals
		virtual const std::vector<Match*> GetSchedule() override { std::vector<Match*> schedule{ this }; return schedule; }
		virtual const std::string GetName() const override {
			if (!HasValidFighters())
				return "T.b.d. vs. T.b.d.";
			return GetFighter(Fighter::White)->GetName() + " vs. " + GetFighter(Fighter::Blue)->GetName();
		};

		virtual void SetMatID(int32_t MatID) override { if (m_State == Status::Scheduled) Schedulable::SetMatID(MatID); }
		virtual Status GetStatus() const override { return m_State; }

		virtual Color GetColor() const override {
			if (GetMatchTable())
				return GetMatchTable()->GetColor();
			return Schedulable::GetColor();
		}

		bool IsScheduled()  const { return m_State == Status::Scheduled; }
		bool IsRunning()    const { return m_State == Status::Running; }
		bool HasConcluded() const { return m_State == Status::Concluded; }
		bool IsAssociatedWithMat() const { return GetMatID() > 0; }
		MatchLog& GetLog() { return m_Log; }
		const MatchLog& GetLog() const { return m_Log; }

		void StartMatch() { m_State = Status::Running; }
		void EndMatch()
		{
			m_State = Status::Concluded;

			if (m_White.m_Judoka)
				m_White.m_Judoka->StartBreak();
			if (m_Blue.m_Judoka)
				m_Blue.m_Judoka->StartBreak();
		}

		const MatchTable* GetMatchTable() const { return m_Table; }
		void SetMatchTable(MatchTable* MatchTable) { m_Table = MatchTable; }

		uint32_t GetMatID() const override {
			if (Schedulable::GetMatID() <= 0 && m_Table)//No override specified and associated to a match table
				return m_Table->GetMatID();//Take the mat of the match table
			return Schedulable::GetMatID();
		}

		Judoka* GetFighter(Fighter Fighter);
		const Judoka* GetFighter(Fighter Fighter) const;
		Result GetMatchResult() const { return m_Result; }//Returns the result of the match (if it has concluded)

		bool Contains(const Judoka& Judoka) const;//Returns true if and only if Judoka is one of the two fighters

		const Judoka* GetWinningJudoka() const;
		bool HasDependentMatches() const { return m_White.m_PreviousMatch || m_Blue.m_PreviousMatch; }//Returns true if and only if this match depends upon as in the depend matches need to conclude in order for this match to be scheduled
		bool HasValidFighters() const { return m_White.m_Judoka && m_Blue.m_Judoka; }//Returns true if and only if GetFighter() returns not a null pointer
		const std::vector<Match*> GetDependentMatches();//Returns a list of matches this match depends upon as in the depend matches need to conclude in order for this match to be scheduled
		const std::vector<const Match*> GetDependentMatches() const;//Returns a list of matches this match depends upon as in the depend matches need to conclude in order for this match to be scheduled

		const RuleSet& GetRuleSet() const;
		void SetRuleSet(const RuleSet* NewRuleSet) { m_Rules = NewRuleSet; }

		//Serialize
		ZED::CSV ToString() const;
		ZED::CSV AllToString() const;

		void operator >>(ZED::CSV& Stream) const;

	private:
		void SetState(Status NewState) { m_State = NewState; }
		void SetResult(const Result& Result) { m_Result = Result; }

		struct {
			Judoka* m_Judoka = nullptr;
			Match* m_PreviousMatch = nullptr;
			bool m_PreviousWinner = true;
		} m_White, m_Blue;

		Status m_State = Status::Scheduled;

		Result m_Result;

		MatchLog m_Log;//Log of the match

		const MatchTable* m_Table = nullptr;//Match table this fight is associated with (if any)
		const RuleSet* m_Rules = nullptr;//Custom rule set for the match (if available)
	};
}