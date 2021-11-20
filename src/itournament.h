#pragma once
#include <cassert>
#include "judoka.h"
#include "matchtable.h"
#include "mat.h"
#include "schedule_entry.h"
#include "database.h"



namespace Judoboard
{
	class ITournament : public ID
	{
	public:
		virtual std::string GetName() const { return ""; }//Returns the name of the tournament
		//const auto& GetSchedule() const { return m_Schedule; }
		virtual Match* FindMatch(uint32_t ID) const { return nullptr; }
		virtual Match* FindMatch(const UUID& UUID) const { return nullptr; }
		[[nodiscard]]
		Match* FindMatch(const Match& Match) const { return FindMatch(Match.GetUUID()); }
		[[nodiscard]]
		const StandingData& GetDatabase() const { return m_StandingData; }//Returns a database containing all participants

		//void EnableAutoSave(bool Enable = true) { m_AutoSave = Enable; }

		virtual Status GetStatus() const { return Status::Concluded; }
		virtual bool CanCloseTournament() const { return false; }
		//void DeleteAllMatchResults();

		virtual bool AddMatch(Match* NewMatch) { return false; }
		bool AddMatch(Match&& NewMatch) { return AddMatch(new Match(NewMatch)); }
		virtual Match* GetNextMatch(int32_t MatID = -1) const { return nullptr; }//Returns the next match for a given mat if available, otherwise null pointer is returned
		//const Match* GetNextMatch(int32_t MatID, uint32_t& StartIndex) const;//Returns the next match for a given mat if available, otherwise null pointer is returned

		virtual std::vector<const Match*> GetNextMatches(uint32_t MatID) const = 0;

		virtual bool DeleteMatch(uint32_t MatchID) { return false; }
		virtual bool MoveMatchUp(uint32_t MatchID) { return false; }
		virtual bool MoveMatchDown(uint32_t MatchID) { return false; }

		virtual bool IsParticipant(const Judoka& Judoka) const = 0;
		//const std::unordered_map<uint32_t, Judoka*>& GetParticipants() const { return m_StandingData.GetAllJudokas(); }
		virtual bool AddParticipant(Judoka* Judoka) { return false; }
		virtual bool RemoveParticipant(uint32_t ID) { return false; }

		virtual uint32_t GetHighestMatIDUsed() const { return 0; }//Returns the highest ID of all mats that are used in the tournament. Returns zero if no mats are used
		virtual bool IsMatUsed(uint32_t ID) const { return false; }

		//Match tables
		//uint32_t GetFreeMatchTableID() const;//Returns an unused/free ID that should be used for the next match table
		virtual void AddMatchTable(MatchTable* NewMatchTable) {}
		virtual void UpdateMatchTable(uint32_t ID) {}//Calling this function we recalculate the given match table
		virtual bool DeleteMatchTable(uint32_t ID) { return false; }
		virtual const std::vector<MatchTable*>& GetMatchTables() const {
			assert(false);
			static std::vector<MatchTable*> ret;
			return ret;
		}

		virtual std::vector<MatchTable*>& SetMatchTables() {
			assert(false);
			static std::vector<MatchTable*> ret;
			return ret;
		}

		virtual MatchTable* FindMatchTable(uint32_t ID) { return nullptr; }
		virtual const MatchTable* FindMatchTable(uint32_t ID) const { return nullptr; }
		virtual MatchTable* FindMatchTable(UUID ID) { return nullptr; }
		virtual const MatchTable* FindMatchTable(UUID ID) const { return nullptr; }
		virtual int FindMatchTableIndex(uint32_t ID) const { return -1; }

		//Judoka
		//Judoka* FindParticipant(uint32_t ID) { return m_StandingData.FindJudoka(ID); }
		//const Judoka* FindParticipant(uint32_t ID) const { return m_StandingData.FindJudoka(ID); }
		virtual Judoka* FindParticipant(const UUID& UUID) = 0;
		virtual const Judoka* FindParticipant(const UUID& UUID) const = 0;

		//Rule Sets
		virtual const RuleSet* GetDefaultRuleSet() const { return nullptr; }
		virtual void SetDefaultRuleSet(const RuleSet* NewDefaultRuleSet) {}
		virtual const RuleSet* FindRuleSetByName(const std::string& Name) const { return nullptr; }
		virtual RuleSet* FindRuleSetByName(const std::string& Name) { return nullptr; }
		virtual const RuleSet* FindRuleSet(const UUID& UUID) const { return nullptr; }
		virtual RuleSet* FindRuleSet(const UUID& UUID) { return nullptr; }

		//Master schedule / schedule entries
		virtual Schedulable* GetScheduleEntry(uint32_t Index) { return nullptr; }
		virtual bool MoveScheduleEntryUp(uint32_t ID) { return false; }
		virtual bool MoveScheduleEntryDown(uint32_t ID) { return false; }

		//Disqualifications
		virtual void Disqualify(const Judoka& Judoka) {}
		virtual void RevokeDisqualification(const Judoka& Judoka) {}

		//Events
		virtual void OnMatchConcluded(const Match& Match) const = 0;

		//Serialization
		virtual const std::string Schedule2String() const { return ""; }
		virtual const std::string Participants2String() const { return ""; }
		virtual const std::string MasterSchedule2String() const { return ""; }

		virtual void GenerateSchedule() {}

		virtual bool Save() const { return false; }

		void Lock()   const { m_mutex.lock(); }
		void Unlock() const { m_mutex.unlock(); }

	protected:
		StandingData m_StandingData;//Local database for the tournament containing all participants and rule sets

		mutable std::mutex m_mutex;
	};
}