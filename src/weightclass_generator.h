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
	struct Generator
	{
		unsigned int m_Min  = 1;
		unsigned int m_Max  = 5;
		unsigned int m_Diff = 10;

		static Weight calculateMedian(std::vector<std::pair<Weight, int>>& v, int Start, int End);
		bool split(std::vector<std::pair<Weight, int>>& WeightsSlots, int Start, int End);

		std::vector<std::pair<Weight, int>> split(const std::vector<int>& Weights)
		{
			std::vector<std::pair<Weight, int>> weightsSlots;
			for (auto weight : Weights)
				weightsSlots.emplace_back(weight, 0);

			split(weightsSlots, 0, (int)weightsSlots.size());
			return weightsSlots;
		}
	};
}