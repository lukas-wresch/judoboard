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
			new_desc.m_Max = weight_max;
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

		if (desc.m_Max != 0)
			name += desc.m_Min.ToString() + " - " + desc.m_Max.ToString() + " kg";
		else
			name += desc.m_Min.ToString() + "+ kg";
		Yaml << YAML::Key << "name" << YAML::Value << name;

		Yaml << YAML::Key << "num_participants" << YAML::Value << desc.m_NumParticipants;
		Yaml << YAML::EndMap;
	}
}



bool Generator::split(std::vector<std::pair<Weight, int>>& WeightsSlots, int Start, int End)
{
	static int next_weight_id = 1;

	if (Start > End)
		return false;

	Weight median = calculateMedian(WeightsSlots, Start, End);
	unsigned int count  = End-Start;

	Weight min = 1000 * 1000;
	Weight max = 0;
	for (int i = Start; i < End; ++i)
	{
		if (WeightsSlots[i].first < min)
			min = WeightsSlots[i].first;
		if (WeightsSlots[i].first > max)
			max = WeightsSlots[i].first;
	}

	bool need_split = count > m_Max;//Must split
	need_split |= (count >= 2 * m_Min) && (max - min > m_Diff * 1000);

	if (need_split)
	{
		int Middle = Start;
		for (int i = Start; i < End; ++i)
		{
			if (WeightsSlots[i].first < median)
			{
				WeightsSlots[i].second = next_weight_id;
				Middle = i;
			}
			else
				WeightsSlots[i].second = next_weight_id + 1;
		}

		next_weight_id += 2;

		split(WeightsSlots, Start, Middle + 1);
		split(WeightsSlots, Middle + 1, End);
	}

	return need_split;
}