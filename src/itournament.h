#pragma once
#include <cassert>
#include "../ZED/include/read_write_mutex.h"
#include "judoka.h"
#include "matchtable.h"
#include "mat.h"
#include "schedule_entry.h"
#include "database.h"
#include "weightclass_generator.h"



namespace Judoboard
{
	class ITournament : public ID
	{
	public:
		virtual bool IsLocal() const = 0;
		virtual std::string GetName() const { return ""; }//Returns the name of the tournament
		virtual std::vector<Match*> GetSchedule() const { std::vector<Match*> ret; return ret; }
		virtual Match* FindMatch(const UUID& UUID) const { return nullptr; }
		//[[nodiscard]]
		//Match* FindMatch(const Match& Match) const { return FindMatch(Match.GetUUID()); }
		//[[nodiscard]]
		//const StandingData& GetDatabase() const { return m_StandingData; }//Returns a database containing all participants
		virtual const std::string JudokaToJSON() const { return ""; }

		virtual Status GetStatus() const { return Status::Concluded; }
		virtual bool CanCloseTournament() const { return false; }
		//void DeleteAllMatchResults();

		[[nodiscard]]
		virtual const StandingData& GetDatabase() const { return m_StandingData; }//Returns a database containing all participants
		[[nodiscard]]
		virtual StandingData& GetDatabase() { return m_StandingData; }//Returns a database containing all participants

		virtual const Association* GetOrganizer() const { return nullptr; }

		virtual bool AddMatch(Match* NewMatch) { return false; }
		bool AddMatch(Match&& NewMatch) { return AddMatch(new Match(std::move(NewMatch))); }
		virtual Match* GetNextMatch(int32_t MatID = -1) const { return nullptr; }//Returns the next match for a given mat if available, otherwise null pointer is returned
		//const Match* GetNextMatch(int32_t MatID, uint32_t& StartIndex) const;//Returns the next match for a given mat if available, otherwise null pointer is returned

		virtual std::vector<Match> GetNextMatches(uint32_t MatID) const = 0;

		virtual bool RemoveMatch(const UUID& MatchID) { return false; }
		virtual bool MoveMatchUp(const UUID& MatchID, uint32_t MatID = 0) { return false; }
		virtual bool MoveMatchDown(const UUID& MatchID, uint32_t MatID = 0) { return false; }

		//Judoka
		virtual bool IsParticipant(const Judoka& Judoka) const = 0;
		virtual std::vector<Judoka*> GetParticipants() const {
			std::vector<Judoka*> ret;
      return ret; }
		virtual const Judoka* FindParticipant(const UUID& UUID) const { return nullptr; }
		virtual Judoka* FindParticipant(const UUID& UUID) { return nullptr; }
			
		virtual bool AddParticipant(Judoka* Judoka) { return false; }
		virtual bool RemoveParticipant(const UUID& ID) { return false; }

		virtual uint32_t GetHighestMatIDUsed() const { return 0; }//Returns the highest ID of all mats that are used in the tournament. Returns zero if no mats are used
		virtual bool IsMatUsed(uint32_t ID) const { return false; }

		//Match tables
		virtual void AddMatchTable(MatchTable* NewMatchTable) {}
		virtual bool UpdateMatchTable(const UUID& UUID) { return false; }//Calling this function we recalculate the given match table
		virtual bool RemoveMatchTable(const UUID& UUID) { return false; }
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

		virtual MatchTable* FindMatchTable(const UUID& ID) { return nullptr; }
		virtual const MatchTable* FindMatchTable(const UUID& ID) const { return nullptr; }

		//Clubs
		virtual Club* FindClub(const UUID& UUID) { return nullptr; }
		virtual Club* FindClubByName(const std::string& Name) { return nullptr; }
		virtual bool RemoveClub(const UUID& UUID) { return false; }

		//Associations
		virtual Association* FindAssociation(const UUID& UUID) { return nullptr; }
		virtual bool RemoveAssociation(const UUID& UUID) { return false; }

		//Rule Sets
		virtual const RuleSet* GetDefaultRuleSet() const { return nullptr; }
		virtual void SetDefaultRuleSet(RuleSet* NewDefaultRuleSet) {}
		virtual bool AddRuleSet(RuleSet* NewRuleSet) { return false; }
		virtual const RuleSet* FindRuleSetByName(const std::string& Name) const { return nullptr; }
		virtual RuleSet* FindRuleSetByName(const std::string& Name) { return nullptr; }
		virtual const RuleSet* FindRuleSet(const UUID& UUID) const { return nullptr; }
		virtual RuleSet* FindRuleSet(const UUID& UUID) { return nullptr; }

		//Age groups
		virtual bool AddAgeGroup(AgeGroup* NewAgeGroup) { return false; }
		virtual bool RemoveAgeGroup(const UUID& UUID) { return false; }
		virtual bool AssignJudokaToAgeGroup(const Judoka* Judoka, const AgeGroup* AgeGroup) { return false; }
		virtual AgeGroup* FindAgeGroup(const UUID& UUID) { return nullptr; }
		virtual const AgeGroup* FindAgeGroup(const UUID& UUID) const { return nullptr; }
		virtual const AgeGroup* GetAgeGroupOfJudoka(const Judoka* Judoka) const { return nullptr; }
		virtual std::vector<const AgeGroup*> GetEligableAgeGroupsOfJudoka(const Judoka* Judoka) const {
			std::vector<const AgeGroup*> ret;
			return ret;
		}
		virtual std::vector<const AgeGroup*> GetAgeGroups() const {
			std::vector<const AgeGroup*> ret;
			return ret;
		}
		virtual void GetAgeGroupInfo(YAML::Emitter& Yaml, const AgeGroup* AgeGroup) const {}

		//Master schedule / schedule entries
		virtual MatchTable* GetScheduleEntry(const UUID& UUID) { return nullptr; }
		virtual bool MoveScheduleEntryUp(const UUID& UUID) { return false; }
		virtual bool MoveScheduleEntryDown(const UUID& UUID) { return false; }

		virtual std::vector<WeightclassDescCollection> GenerateWeightclasses(int Min, int Max, int Diff, const std::vector<const AgeGroup*>& AgeGroups, bool SplitGenders) const {
			std::vector<WeightclassDescCollection> ret;
			return ret;
		}
		virtual bool ApplyWeightclasses(const std::vector<WeightclassDescCollection>& Descriptors) { return false; }

		//Disqualifications
		virtual void Disqualify(const Judoka& Judoka) {}
		virtual void RevokeDisqualification(const Judoka& Judoka) {}

		//Lots
		virtual bool PerformLottery() { return false; }
		virtual uint32_t GetLotteryTier() const { return 0; }
		virtual void SetLotteryTier(uint32_t NewLotteryTier) {}
		virtual std::vector<std::pair<UUID, size_t>> GetLots() const { std::vector<std::pair<UUID, size_t>> ret; return ret; }

		//Events
		virtual void OnMatchConcluded(const Match& Match) const = 0;

		//Serialization
		virtual const std::string Schedule2String() const { return ""; }
		[[deprecated]]
		virtual const std::string Participants2String() const { return ""; }
		virtual const std::string MasterSchedule2String() const { return ""; }

		virtual void GenerateSchedule() {}

		virtual bool Save() { return false; }

		void LockRead()   const { m_mutex.LockRead(); }
		void UnlockRead() const { m_mutex.UnlockRead(); }
		[[nodiscard]] auto LockReadForScope() const { return m_mutex.LockReadForScope(); }
		void LockWrite()   { m_mutex.LockWrite(); }
		void UnlockWrite() { m_mutex.UnlockWrite(); }
		[[nodiscard]] auto LockWriteForScope() { return m_mutex.LockWriteForScope(); }

	protected:
		StandingData m_StandingData;//Local database for the tournament containing all participants and rule sets

		//mutable std::recursive_mutex m_mutex;
		ZED::RecursiveReadWriteMutex m_mutex;
	};
}