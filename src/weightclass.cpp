#include <algorithm>
#include <random>
#include "weightclass.h"
#include "tournament.h"
#include "localizer.h"
#include "match.h"
#include "timer.h"



using namespace Judoboard;



Weightclass::Weightclass(Weight MinWeight, Weight MaxWeight, const MatchTable* Parent)
	: IFilter(Parent)
{
	m_MinWeight = MinWeight;
	m_MaxWeight = MaxWeight;
}



Weightclass::Weightclass(Weight MinWeight, Weight MaxWeight, Gender Gender, const MatchTable* Parent)
	: Weightclass(MinWeight, MaxWeight, Parent)
{
	m_Gender = Gender;
}



Weightclass::Weightclass(const YAML::Node& Yaml, const MatchTable* Parent)
	: IFilter(Yaml, Parent)
{
	if (Yaml["min_weight"])
		m_MinWeight = Weight(Yaml["min_weight"]);
	if (Yaml["max_weight"])
		m_MaxWeight = Weight(Yaml["max_weight"]);
	if (Yaml["gender"])
		m_Gender = (Gender)Yaml["gender"].as<int>();
}



Weightclass::Weightclass(const MD5::Weightclass& Weightclass, const MatchTable* Parent)
	: IFilter(Parent)
{
	if (Weightclass.WeightLargerThan > 0)
		m_MinWeight = Weightclass.WeightLargerThan  * 1000 + Weightclass.WeightInGrammsLargerThan;
	if (Weightclass.WeightSmallerThan > 0)
		m_MaxWeight = Weightclass.WeightSmallerThan * 1000 + Weightclass.WeightInGrammsSmallerThan;

	if (Weightclass.AgeGroup)
		m_Gender = Weightclass.AgeGroup->Gender;
}



void Weightclass::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	IFilter::operator>>(Yaml);

	Yaml << YAML::Key << "min_weight" << YAML::Value;
	m_MinWeight >> Yaml;
	Yaml << YAML::Key << "max_weight" << YAML::Value;
	m_MaxWeight >> Yaml;

	Yaml << YAML::Key << "gender"     << YAML::Value << (int)m_Gender;

	Yaml << YAML::EndMap;
}



void Weightclass::ToString(YAML::Emitter& Yaml) const
{
	IFilter::ToString(Yaml);
	Yaml << YAML::Key << "min_weight" << YAML::Value << m_MinWeight.ToString();
	Yaml << YAML::Key << "max_weight" << YAML::Value << m_MaxWeight.ToString();
	Yaml << YAML::Key << "gender"     << YAML::Value << (int)m_Gender;
}



std::string Weightclass::GetDescription() const
{
	std::string desc;

	if (GetAgeGroup())
	{
		if (GetAgeGroup()->GetGender() != Gender::Unknown)
			desc = GetAgeGroup()->GetName();
		else
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
		else if ((uint32_t)m_MinWeight == 0)
			desc = "- " + m_MaxWeight.ToString() + " kg";
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
)";

	return ret;
}



bool Weightclass::IsElgiable(const Judoka& Fighter) const
{
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