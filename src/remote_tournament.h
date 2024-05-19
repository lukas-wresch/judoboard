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

		virtual std::vector<Match> GetNextMatches(int32_t MatID) const override;
		virtual Match* GetNextMatch(int32_t MatID = -1) const override {
			auto ret = GetNextMatches(MatID);
			if (ret.empty())
				return nullptr;
			return FindInCache(ret[0].GetUUID());
		}

		virtual bool AddMatch(Match* NewMatch) override;

		//Judoka
		virtual bool IsParticipant(const Judoka& Judoka) const override;
		virtual Judoka* FindParticipant(const UUID& UUID) override;
		virtual const Judoka* FindParticipant(const UUID& UUID) const override;

		//Match tables
		virtual MatchTable* FindMatchTable(const UUID& ID) override;
		virtual const MatchTable* FindMatchTable(const UUID& ID) const override;

		//Rule Sets
		virtual const RuleSet* FindRuleSet(const UUID& UUID) const override;
		virtual RuleSet* FindRuleSet(const UUID& UUID) override;

		//Events
		virtual void OnMatchStarted(const Match& Match) const { assert(false); }
		virtual void OnMatchConcluded(const Match& Match) const override;

	private:
		std::string Request2Master(const std::string& URL) const;
		bool Post2Master(const std::string& URL, const YAML::Emitter& Data) const;

		bool IsMatchInCache(const UUID& UUID) const;
		Match* FindInCache(const UUID& UUID) const;
		MatchTable* FindMatchTableInCache(const UUID& UUID) const;


		std::string m_Hostname;
		uint16_t m_Port;

		mutable std::vector<Match*> m_MatchCache;

		mutable std::vector<Match> m_NextMatches;
		mutable uint32_t m_NextMatches_Timestamp = 0;//Timestamp when m_NextMatches was received

		mutable std::vector<MatchTable*> m_MatchTables;

		mutable StandingData m_StandingData;//Local cache

		mutable ZED::RecursiveReadWriteMutex m_Mutex;
	};
}