#include <algorithm>
#include "../ZED/include/log.h"
#include "weightclass.h"
#include "weightclass_generator.h"



using namespace Judoboard;



int Generator::calculateMedian(std::vector<std::pair<int, int>>& v, int Start, int End)
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



bool Generator::split(std::vector<std::pair<int, int>>& WeightsSlots, int Start, int End)
{
	static int next_weight_id = 1;

	int median = calculateMedian(WeightsSlots, Start, End);
	int count = End-Start;

	int min = 1000;
	int max = 0;
	for (int i = Start; i < End; ++i)
	{
		if (WeightsSlots[i].first < min)
			min = WeightsSlots[i].first;
		if (WeightsSlots[i].first > max)
			max = WeightsSlots[i].first;
	}

	bool need_split = count > m_Max;//Must split
	need_split |= (count >= 2 * m_Min) && (max - min > m_Diff);

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