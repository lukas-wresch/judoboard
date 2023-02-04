#pragma once
#include "itournament.h"



namespace Judoboard
{
	class RemoteTournament : public ITournament
	{
	public:
		friend class Application;

		RemoteTournament(const std::string& Host, uint16_t Port);

		virtual std::vector<Match> GetNextMatches(uint32_t MatID) const override;

		virtual bool IsParticipant(const Judoka& Judoka) const override;

		virtual Judoka* FindParticipant(const UUID& UUID) override;
		virtual const Judoka* FindParticipant(const UUID& UUID) const override;

		//Events
		virtual void OnMatchConcluded(const Match& Match) const override;

	private:
		std::string Request2Master(const std::string& URL) const;
		bool Post2Master(const std::string& URL, const YAML::Emitter& Data) const;

		std::string m_Hostname;
		uint16_t m_Port;

		mutable StandingData m_StandingData;//Local cache
	};
}