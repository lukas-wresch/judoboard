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
	class Application;


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
		Tournament(const std::string& Name, std::shared_ptr<const RuleSet> RuleSet);
		Tournament(const MD5& File, Database* pDatabase = nullptr);
		~Tournament();

		virtual bool IsLocal() const { return true; }

		void Reset();

		bool Load(const YAML::Node& yaml);

		[[nodiscard]]
		virtual std::string GetName() const override { return m_Name; }//Returns the name of the tournament
		[[nodiscard]]
		virtual std::vector<std::shared_ptr<Match>> GetSchedule() const override;
		std::shared_ptr<Match> FindMatch(const UUID& UUID) const override;
		[[nodiscard]]
		virtual const StandingData& GetDatabase() const override { return m_StandingData; }//Returns a database containing all participants
		void SetYear(uint32_t NewYear) {
			if (!IsReadonly())
				m_StandingData.SetYear(NewYear);
		}

		void SetName(const std::string& NewName) {\
			if (!IsReadonly())
				m_Name = NewName;
		}
		void EnableAutoSave(bool Enable = true) { m_AutoSave = Enable; }
		bool IsAutoSave() const { return m_AutoSave; }

		const std::string& GetDescription() const { return m_Description; }
		void SetDescription(const std::string& Description) { m_Description = Description; }

		virtual std::shared_ptr<const Association> GetOrganizer() const override { return m_Organizer; }
		void SetOrganizer(std::shared_ptr<Association> NewOrganizer) {
			if (!NewOrganizer || IsReadonly())
				return;
			if (NewOrganizer->GetLevel() == -1)
				m_StandingData.AddClub(std::dynamic_pointer_cast<Club>(NewOrganizer));
			else
				m_StandingData.AddAssociation(NewOrganizer);
			m_Organizer = NewOrganizer;
		}

		Status GetStatus() const;
		bool CanCloseTournament() const;
		void DeleteAllMatchResults();

		bool IsReadonly() const { return m_Readonly; }
		void IsReadonly(bool Enable) { if (m_Name[0] != 0x00) m_Readonly = Enable; }

		bool AddMatch(std::shared_ptr<Match> NewMatch);
		//bool AddMatch(Match&& NewMatch) { return AddMatch(new Match(NewMatch)); }
		virtual std::shared_ptr<Match> GetNextMatch(int32_t MatID = -1) const override;//Returns the next match for a given mat if available, otherwise null pointer is returned
		std::shared_ptr<const Match> GetNextMatch(int32_t MatID, uint32_t& StartIndex) const;//Returns the next match for a given mat if available, otherwise null pointer is returned
		virtual std::shared_ptr<Match> GetNextOngoingMatch(int32_t MatID) override;//Returns the next match that has already started for a given mat if available, otherwise null pointer is returned
		bool ReviseMatch(const UUID& MatchID, IMat& Mat);

		virtual bool RemoveMatch(const UUID& MatchID) override;
		virtual bool MoveMatchUp(const UUID&  MatchID, uint32_t MatID = 0) override;
		virtual bool MoveMatchDown(const UUID&  MatchID, uint32_t MatID = 0) override;

		virtual std::vector<Match> GetNextMatches(int32_t MatID) const override;

		//Participant / Judoka
		virtual bool IsParticipant(const Judoka& Judoka) const override { return m_StandingData.FindJudoka(Judoka.GetUUID()); }
		virtual std::vector<Judoka*> GetParticipants() const override { return m_StandingData.GetAllJudokas(); }
		virtual bool AddParticipant(Judoka* Judoka) override;
		virtual bool RemoveParticipant(const UUID& UUID) override;

		virtual       Judoka* FindParticipant(const UUID& UUID)       override { return m_StandingData.FindJudoka(UUID); }
		virtual const Judoka* FindParticipant(const UUID& UUID) const override { return m_StandingData.FindJudoka(UUID); }

		virtual bool MarkedAsWeighted(const Judoka& Judoka) override;
		virtual bool IsMarkedAsWeighted(const Judoka& Judoka) const override;

		uint32_t GetHighestMatIDUsed() const;//Returns the highest ID of all mats that are used in the tournament. Returns zero if no mats are used
		bool IsMatUsed(uint32_t ID) const;

		void SwapAllFighters();//Swaps the white with the blue fighter of all matches

		//Match tables
		void AddMatchTable(std::shared_ptr<MatchTable> NewMatchTable);
		bool RemoveMatchTable(const UUID& UUID);
		const std::vector<std::shared_ptr<MatchTable>>& GetMatchTables() const { return m_MatchTables; }
		virtual std::shared_ptr<MatchTable> FindMatchTable(const UUID& ID) override;
		virtual std::shared_ptr<const MatchTable> FindMatchTable(const UUID& ID) const override;
		std::shared_ptr<MatchTable> FindMatchTableByName(const std::string& Name);
		std::shared_ptr<MatchTable> FindMatchTableByDescription(const std::string& Description);

		//Clubs
		virtual std::shared_ptr<Club> FindClub(const UUID& UUID) override { return m_StandingData.FindClub(UUID); }
		virtual std::shared_ptr<Club> FindClubByName(const std::string& Name) override { return m_StandingData.FindClubByName(Name); }
		virtual bool RemoveClub(const UUID& UUID) override { return m_StandingData.DeleteClub(UUID); }

		//Associations
		virtual std::shared_ptr<Association> FindAssociation(const UUID& UUID) override { return m_StandingData.FindAssociation(UUID); }
		virtual bool RemoveAssociation(const UUID& UUID) override { return m_StandingData.DeleteAssociation(UUID); }

		//Rule Sets
		virtual std::shared_ptr<const RuleSet> GetDefaultRuleSet() const override { return m_pDefaultRules; }
		virtual void SetDefaultRuleSet(std::shared_ptr<RuleSet> NewDefaultRuleSet) override {
			if (!IsReadonly())
			{
				m_StandingData.AddRuleSet(NewDefaultRuleSet);
				m_pDefaultRules = NewDefaultRuleSet;
			}
		}
		virtual bool AddRuleSet(std::shared_ptr<RuleSet> NewRuleSet) override { return m_StandingData.AddRuleSet(NewRuleSet); }
		virtual std::shared_ptr<const RuleSet> FindRuleSetByName(const std::string& Name) const override { return m_StandingData.FindRuleSetByName(Name); }
		virtual std::shared_ptr<RuleSet> FindRuleSetByName(const std::string& Name) override { return m_StandingData.FindRuleSetByName(Name); }
		virtual std::shared_ptr<const RuleSet> FindRuleSet(const UUID& UUID) const override { return m_StandingData.FindRuleSet(UUID); }
		virtual std::shared_ptr<RuleSet> FindRuleSet(const UUID& UUID) override { return m_StandingData.FindRuleSet(UUID); }

		//Age groups
		virtual bool AddAgeGroup(std::shared_ptr<AgeGroup> NewAgeGroup) override;
		virtual bool RemoveAgeGroup(const UUID& UUID) override;
		virtual bool AssignJudokaToAgeGroup(const Judoka* Judoka, std::shared_ptr<const AgeGroup> AgeGroup) override;
		virtual std::shared_ptr<const AgeGroup> GetAgeGroupOfJudoka(const Judoka* Judoka) const override {
			if (!Judoka) return nullptr;
			auto it = m_JudokaToAgeGroup.find(Judoka->GetUUID());
			if (it != m_JudokaToAgeGroup.end())
				return m_StandingData.FindAgeGroup(it->second);
			return nullptr;
		}
		std::shared_ptr<AgeGroup> FindAgeGroup(const UUID& UUID) { return m_StandingData.FindAgeGroup(UUID); }
		std::shared_ptr<const AgeGroup> FindAgeGroup(const UUID& UUID) const { return m_StandingData.FindAgeGroup(UUID); }
		virtual std::vector<std::shared_ptr<const AgeGroup>> GetEligableAgeGroupsOfJudoka(const Judoka* Judoka) const override;
		virtual std::vector<std::shared_ptr<const AgeGroup>> GetAgeGroups() const override;
		virtual void GetAgeGroupInfo(YAML::Emitter& Yaml, std::shared_ptr<const AgeGroup> AgeGroup) const override;

		//Master schedule / schedule entries
		bool MoveScheduleEntryUp(const UUID& UUID) override;
		bool MoveScheduleEntryDown(const UUID& UUID) override;

		virtual std::vector<WeightclassDescCollection> GenerateWeightclasses(int Min, int Max, int Diff, const std::vector<std::shared_ptr<const AgeGroup>>& AgeGroups, bool SplitGenders) const override;
		virtual bool ApplyWeightclasses(const std::vector<WeightclassDescCollection>& Descriptors) override;

		//Disqualifications
		bool IsDisqualified(const Judoka& Judoka) const;
		void Disqualify(const Judoka& Judoka) override;
		void RevokeDisqualification(const Judoka& Judoka) override;

		//Lottery
		virtual bool PerformLottery() override;
		virtual uint32_t GetLotteryTier() const override { return m_LotteryTier; }
		virtual bool SetLotteryTier(uint32_t NewLotteryTier) override {
			if (GetStatus() == Status::Scheduled && !IsReadonly())
			{
				m_LotteryTier = NewLotteryTier;
				return PerformLottery();
			}
			return false;
		}
		size_t GetLotOfAssociation(const UUID& UUID) const;
		virtual std::vector<std::pair<UUID, size_t>> GetLots() const override { return m_AssociationToLotNumber; }

		//Events
		virtual void OnMatchStarted(const Match& Match) const override;
		virtual void OnMatchConcluded(const Match& Match) const override;
		virtual bool OnUpdateParticipant(const UUID& UUID) override;//Calling this function we recalculate the given judoka
		virtual bool OnUpdateMatchTable(const UUID& UUID) override;//Calling this function we recalculate the given match table

		//Serialization
		virtual const std::string Schedule2String(bool ImportantOnly, int Mat = -1) const override;
		virtual const std::string MasterSchedule2String() const override;
		virtual nlohmann::json Schedule2ResultsServer() const override;

		void OrganizeMasterSchedule();
		virtual void BuildSchedule();
		virtual void GenerateSchedule() override;

		virtual bool Save() override {
			if (!m_AutoSave) return true;
			return SaveYAML("tournaments/" + m_Name + ".yml");
		}
		void ScheduleSave() const
		{
			m_LastSaveTime = 0;
		}
		uint32_t TimeSinceLastSave() const
		{
			return Timer::GetTimestamp() - m_LastSaveTime;
		}

		void SetApplication(const Application* App) { m_Application = App; }

	private:
		bool LoadYAML(const std::string& Filename);
		bool SaveYAML(const std::string& Filename);

		void FindAgeGroupForJudoka(const Judoka& Judoka);

		int32_t  GetFreeScheduleIndex(uint32_t Mat = 0) const;
		int32_t  GetMaxScheduleIndex(uint32_t Mat = 0) const;
		uint32_t GetMaxEntriesAtScheduleIndex(uint32_t MatID, int32_t ScheduleIndex) const;

		std::vector<std::shared_ptr<MatchTable>>& SetMatchTables() { return m_MatchTables; }

		class ScopedLock
		{
		public:
			ScopedLock(std::recursive_mutex& Mutex) : m_Mutex(Mutex) { Mutex.lock(); }
			~ScopedLock() { m_Mutex.unlock(); }

		private:
			std::recursive_mutex& m_Mutex;
		};

		//[[nodiscard]]
		//ScopedLock LockTillScopeEnd() const { return ScopedLock(m_mutex); }

		std::string m_Name;
		bool m_AutoSave = true;
		mutable uint32_t m_LastSaveTime = 0;//Timestamp when the file was saved
		std::string m_Description;
		bool m_Readonly = false;

		std::shared_ptr<const Association> m_Organizer;//Tournament is organized by this association, only childen of this association can participate
		uint32_t m_LotteryTier = 0;//Tier for performing lottery (usually organizer tier + 1)

		const Application* m_Application = nullptr;

		std::vector<std::shared_ptr<MatchTable>> m_MatchTables;
		//std::vector<Match*> m_Schedule;
		std::vector<std::pair<std::shared_ptr<MatchTable>, size_t>> m_Schedule;

		std::shared_ptr<const RuleSet> m_pDefaultRules;//Default rule set of the tournament

		std::unordered_set<UUID> m_DisqualifiedJudoka;

		std::unordered_map<UUID, UUID> m_JudokaToAgeGroup;//Maps judoka to the age group he/she is starting in
		std::set<UUID> m_JudokaWeighted;
		std::vector<std::pair<UUID, size_t>> m_AssociationToLotNumber;//Maps associations to the lot number
	};
}