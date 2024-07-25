#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "judoboard.h"



namespace Judoboard
{
	class AgeGroup;



	struct WeightclassDesc
	{
		Weight m_Min = 0;
		Weight m_Max = 0;

		size_t m_NumParticipants = 0;
	};



	struct WeightclassDescCollection
	{
		WeightclassDescCollection(std::vector<std::pair<Weight, int>>& WeightSlots);
		void ToString(YAML::Emitter& Yaml) const;

		std::vector<WeightclassDesc> m_Collection;

		std::shared_ptr<const AgeGroup> m_AgeGroup;
		Gender m_Gender = Gender::Unknown;
	};



	struct Generator
	{
		unsigned int m_Min  = 1;
		unsigned int m_Max  = 5;
		unsigned int m_Diff = 10;

		static Weight calculateMedian(std::vector<std::pair<Weight, int>>& v, int Start, int End);

		Weight getSpread(std::vector<std::pair<Weight, int>>& WeightsSlots, int Start, int End);
		bool isGroupOK(std::vector<std::pair<Weight, int>>& WeightsSlots, int Start, int End);
		bool split(std::vector<std::pair<Weight, int>>& WeightsSlots, int Start, int End);

		WeightclassDescCollection split(const std::vector<Weight>& Weights)
		{
			std::vector<std::pair<Weight, int>> weightsSlots;
			for (auto weight : Weights)
				weightsSlots.emplace_back(weight, 0);

			split(weightsSlots, 0, (int)weightsSlots.size());

			WeightclassDescCollection ret(weightsSlots);
			return ret;
		}
	};
}