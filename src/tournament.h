#pragma once
#include <string>
#include <unordered_map>
#include "itournament.h"
#include "judoka.h"
#include "matchtable.h"
#include "mat.h"
#include "schedule_entry.h"
#include "database.h"
#include "dm4.h"



namespace Judoboard
{
	class Tournament : public ITournament
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
		Tournament(const MD5& File, Database* pDatabase = nullptr);
		~Tournament();

		void Reset();
		void ConnectToDatabase(Database& db);//Replaces all local references to judoka with reference to the database (as long as the tournament is not finalized)

		[[nodiscard]]
		virtual std::string GetName() const override { return m_Name; }//Returns the name of the tournament
		const auto& GetSchedule() const { return m_Schedule; }
		Match* FindMatch(uint32_t ID) const;
		Match* FindMatch(const UUID& UUID) const override;

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
		MatchTable* FindMatchTable(const UUID& ID);
		const MatchTable* FindMatchTable(const UUID& ID) const;
		int FindMatchTableIndex(uint32_t ID) const;

		//Judoka
		Judoka* FindParticipant(uint32_t ID) { return m_StandingData.FindJudoka(ID); }
		const Judoka* FindParticipant(uint32_t ID) const { return m_StandingData.FindJudoka(ID); }
		Judoka* FindParticipant(const UUID& UUID) { return m_StandingData.FindJudoka(UUID); }
		const Judoka* FindParticipant(const UUID& UUID) const { return m_StandingData.FindJudoka(UUID); }

		//Rule Sets
		virtual const RuleSet* GetDefaultRuleSet() const override { return m_pDefaultRules; }
		virtual void SetDefaultRuleSet(const RuleSet* NewDefaultRuleSet) override { m_pDefaultRules = NewDefaultRuleSet; }
		virtual const RuleSet* FindRuleSetByName(const std::string& Name) const override { return m_StandingData.FindRuleSetByName(Name); }
		virtual RuleSet* FindRuleSetByName(const std::string& Name) override { return m_StandingData.FindRuleSetByName(Name); }
		virtual const RuleSet* FindRuleSet(const UUID& UUID) const override { return m_StandingData.FindRuleSet(UUID); }
		virtual RuleSet* FindRuleSet(const UUID& UUID) override { return m_StandingData.FindRuleSet(UUID); }

		//Master schedule / schedule entries
		Schedulable* GetScheduleEntry(uint32_t Index);
		bool MoveScheduleEntryUp(uint32_t ID);
		bool MoveScheduleEntryDown(uint32_t ID);

		//Disqualifications
		void Disqualify(const Judoka& Judoka);
		void RevokeDisqualification(const Judoka& Judoka);

		//Events
		virtual void OnMatchConcluded(const Match& Match) const override {}

		//Serialization
		const std::string Schedule2String() const;
		const std::string Participants2String() const;
		const std::string MasterSchedule2String() const;

		void GenerateSchedule();

		bool Save() const {
			if (!m_AutoSave) return true;
			return Save("tournaments/" + m_Name);
		}

	private:
		bool Load(const std::string& Filename);
		bool Save(const std::string& Filename) const;

		int32_t  GetMaxScheduleIndex(uint32_t Mat = 0) const;
		uint32_t GetMaxEntriesAtScheduleIndex(uint32_t MatID, int32_t ScheduleIndex) const;

		std::vector<MatchTable*>& SetMatchTables() { return m_MatchTables; }

		std::string m_Name;
		bool m_AutoSave = true;

		std::vector<MatchTable*> m_MatchTables;
		std::vector<Match*> m_Schedule;

		std::vector<Schedulable*> m_SchedulePlanner;

		const RuleSet* m_pDefaultRules = nullptr;//Default rule set of the tournament
	};
}