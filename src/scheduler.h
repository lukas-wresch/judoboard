#pragma once
#include "judoboard.h"
#include "matchtable.h"



namespace Judoboard
{
	struct Scheduler
	{
		void Start(std::vector<const MatchTable*> Plan);

		std::vector<Match*> m_Schedule;
	};
}