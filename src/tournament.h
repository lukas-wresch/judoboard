#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
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
		[[deprecated]]
		void ConnectToDatabase(Database& db);//Replaces all local references to judoka with reference to the database (as long as the tournament is not finalized)

		[[nodiscard]]
		virtual std::string GetName() const override { return m_Name; }//Returns the name of the tournament
		const auto& GetSchedule() const { return m_Schedule; }
		Match* FindMatch(uint32_t ID) const;
		Match* FindMatch(const UUID& UUID) const override;
		[[nodiscard]]
		const StandingData& GetDatabase() const { return m_StandingData; }//Returns a database containing all participants
		//virtual const std::string JudokaToJSON() const override { return m_StandingData.JudokaToJSON(); }

		void SetName(const std::string& NewName) { m_Name = NewName; }
		void EnableAutoSave(bool Enable = true) { m_AutoSave = Enable; }

		Status GetStatus() const;
		bool CanCloseTournament() const;
		void DeleteAllMatchResults();

		bool AddMatch(Match* NewMatch);
		bool AddMatch(Match&& NewMatch) { return AddMatch(new Match(NewMatch)); }
		Match* GetNextMatch(int32_t MatID = -1) const;//Returns the next match for a given mat if available, otherwise null pointer is returned
		const Match* GetNextMatch(int32_t MatID, uint32_t& StartIndex) const;//Returns the next match for a given mat if available, otherwise null pointer is returned

		bool RemoveMatch(const UUID& MatchID);
		bool MoveMatchUp(const UUID&  MatchID);
		bool MoveMatchDown(const UUID&  MatchID);

		std::vector<Match> GetNextMatches(uint32_t MatID) const;

		//Participant / Judoka
		virtual bool IsParticipant(const Judoka& Judoka) const override { return m_StandingData.FindJudoka(Judoka.GetUUID()); }
		virtual std::unordered_map<UUID, Judoka*> GetParticipants() const override { return m_StandingData.GetAllJudokas(); }
		virtual bool AddParticipant(Judoka* Judoka) override;
		virtual bool RemoveParticipant(const UUID& UUID) override;

		virtual       Judoka* FindParticipant(const UUID& UUID)       override { return m_StandingData.FindJudoka(UUID); }
		virtual const Judoka* FindParticipant(const UUID& UUID) const override { return m_StandingData.FindJudoka(UUID); }

		uint32_t GetHighestMatIDUsed() const;//Returns the highest ID of all mats that are used in the tournament. Returns zero if no mats are used
		bool IsMatUsed(uint32_t ID) const;

		//Match tables
		void AddMatchTable(MatchTable* NewMatchTable);
		bool UpdateMatchTable(const UUID& UUID);//Calling this function we recalculate the given match table
		bool RemoveMatchTable(const UUID& UUID);
		const std::vector<MatchTable*>& GetMatchTables() const { return m_MatchTables; }
		virtual MatchTable* FindMatchTable(const UUID& ID) override;
		virtual const MatchTable* FindMatchTable(const UUID& ID) const override;
		MatchTable* FindMatchTableByName(const std::string& Name);
		MatchTable* FindMatchTableByDescription(const std::string& Description);

		//Rule Sets
		virtual const RuleSet* GetDefaultRuleSet() const override { return m_pDefaultRules; }
		virtual void SetDefaultRuleSet(RuleSet* NewDefaultRuleSet) override {
			m_StandingData.AddRuleSet(NewDefaultRuleSet);
			m_pDefaultRules = NewDefaultRuleSet;
		}
		virtual bool AddRuleSet(RuleSet* NewRuleSet) override { return m_StandingData.AddRuleSet(NewRuleSet); }
		virtual const RuleSet* FindRuleSetByName(const std::string& Name) const override { return m_StandingData.FindRuleSetByName(Name); }
		virtual RuleSet* FindRuleSetByName(const std::string& Name) override { return m_StandingData.FindRuleSetByName(Name); }
		virtual const RuleSet* FindRuleSet(const UUID& UUID) const override { return m_StandingData.FindRuleSet(UUID); }
		virtual RuleSet* FindRuleSet(const UUID& UUID) override { return m_StandingData.FindRuleSet(UUID); }

		//Age groups
		virtual bool AddAgeGroup(AgeGroup* NewAgeGroup) override;
		virtual bool RemoveAgeGroup(const UUID& UUID) override;
		virtual bool AssignJudokaToAgeGroup(const Judoka* Judoka, const AgeGroup* AgeGroup) override;
		virtual const AgeGroup* GetAgeGroupOfJudoka(const Judoka* Judoka) const override {
			if (!Judoka) return nullptr;
			auto it = m_JudokaToAgeGroup.find(Judoka->GetUUID());
			if (it != m_JudokaToAgeGroup.end())
				return m_StandingData.FindAgeGroup(it->second);
			return nullptr;
		}
		AgeGroup* FindAgeGroup(const UUID& UUID) { return m_StandingData.FindAgeGroup(UUID); }
		const AgeGroup* FindAgeGroup(const UUID& UUID) const { return m_StandingData.FindAgeGroup(UUID); }
		virtual std::vector<const AgeGroup*> GetEligableAgeGroupsOfJudoka(const Judoka* Judoka) const override;
		virtual std::vector<const AgeGroup*> GetAgeGroups() const override;
		virtual void ListAgeGroups(YAML::Emitter& Yaml) const override;

		//Master schedule / schedule entries
		Schedulable* GetScheduleEntry(const UUID& UUID) override;
		bool MoveScheduleEntryUp(const UUID& UUID) override;
		bool MoveScheduleEntryDown(const UUID& UUID) override;

		virtual std::string GenerateWeightclasses(int Min, int Max, int Diff, const std::vector<const AgeGroup*>& AgeGroups) override;

		//Disqualifications
		bool IsDisqualified(const Judoka& Judoka) const;
		void Disqualify(const Judoka& Judoka) override;
		void RevokeDisqualification(const Judoka& Judoka) override;

		//Events
		virtual void OnMatchConcluded(const Match& Match) const override {}

		//Serialization
		const std::string Schedule2String() const override;
		const std::string Participants2String() const override;
		const std::string MasterSchedule2String() const override;

		void GenerateSchedule();

		bool Save() const {
			if (!m_AutoSave) return true;
			return SaveYAML("tournaments/" + m_Name + ".yml");
		}

	private:
		bool LoadYAML(const std::string& Filename);
		bool SaveYAML(const std::string& Filename) const;

		void FindAgeGroupForJudoka(const Judoka& Judoka);

		int32_t  GetMaxScheduleIndex(uint32_t Mat = 0) const;
		uint32_t GetMaxEntriesAtScheduleIndex(uint32_t MatID, int32_t ScheduleIndex) const;

		std::vector<MatchTable*>& SetMatchTables() { return m_MatchTables; }

		std::string m_Name;
		bool m_AutoSave = true;

		std::vector<MatchTable*> m_MatchTables;
		std::vector<Match*> m_Schedule;

		std::vector<Schedulable*> m_SchedulePlanner;

		const RuleSet* m_pDefaultRules = nullptr;//Default rule set of the tournament

		std::unordered_set<UUID> m_DisqualifiedJudoka;

		std::unordered_map<UUID, UUID> m_JudokaToAgeGroup;//Maps judoka to the age group he/she is starting in
	};
}