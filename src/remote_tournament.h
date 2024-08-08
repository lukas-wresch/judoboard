#pragma once
#include "itournament.h"



namespace Judoboard
{
	class RemoteTournament : public ITournament
	{
	public:
		friend class Application;

		RemoteTournament(const std::string& Host, uint16_t Port);

		virtual bool IsLocal() const { return false; }

		virtual std::shared_ptr<Match> GetNextMatch(int32_t MatID = -1) override {
			auto ret = GetNextMatches(MatID);
			if (ret.empty())
				return nullptr;
			return FindInCache(ret[0]->GetUUID());
		}
		virtual std::shared_ptr<const Match> GetNextMatch(int32_t MatID = -1) const override {
			auto ret = GetNextMatches(MatID);
			if (ret.empty())
				return nullptr;
			return FindInCache(ret[0]->GetUUID());
		}
		virtual std::vector<std::shared_ptr<const Match>> GetNextMatches(int32_t MatID) const override;

		virtual bool AddMatch(std::shared_ptr<Match> NewMatch) override;

		virtual std::shared_ptr<Match> GetNextOngoingMatch(int32_t MatID) override;

		//Judoka
		virtual bool IsParticipant(const Judoka& Judoka) const override;
		virtual Judoka* FindParticipant(const UUID& UUID) override;
		virtual const Judoka* FindParticipant(const UUID& UUID) const override;

		//Match tables
		virtual std::shared_ptr<MatchTable> FindMatchTable(const UUID& ID) override;
		virtual std::shared_ptr<const MatchTable> FindMatchTable(const UUID& ID) const override;

		//Rule Sets
		virtual std::shared_ptr<const RuleSet> FindRuleSet(const UUID& UUID) const override;
		virtual std::shared_ptr<RuleSet> FindRuleSet(const UUID& UUID) override;

		//Events
		virtual void OnMatchStarted(const Match& Match) const override {}
		virtual void OnMatchConcluded(const Match& Match) const override;

	private:
		std::string Request2Master(const std::string& URL) const;
		bool Post2Master(const std::string& URL, const YAML::Emitter& Data) const;

		bool IsMatchInCache(const UUID& UUID) const;
		std::shared_ptr<Match> FindInCache(const UUID& UUID) const;
		std::shared_ptr<MatchTable> FindMatchTableInCache(const UUID& UUID) const;


		std::string m_Hostname;
		uint16_t m_Port;

		mutable std::vector<std::shared_ptr<Match>> m_MatchCache;

		mutable std::vector<std::shared_ptr<const Match>> m_NextMatches;
		mutable uint32_t m_NextMatches_Timestamp = 0;//Timestamp when m_NextMatches was received

		mutable std::vector<std::shared_ptr<MatchTable>> m_MatchTables;

		mutable StandingData m_StandingData;//Local cache

		mutable ZED::RecursiveReadWriteMutex m_Mutex;
	};
}