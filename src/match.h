#pragma once
#include "matchlog.h"
#include "judoboard.h"
#include "judoka.h"



namespace Judoboard
{
	class ITournament;
	class Tournament;
	class MatchTable;
	class RuleSet;
	class Judoka;



	class Match : public ID
	{
		friend class Application;
		friend class Tournament;
		friend class Mat;
		friend class MatchTable;
		friend class SingleElimination;

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
				m_Score = Score;
				m_Time = Time;
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


		struct Tag
		{
			static Tag Finals() {
				Tag ret;
				ret.finals = true;
				return ret;
			}

			static Tag Semi() {
				Tag ret;
				ret.semi = true;
				return ret;
			}

			static Tag Third() {
				Tag ret;
				ret.third = true;
				return ret;
			}

			static Tag Fifth() {
				Tag ret;
				ret.fifth = true;
				return ret;
			}

			bool IsNormal() const { return value == 0; }

			bool operator ==(const Tag& rhs) const {
				return value == rhs.value;
			}
			Tag operator &(const Tag& rhs) const {
				Tag ret;
				ret.value = this->value | rhs.value;
				return ret;
			}

			union
			{
				uint8_t value = 0;

				struct
				{
					bool finals : 1;
					bool semi : 1;
					bool third : 1;
					bool fifth : 1;
					bool reserved1 : 1;
					bool reserved2 : 1;
					bool reserved3 : 1;
					bool reserved4 : 1;
				};
			};
		};


		Match(const Judoka* White, const Judoka* Blue, const ITournament* Tournament, uint32_t MatID = 0);
		Match(const DependentJudoka& White, const DependentJudoka& Blue, const ITournament* Tournament, uint32_t MatID = 0);
		Match(const YAML::Node& Yaml, MatchTable* MatchTable, const ITournament* Tournament);
		Match(const Match&) = default;//Deep copy


		void SetMatID(int32_t MatID) { if (m_State == Status::Scheduled) m_MatID = MatID; }
		Status GetStatus() const;

		bool IsScheduled()  const { return GetStatus() == Status::Scheduled || GetStatus() == Status::Optional; }
		bool IsRunning()    const { return m_State == Status::Running; }
		bool HasConcluded() const;
		bool IsAssociatedWithMat() const { return GetMatID() > 0; }
		MatchLog& GetLog() { return m_Log; }
		const MatchLog& GetLog() const { return m_Log; }

		void StartMatch();
		void EndMatch();

		const MatchTable* GetMatchTable() const { return m_Table; }
		void  SetMatchTable(const MatchTable* MatchTable) { m_Table = MatchTable; }

		uint32_t GetMatID() const;

		//Judoka* GetFighter(Fighter Fighter);
		//const Judoka* GetFighter(Fighter Fighter) const { return (const_cast<Match*>(this))->GetFighter(Fighter); }
		[[nodiscard]] const Judoka* GetFighter(Fighter Fighter) const;
		void SetFighter(Fighter Fighter, const Judoka* NewFighter) {
			if (Fighter == Fighter::White)
				m_White.m_Judoka = NewFighter;
			else
				m_Blue.m_Judoka = NewFighter;
		}
		void SwapFighters();
		[[nodiscard]] std::vector<const Judoka*> GetPotentialFighters() const;
		[[nodiscard]] std::vector<const Judoka*> GetPotentialFighters(Fighter Fighter) const;

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
		bool IsEmptySlot(Fighter Fighter) const;
		bool IsEmptyMatch() const;
		const std::vector<const Match*> GetDependentMatches() const;//Returns a list of matches this match depends upon as in the depend matches need to conclude in order for this match to be scheduled
		auto GetDependencyTypeOf(Fighter Fighter) const {
			if (Fighter == Fighter::White)
				return m_White.m_Type;
			return m_Blue.m_Type;
		}
		const Match* GetDependentMatchOf(Fighter Fighter) const {
			if (Fighter == Fighter::White)
				return m_White.m_DependentMatch;
			return m_Blue.m_DependentMatch;
		}
		const MatchTable* GetDependentMatchTableOf(Fighter Fighter) const {
			if (Fighter == Fighter::White)
				return m_White.m_DependentMatchTable;
			return m_Blue.m_DependentMatchTable;
		}

		const RuleSet& GetRuleSet() const;
		const RuleSet* GetOwnRuleSet() const { return m_Rules; }
		void SetRuleSet(const RuleSet* NewRuleSet) { m_Rules = NewRuleSet; }

		uint32_t GetCurrentBreaktime() const;

		const ITournament* GetTournament() const { return m_Tournament; }

		bool IsBestOfThree() const { return m_White.m_Type == DependencyType::BestOfThree; }

		Tag  GetTag() const { return m_Tag; }
		void SetTag(Tag NewTag) { m_Tag = NewTag; }

		//Serialize
		void ToString(YAML::Emitter& Yaml) const;

		void operator >>(YAML::Emitter& Yaml) const;
		void operator >>(nlohmann::json& Json) const;

		bool operator ==(const Match& rhs) const {
			return GetUUID() == rhs.GetUUID();
		}

	private:
		bool IsCompletelyEmptyMatch() const { return IsEmptySlot(Fighter::White) && IsEmptySlot(Fighter::Blue); }
		void SetResult(const Result& Result) { m_Result = Result; SetState(Status::Concluded); }
		void SetState(Status NewState) { m_State = NewState; }
		void SetTournament(const ITournament* Tournament) { m_Tournament = Tournament; }

		void SetLog(const MatchLog& NewLog) {
			m_Log.m_Mutex.lock();
			m_Log.SetEvents() = NewLog.GetEvents();
			m_Log.m_Mutex.unlock();
		}


		DependentJudoka m_White;
		DependentJudoka m_Blue;

		Status m_State = Status::Scheduled;

		Result m_Result;

		MatchLog m_Log;//Log of the match

		const MatchTable* m_Table = nullptr;//Match table this fight is associated with (if any)
		const RuleSet*    m_Rules = nullptr;//Custom rule set for the match (if available)

		const ITournament* m_Tournament = nullptr;

		uint32_t m_MatID = 0;

		Tag m_Tag;
	};
}