#include <algorithm>
#include <random>
#include "weightclass.h"
#include "tournament.h"
#include "localizer.h"
#include "match.h"
#include "timer.h"



using namespace Judoboard;



Weightclass::Weightclass(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament)
	: MatchTable(Tournament)
{
	m_MinWeight = MinWeight;
	m_MaxWeight = MaxWeight;
}



Weightclass::Weightclass(Weight MinWeight, Weight MaxWeight, Gender Gender, const ITournament* Tournament)
	: Weightclass(MinWeight, MaxWeight, Tournament)
{
	m_Gender = Gender;
}



Weightclass::Weightclass(const YAML::Node& Yaml, ITournament* Tournament)
	: MatchTable(Yaml, Tournament)
{
	if (Yaml["min_weight"])
		m_MinWeight = Weight(Yaml["min_weight"]);
	if (Yaml["max_weight"])
		m_MaxWeight = Weight(Yaml["max_weight"]);
	if (Yaml["gender"])
		m_Gender = (Gender)Yaml["gender"].as<int>();
	if (Yaml["best_of_three"])
		m_BestOfThree = Yaml["best_of_three"].as<bool>();
}



Weightclass::Weightclass(const MD5::Weightclass& Weightclass, const ITournament* Tournament)
	: MatchTable(Tournament)
{
	SetName(Weightclass.Description);

	if (Weightclass.WeightLargerThan > 0)
		m_MinWeight = Weightclass.WeightLargerThan  * 1000 + Weightclass.WeightInGrammsLargerThan;
	if (Weightclass.WeightSmallerThan > 0)
		m_MaxWeight = Weightclass.WeightSmallerThan * 1000 + Weightclass.WeightInGrammsSmallerThan;

	if (Weightclass.AgeGroup)
		m_Gender = Weightclass.AgeGroup->Gender;
}



void Weightclass::operator >> (YAML::Emitter& Yaml) const
{
	MatchTable::operator >>(Yaml);

	Yaml << YAML::Key << "min_weight" << YAML::Value;
	m_MinWeight >> Yaml;
	Yaml << YAML::Key << "max_weight" << YAML::Value;
	m_MaxWeight >> Yaml;

	Yaml << YAML::Key << "gender"     << YAML::Value << (int)m_Gender;

	if (m_BestOfThree)
		Yaml << YAML::Key << "best_of_three" << YAML::Value << m_BestOfThree;
}



void Weightclass::ToString(YAML::Emitter& Yaml) const
{
	MatchTable::ToString(Yaml);

	Yaml << YAML::Key << "min_weight" << YAML::Value << m_MinWeight.ToString();
	Yaml << YAML::Key << "max_weight" << YAML::Value << m_MaxWeight.ToString();
	Yaml << YAML::Key << "gender"     << YAML::Value << (int)m_Gender;
	Yaml << YAML::Key << "best_of_three" << YAML::Value << m_BestOfThree;
}



std::string Weightclass::GetDescription() const
{
	std::string desc = GetName();

	if (desc.length() > 0)
	{
		if (GetAgeGroup() && GetAgeGroup()->GetGender() != Gender::Unknown)
			return GetAgeGroup()->GetName() + " " + desc;
		else if (GetAgeGroup())
			return GetAgeGroup()->GetName() + Localizer::Gender2ShortForm(m_Gender) + " " + desc;
		return desc;
	}

	if (GetAgeGroup())
	{
		desc = GetAgeGroup()->GetName() + Localizer::Gender2ShortForm(m_Gender);

		if ((uint32_t)m_MaxWeight == 0)
			desc += " +" + m_MinWeight.ToString() + " kg";
		else
			desc += " -" + m_MaxWeight.ToString() + " kg";
	}

	else
	{
		if ((uint32_t)m_MaxWeight == 0)
			desc = "+" + m_MinWeight.ToString() + " kg";
		else
			desc = m_MinWeight.ToString() + " - " + m_MaxWeight.ToString() + " kg";

		if (m_Gender != Gender::Unknown)
			desc += " (" + Localizer::Gender2ShortForm(m_Gender) + ")";
	}

	return desc;
}



std::string Weightclass::GetHTMLForm()
{
	std::string ret = R"(
<div>
    <label style="width:150px;float:left;margin-top:5px;" id="label_min_weight">Min Weight</label>
    <input style="margin-bottom:20px;" type="text" id="minWeight" value="" size="1" />
</div>

<div>
    <label style="width:150px;float:left;margin-top:5px;" id="label_max_weight">Max Weight</label>
    <input style="margin-bottom:20px;" type="text" id="maxWeight" value="" size="1" />
</div>

<div>
  <label style="width:150px;float:left;margin-top:5px;" id="label_gender">Gender</label>
  <select style="margin-bottom:20px;" id="gender">
    <option selected value="-1" id="all">All</option>
    <option value="0" id="male">Male</option>
    <option value="1" id="female">Female</option>
  </select>
</div>

<div>
  <label style="width:150px;float:left;margin-top:5px;" id="label_bo3">Best of 3</label>
  <input type="checkbox" id="bo3" class="switch-input">
  <label style="padding-top:0px;padding-bottom:0px;margin-top:5px;margin-bottom:20px;" class="switch-label" for="bo3">
    <span class="toggle-on" id="bo3_enabled"></span><span class="toggle-off" id="bo3_disabled"></span>
  </label>
</div>
)";

	return ret;
}



bool Weightclass::IsElgiable(const Judoka& Fighter) const
{
	//Is already in this weight class?
	/*for (auto judoka : GetParticipants())
		if (*judoka == Fighter)
			return false;*/

	if ((uint32_t)m_MaxWeight == 0)//No maximum weight
	{
		if (m_MinWeight > Fighter.GetWeight())
			return false;
	}

	else if (m_MinWeight > Fighter.GetWeight() || Fighter.GetWeight() > m_MaxWeight)
		return false;


	if (GetAgeGroup())
	{
		//Does the judoka belong in this age group?
		if (!GetAgeGroup()->IsElgiable(Fighter))
			return false;

		//Check if the judoka is indeed starting for that age group
		if (GetTournament())
		{
			auto age_group_starting_for = GetTournament()->GetAgeGroupOfJudoka(&Fighter);
			if (!age_group_starting_for || GetAgeGroup()->GetUUID() != age_group_starting_for->GetUUID())
				return false;
		}
	}

	if (m_Gender != Gender::Unknown)//Gender enforced?
		if (m_Gender != Fighter.GetGender())
			return false;

	return true;
}



void Weightclass::GenerateSchedule()
{
	for (auto it = m_Schedule.begin(); it != m_Schedule.end();)
	{
		if ((*it)->IsAutoGenerated())
			it = m_Schedule.erase(it);
		else
			++it;
	}

	if (GetParticipants().size() <= 3)
		m_RecommendedNumMatches_Before_Break = 1;
	else
		m_RecommendedNumMatches_Before_Break = 2;

	if (GetParticipants().size() == 2)
		AddAutoMatch(1, 0);

	else if (GetParticipants().size() == 3)
	{
		AddAutoMatch(2, 0);
		AddAutoMatch(1, 0);
		AddAutoMatch(2, 1);
	}

	else if (GetParticipants().size() == 4)
	{
		AddAutoMatch(2, 0);
		AddAutoMatch(3, 1);

		AddAutoMatch(3, 0);
		AddAutoMatch(2, 1);

		AddAutoMatch(1, 0);
		AddAutoMatch(3, 2);
	}

	else if (GetParticipants().size() == 5)
	{
		AddAutoMatch(3, 0);
		AddAutoMatch(4, 1);

		AddAutoMatch(2, 0);
		AddAutoMatch(3, 1);

		AddAutoMatch(4, 2);
		AddAutoMatch(1, 0);

		AddAutoMatch(3, 2);
		AddAutoMatch(4, 0);

		AddAutoMatch(2, 1);
		AddAutoMatch(4, 3);
	}

	else if (GetParticipants().size() == 6)
	{
		AddAutoMatch(3, 0);
		AddAutoMatch(4, 1);
		AddAutoMatch(5, 2);

		AddAutoMatch(3, 1);
		AddAutoMatch(4, 0);

		AddAutoMatch(5, 1);
		AddAutoMatch(2, 0);

		AddAutoMatch(5, 3);
		AddAutoMatch(4, 2);
		AddAutoMatch(1, 0);

		AddAutoMatch(4, 3);
		AddAutoMatch(5, 0);
		AddAutoMatch(2, 1);

		AddAutoMatch(5, 4);
		AddAutoMatch(3, 2);
	}

	else
	{
		for (size_t blue = 0; blue < GetParticipants().size(); ++blue)
			for (size_t white = blue + 1; white < GetParticipants().size(); ++white)
			{
				AddAutoMatch(white, blue);
			}

		auto rng = std::default_random_engine{};
		std::shuffle(std::begin(m_Schedule), std::end(m_Schedule), rng);
	}

	
	//Add additional matches for best of three
	if (m_BestOfThree)
	{
		auto length = m_Schedule.size();
		for (size_t i = 0; i < length; ++i)
		{
			auto match1 = m_Schedule[i];
			auto indices = GetParticipantIndicesOfMatch(match1);

			auto match2 = AddAutoMatch(indices.second, indices.first);
			auto match3 = AddAutoMatch(indices.first,  indices.second);
			if (match3)
				match3->SetBestOfThree(match1, match2);
		}
	}


	for (auto match : m_Schedule)
		match->SetMatchTable(this);
}



std::vector<MatchTable::Result> Weightclass::CalculateResults() const
{
	std::vector<Result> ret(GetParticipants().size());

	for (size_t i = 0; i < GetParticipants().size(); i++)
	{
		auto fighter = GetParticipant(i);
		ret[i].Set(fighter, this);		
	}

	for (const Match* match : m_Schedule)
	{
		if (!match->HasConcluded())
			continue;

		const auto& result = match->GetResult();

		auto i = GetIndexOfParticipant(match->GetWinner());
		auto j = GetIndexOfParticipant(match->GetLoser());

		ret[i].Wins++;
		ret[i].Score += (uint32_t)result.m_Score;

		ret[i].Time += result.m_Time;
		ret[j].Time += result.m_Time;
	}

	std::sort(ret.begin(), ret.end());

	return ret;
}



const std::string Weightclass::ToHTML() const
{
	std::string ret;

	ret += "<a href=\"#matchtable_add.html?id=" + (std::string)GetUUID() + "\">" + GetDescription() + "</a>";

	ret += " / " + Localizer::Translate("Mat") + " " + std::to_string(GetMatID()) + " / " + GetRuleSet().GetName() + "<br/>";

	ret += R"(<table width="50%" border="1" rules="all"><tr><th style="text-align: center;">)" + Localizer::Translate("No.")
		+ "</th><th style=\"width: 5.0cm;\">" + Localizer::Translate("Name") + "</th>";

	for (uint32_t j = 0; j < GetParticipants().size(); j++)//Number of fights + 1
		ret += "<th>vs " + GetParticipants()[j]->GetName(NameStyle::GivenName) + "</th>";

	ret += "<th style=\"text-align: center; width: 2.0cm;\">Total</th>";
	ret += "</tr>";

	auto results = CalculateResults();

	for (size_t i = 0; i < GetParticipants().size();i++)
	{
		auto fighter = GetParticipant(i);

		if (!fighter)
			continue;

		ret += "<tr>";
		ret += "<td style=\"text-align: center;\">" + std::to_string(i+1) + "</td>";
		ret += "<td>" + fighter->GetName(NameStyle::GivenName) + "<br/>(" + fighter->GetWeight().ToString() + " kg)</td>";

		for (size_t j = 0; j < GetParticipants().size(); j++)//Number of fights + 1
		{
			auto enemy = GetParticipant(j);
			if (!enemy)
				continue;

			auto matches = FindMatches(*fighter, *enemy);

			if (matches.empty())
				ret += "<td style=\"background-color: #ccc;\"></td>";
			else
			{
				if (matches[0]->IsRunning())
					ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + (std::string)matches[0]->GetUUID() + "\">In Progress</a></td>";
				else if (!matches[0]->HasConcluded())
					ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + (std::string)matches[0]->GetUUID() + "\">- - -</a></td>";
				else if (matches[0]->GetWinner()->GetUUID() == fighter->GetUUID())
				{
					const auto& result = matches[0]->GetResult();
					ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + (std::string)matches[0]->GetUUID() + "\">" + std::to_string((int)result.m_Score) + " (" + Timer::TimestampToString(result.m_Time) + ")</a></td>";
				}
				else
				{
					const auto& result = matches[0]->GetResult();
					ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + (std::string)matches[0]->GetUUID() + "\">0 (" + Timer::TimestampToString(result.m_Time) + ")</a></td>";
				}
			}
		}

		for (const auto& result : results)
		{
			if (result.Judoka && result.Judoka->GetUUID() == fighter->GetUUID())
				ret += "<td style=\"text-align: center;\">" + std::to_string(result.Wins) + " : " + std::to_string(result.Score) + "<br/>(" + Timer::TimestampToString(result.Time) + ")</td>";
		}

		ret += "</tr>";
	}

	ret += ResultsToHTML();

	return ret;
}