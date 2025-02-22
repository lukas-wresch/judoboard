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

		virtual const Association* GetOrganizer() const { assert(false); return nullptr; }

		virtual bool AddMatch(Match* NewMatch) { assert(false); return false; }
		bool AddMatch(Match&& NewMatch) { return AddMatch(new Match(std::move(NewMatch))); }

		virtual Match* GetNextOngoingMatch(int32_t MatID) { assert(false); return nullptr; }//Returns the next match that has already started for a given mat if available, otherwise null pointer is returned

		virtual Match* GetNextMatch(int32_t MatID = -1) = 0;//Returns the next match for a given mat if available, otherwise null pointer is returned
		virtual const Match* GetNextMatch(int32_t MatID = -1) const = 0;//Returns the next match for a given mat if available, otherwise null pointer is returned
		virtual std::vector<const Match*> GetNextMatches(int32_t MatID) const = 0;

		virtual bool RemoveMatch(const UUID& MatchID) { assert(false); return false; }
		virtual bool MoveMatchUp(const UUID& MatchID, uint32_t MatID = 0) { assert(false); return false; }
		virtual bool MoveMatchDown(const UUID& MatchID, uint32_t MatID = 0) { assert(false); return false; }
		virtual bool MoveMatchTo(const UUID& From, const UUID& To, bool Above) { assert(false); return false; }

		//Judoka
		virtual bool IsParticipant(const Judoka& Judoka) const = 0;
		virtual std::vector<Judoka*> GetParticipants() const {
			std::vector<Judoka*> ret;
			return ret;
		}
		virtual const Judoka* FindParticipant(const UUID& UUID) const { assert(false); return nullptr; }
		virtual Judoka* FindParticipant(const UUID& UUID) { assert(false); return nullptr; }
			
		virtual bool AddParticipant(Judoka* Judoka) { assert(false); return false; }
		virtual bool RemoveParticipant(const UUID& ID) { assert(false); return false; }

		virtual bool MarkedAsWeighted(const Judoka& Judoka) { assert(false); return false; }
		virtual bool IsMarkedAsWeighted(const Judoka& Judoka) const { assert(false); return false; }

		virtual uint32_t GetHighestMatIDUsed() const { assert(false); return 0; }//Returns the highest ID of all mats that are used in the tournament. Returns zero if no mats are used
		virtual bool IsMatUsed(uint32_t ID) const { assert(false); return false; }

		//Match tables
		virtual void AddMatchTable(MatchTable* NewMatchTable) { assert(false); }
		virtual bool RemoveMatchTable(const UUID& UUID) { assert(false); return false; }
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

		virtual MatchTable* FindMatchTable(const UUID& ID) = 0;
		virtual const MatchTable* FindMatchTable(const UUID& ID) const = 0;

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
		virtual const RuleSet* FindRuleSet(const UUID& UUID) const = 0;
		virtual RuleSet* FindRuleSet(const UUID& UUID) = 0;

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
		virtual bool ApplyWeightclasses(const std::vector<WeightclassDescCollection>& Descriptors) { assert(false); return false; }

		//Disqualifications
		virtual void Disqualify(const Judoka& Judoka) { assert(false); }
		virtual void RevokeDisqualification(const Judoka& Judoka) { assert(false); }

		//Lots
		virtual bool PerformLottery() { assert(false); return false; }
		virtual uint32_t GetLotteryTier() const { assert(false); return 0; }
		virtual bool SetLotteryTier(uint32_t NewLotteryTier) { assert(false); return false; }
		virtual std::vector<std::pair<UUID, size_t>> GetLots() const { std::vector<std::pair<UUID, size_t>> ret; return ret; }

		//Events
		virtual void OnMatchStarted(const Match& Match) const = 0;
		virtual void OnMatchConcluded(const Match& Match) const = 0;
		virtual bool OnUpdateParticipant(const UUID& UUID) { assert(false); return false; }//Calling this function we recalculate the given judoka
		virtual bool OnUpdateMatchTable(const UUID& UUID)  { assert(false); return false; }//Calling this function we recalculate the given match table
		virtual bool OnUpdateAgeGroup(const UUID& UUID)  { assert(false); return false; }

		//Serialization
		virtual const std::string Schedule2String(bool ImportantOnly, int Mat = -1) const { assert(false); return ""; }
		virtual const std::string MasterSchedule2String() const { assert(false); return ""; }
		virtual nlohmann::json Schedule2ResultsServer() const { assert(false); return ""; }

		virtual void BuildSchedule() {}
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

		ZED::RecursiveReadWriteMutex m_mutex;
	};
}