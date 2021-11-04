#pragma once
#include <string>
#include <unordered_map>
#include "judoka.h"
#include "matchtable.h"
#include "mat.h"
#include "schedule_entry.h"
#include "database.h"



namespace Judoboard
{
	class Tournament
	{
		friend class Application;

	public:
		struct MatchEntry
		{
			uint32_t Index;
			Match* NextMatch;
		};


		Tournament() = default;
		Tournament(const std::string& Name);
		Tournament(const std::string& Name, const RuleSet* RuleSet);
		~Tournament();

		void Reset();
		void ConnectToDatabase(Database& db);//Replaces all local references to judoka with reference to the database (as long as the tournament is not finalized)

		const std::string& GetName() const { return m_Name; }//Returns the name of the tournament
		const auto& GetSchedule() const { return m_Schedule; }
		Match* FindMatch(uint32_t ID) const;
		const StandingData& GetDatabase() const { return m_StandingData; }//Returns a database containing all participants

		void EnableAutoSave(bool Enable = true) { m_AutoSave = Enable; }

		Status GetStatus() const;
		bool CanCloseTournament() const;
		void DeleteAllMatchResults();

		bool AddMatch(Match* NewMatch);
		bool AddMatch(Match&& NewMatch) { return AddMatch(new Match(NewMatch)); }
		Match* GetNextMatch(int32_t MatID = -1) const;//Returns the next match for a given mat if available, otherwise null pointer is returned
		const Match* GetNextMatch(int32_t MatID, uint32_t& StartIndex) const;//Returns the next match for a given mat if available, otherwise null pointer is returned

		bool DeleteMatch(uint32_t MatchID);
		bool MoveMatchUp(uint32_t MatchID);
		bool MoveMatchDown(uint32_t MatchID);

		std::vector<const Match*> GetNextMatches(uint32_t MatID) const;

		bool IsParticipant(const Judoka& Judoka) const { return m_StandingData.FindJudoka(Judoka.GetUUID()); }
		const std::unordered_map<uint32_t, Judoka*>& GetParticipants() const { return m_StandingData.GetAllJudokas(); }
		bool AddParticipant(Judoka* Judoka);
		bool RemoveParticipant(uint32_t ID);

		uint32_t GetHighestMatIDUsed() const;//Returns the highest ID of all mats that are used in the tournament. Returns zero if no mats are used
		bool IsMatUsed(uint32_t ID) const;

		//Match tables
		uint32_t GetFreeMatchTableID() const;//Returns an unused/free ID that should be used for the next match table
		void AddMatchTable(MatchTable* NewMatchTable);
		void UpdateMatchTable(uint32_t ID);//Calling this function we recalculate the given match table
		bool DeleteMatchTable(uint32_t ID);
		const std::vector<MatchTable*>& GetMatchTables() const { return m_MatchTables; }
		MatchTable* FindMatchTable(uint32_t ID);
		const MatchTable* FindMatchTable(uint32_t ID) const;
		MatchTable* FindMatchTable(UUID ID);
		const MatchTable* FindMatchTable(UUID ID) const;
		int FindMatchTableIndex(uint32_t ID) const;

		//Judoka
		Judoka* FindParticipant(uint32_t ID) { return m_StandingData.FindJudoka(ID); }
		const Judoka* FindParticipant(uint32_t ID) const { return m_StandingData.FindJudoka(ID); }
		Judoka* FindParticipant(const UUID& UUID) { return m_StandingData.FindJudoka(UUID); }
		const Judoka* FindParticipant(const UUID& UUID) const { return m_StandingData.FindJudoka(UUID); }

		//Rule Sets
		const RuleSet* GetDefaultRuleSet() const { return m_pDefaultRules; }
		void SetDefaultRuleSet(const RuleSet* NewDefaultRuleSet) { m_pDefaultRules = NewDefaultRuleSet; }
		const RuleSet* FindRuleSet(const std::string& Name) const { return m_StandingData.FindRuleSet(Name); }
		RuleSet* FindRuleSet(const std::string& Name) { return m_StandingData.FindRuleSet(Name); }

		//Master schedule / schedule entries
		Schedulable* GetScheduleEntry(uint32_t Index);
		bool MoveScheduleEntryUp(uint32_t ID);
		bool MoveScheduleEntryDown(uint32_t ID);

		//Disqualifications
		void Disqualify(const Judoka& Judoka);
		void RevokeDisqualification(const Judoka& Judoka);

		void GenerateSchedule();

		//Serialization
		const std::string Schedule2String() const;
		const std::string Participants2String() const;
		const std::string MasterSchedule2String() const;

	private:
		bool Load(const std::string& Filename);
		bool Save(const std::string& Filename) const;
		bool Save() const {
			if (!m_AutoSave) return false;
			return Save("tournaments/" + m_Name);
		}

		int32_t  GetMaxScheduleIndex(uint32_t Mat = 0) const;
		uint32_t GetMaxEntriesAtScheduleIndex(uint32_t MatID, int32_t ScheduleIndex) const;

		std::vector<MatchTable*>& SetMatchTables() { return m_MatchTables; }

		void Lock()   const { m_mutex.lock(); }
		void Unlock() const { m_mutex.unlock(); }

		std::string m_Name;
		bool m_AutoSave = true;

		std::vector<MatchTable*> m_MatchTables;
		std::vector<Match*> m_Schedule;

		std::vector<Schedulable*> m_SchedulePlanner;

		const RuleSet* m_pDefaultRules = nullptr;//Default rule set of the tournament

		StandingData m_StandingData;//Local database for the tournament containing all participants and rule sets

		mutable std::mutex m_mutex;
	};
}