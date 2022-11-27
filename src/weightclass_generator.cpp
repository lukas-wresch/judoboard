#include <algorithm>
#include "../ZED/include/log.h"
#include "weightclass_generator.h"
#include "weightclass.h"
#include "age_group.h"
#include "localizer.h"



using namespace Judoboard;



Weight Generator::calculateMedian(std::vector<std::pair<Weight, int>>& v, int Start, int End)
{
	if (End <= Start)
		return 0;

	else if ((End-Start) % 2 == 0)
	{
		size_t n = Start + (End-Start) / 2 - 1;
		return (v[n].first + v[n+1].first) / 2;
	}
	else
	{
		size_t n = Start + (End-Start) / 2;
		return v[n].first;
	}
}



WeightclassDescCollection::WeightclassDescCollection(std::vector<std::pair<Weight, int>>& WeightSlots)
{
	int currentClass = -1;
	Weight weight_min = 0;
	Weight weight_max = 0;
	int judoka_count = 0;
	for (auto [weight, weightclass] : WeightSlots)
	{
		//New weightclass
		if (currentClass != weightclass && currentClass != -1)
		{
			weight_max = weight;

			//Close weightclass
			WeightclassDesc new_desc;
			new_desc.m_Min = weight_min;
			new_desc.m_Max = weight_max - Weight("0,001");
			new_desc.m_NumParticipants = judoka_count;
			m_Collection.emplace_back(new_desc);

			weight_min = weight;
			judoka_count = 0;
		}

		judoka_count++;
		currentClass = weightclass;
	}

	//Close weightclass
	WeightclassDesc new_desc;
	new_desc.m_Min = weight_min;
	new_desc.m_Max = 0;
	new_desc.m_NumParticipants = judoka_count;
	m_Collection.emplace_back(new_desc);
}



void WeightclassDescCollection::ToString(YAML::Emitter& Yaml) const
{
	for (const auto& desc : m_Collection)
	{
		//Generate yaml output

		Yaml << YAML::BeginMap;
		Yaml << YAML::Key << "min" << YAML::Value << desc.m_Min.ToString();
		Yaml << YAML::Key << "max" << YAML::Value << desc.m_Max.ToString();

		std::string name;
		if (m_AgeGroup)
			name += m_AgeGroup->GetName() + " ";
		if (m_Gender != Gender::Unknown)
			name += "(" + Localizer::Gender2ShortForm(m_Gender) + ") ";

		if ((uint32_t)desc.m_Max != 0)
			name += desc.m_Min.ToString() + " - " + desc.m_Max.ToString() + " kg";
		else
			name += desc.m_Min.ToString() + "+ kg";
		Yaml << YAML::Key << "name" << YAML::Value << name;

		Yaml << YAML::Key << "num_participants" << YAML::Value << desc.m_NumParticipants;
		Yaml << YAML::EndMap;
	}
}



Weight Generator::getSpread(std::vector<std::pair<Weight, int>>& WeightsSlots, int Start, int End)
{
	Weight min = 1000 * 1000;
	Weight max = 0;
	
	for (int i = Start; i < End; ++i)
	{
		if (WeightsSlots[i].first < min)
			min = WeightsSlots[i].first;
		if (WeightsSlots[i].first > max)
			max = WeightsSlots[i].first;
	}

	return max - min;
}



bool Generator::isGroupOK(std::vector<std::pair<Weight, int>>& WeightsSlots, int Start, int End)
{
	const auto spread = getSpread(WeightsSlots, Start, End);
	const unsigned int count  = End-Start;

	const bool must_split = count > m_Max;//Must split
	const bool should_split = must_split || ((count >= 2 * m_Min) && (spread > Weight(m_Diff)));

	return !should_split;
}



bool Generator::split(std::vector<std::pair<Weight, int>>& WeightsSlots, int Start, int End)
{
	static int next_weight_id = 1;

	if (Start > End)
		return false;

	Weight median = calculateMedian(WeightsSlots, Start, End);
	unsigned int count = End-Start;

	Weight min = 1000 * 1000;
	Weight max = 0;
	int Middle = Start;
	for (int i = Start; i < End; ++i)
	{
		if (WeightsSlots[i].first < min)
			min = WeightsSlots[i].first;
		if (WeightsSlots[i].first > max)
			max = WeightsSlots[i].first;

		if (WeightsSlots[i].first < median)
			Middle = i;
	}
		

	bool must_split = count > m_Max;//Must split
	bool should_split = must_split || ((count >= 2 * m_Min) && (max - min > Weight(m_Diff)));
	//Don't split if the result is too tiny, unless we must
	bool can_split = (Middle + 1 - Start >= (int)m_Min) && (End - Middle - 1 >= (int)m_Min);

	if (must_split || (should_split && can_split))
	{
#ifdef _DEBUG
		std::string line;
		for (int i = Start; i < End; ++i)
			line += std::to_string((uint32_t)WeightsSlots[i].first) + " ";
		ZED::Log::Debug(line);
		ZED::Log::Debug("Median: " + std::to_string((uint32_t)median));
		ZED::Log::Debug("Spread: " + std::to_string((uint32_t)(max - min)));
		ZED::Log::Debug(std::string("Must split: ")   + (must_split   ? "Yes" : "No"));
		ZED::Log::Debug(std::string("Should split: ") + (should_split ? "Yes" : "No"));
		ZED::Log::Debug(std::string("Can split: ")    + (can_split    ? "Yes" : "No"));
		ZED::Log::Debug("Will split!");
		ZED::Log::Debug("Middle: " + std::to_string(Middle));
		ZED::Log::Debug("Left#:  " + std::to_string(Middle - Start + 1));
		ZED::Log::Debug("Right#: " + std::to_string(End - Middle-1));
		ZED::Log::Debug("- - -");
#endif

		bool moved_for_complete_group   = false;
		bool moved_for_spread_reduction = false;

		//Can we move 'Middle' to the left, if we can split of a complete group?

		if (Middle - 1 > Start && !isGroupOK(WeightsSlots, Start, Middle + 1)
			                   &&  isGroupOK(WeightsSlots, Start, Middle))
		{
			Middle--;
			moved_for_complete_group = true;

			ZED::Log::Debug("Moved 'Middle' to the left to split a group of!");
			ZED::Log::Debug("Middle: " + std::to_string(Middle));
		}
		else if (Middle + 1 < End && !isGroupOK(WeightsSlots, Middle + 1, End)
			                      &&  isGroupOK(WeightsSlots, Middle + 2, End))
		{
			Middle++;
			moved_for_complete_group = true;

			ZED::Log::Debug("Moved 'Middle' to the right to split a group of!");
			ZED::Log::Debug("Middle: " + std::to_string(Middle));
		}


		if (!moved_for_complete_group && count >= 5)
		{
			//Can we move the 'Middle' to reduce the spread?
			auto spread_left  = getSpread(WeightsSlots, Start, Middle + 1);
			auto spread_right = getSpread(WeightsSlots, Middle, End);
			auto spread_now   = std::max(spread_left, spread_right);

			
			if (Middle > Start + 1)
			{
				auto spread_left_new  = getSpread(WeightsSlots, Start, Middle);
				auto spread_right_new = getSpread(WeightsSlots, Middle - 1, End);
				auto spread_new       = std::max(spread_left_new, spread_right_new);

				if (spread_new < spread_now)//Spread got reduced
				{
					Middle--;
					moved_for_spread_reduction = true;

					ZED::Log::Debug("Moved 'Middle' to the left to reduce spread!");
					ZED::Log::Debug("Middle: " + std::to_string(Middle));
				}
			}

			if (!moved_for_spread_reduction && End - 1 > Middle)
			{
				auto spread_left_new  = getSpread(WeightsSlots, Start, Middle + 2);
				auto spread_right_new = getSpread(WeightsSlots, Middle + 1, End);
				auto spread_new       = std::max(spread_left_new, spread_right_new);

				if (spread_new < spread_now)//Spread got reduced
				{
					Middle++;
					moved_for_spread_reduction = true;

					ZED::Log::Debug("Moved 'Middle' to the right to reduce spread!");
					ZED::Log::Debug("Middle: " + std::to_string(Middle));
				}
			}
		}


		//Perform splitting!
		for (int i = Start; i < End; ++i)
		{
			if (i <= Middle)
				WeightsSlots[i].second = next_weight_id;
			else
				WeightsSlots[i].second = next_weight_id + 1;
		}

		next_weight_id += 2;


		split(WeightsSlots, Start, Middle + 1);
		split(WeightsSlots, Middle + 1, End);
		return true;
	}

#ifdef _DEBUG
	else
	{
		std::string line;
		for (int i = Start; i < End; ++i)
			line += std::to_string((uint32_t)WeightsSlots[i].first) + " ";
		ZED::Log::Debug(line);
		ZED::Log::Debug("Median: " + std::to_string((uint32_t)median));
		ZED::Log::Debug("Spread: " + std::to_string((uint32_t)(max - min)));
		ZED::Log::Debug(std::string("Must split: ")   + (must_split   ? "Yes" : "No"));
		ZED::Log::Debug(std::string("Should split: ") + (should_split ? "Yes" : "No"));
		ZED::Log::Debug(std::string("Can split: ")    + (can_split    ? "Yes" : "No"));
		ZED::Log::Debug("Middle: " + std::to_string(Middle));
		ZED::Log::Debug("Left#:  " + std::to_string(Middle - Start + 1));
		ZED::Log::Debug("Right#: " + std::to_string(End - Middle-1));
		ZED::Log::Debug("Group complete!");
		ZED::Log::Debug("- - -");
	}
#endif

	return false;
}