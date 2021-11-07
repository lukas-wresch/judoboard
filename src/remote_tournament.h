#pragma once
#include "tournament.h"



namespace Judoboard
{
	class RemoteTournament : public Tournament
	{
	public:
		friend class Application;

		RemoteTournament(const std::string& Host, uint16_t Port);

	private:
		std::string m_Hostname;
		uint16_t m_Port;
	};
}