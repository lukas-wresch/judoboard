#include <algorithm>
#include <random>
#include "weightclass.h"
#include "tournament.h"
#include "localizer.h"
#include "match.h"
#include "timer.h"



using namespace Judoboard;



Weightclass::Weightclass(const Tournament* Tournament, uint16_t MinWeight, uint16_t MaxWeight) : MatchTable(Tournament)
{
	m_MinWeight = MinWeight;
	m_MaxWeight = MaxWeight;

	SetName(Localizer::Translate("Weightclass") + " " + GetDescription());
}



Weightclass::Weightclass(const Tournament* Tournament, uint16_t MinWeight, uint16_t MaxWeight, Gender Gender) : Weightclass(Tournament, MinWeight, MaxWeight)
{
	m_Gender = Gender;
	m_GenderEnforced = true;

	SetName(Localizer::Translate("Weightclass") + " " + GetDescription());
}



Weightclass::Weightclass(ZED::CSV& Stream, const Tournament* Tournament) : MatchTable(Stream, Tournament)
{
	Stream >> m_MinWeight >> m_MaxWeight >> m_Gender >> m_GenderEnforced;
}



void Weightclass::operator >> (ZED::CSV& Stream) const
{
	MatchTable::operator >>(Stream);
	Stream << m_MinWeight << m_MaxWeight << m_Gender << m_GenderEnforced;
}



const std::string Weightclass::GetDescription() const
{
	std::string name = std::to_string(m_MinWeight) + " - " + std::to_string(m_MaxWeight) + " kg";
	if (m_GenderEnforced)
		name += (m_Gender == Gender::Male) ? " (m)" : " (f)";
	return name;
}



Status Weightclass::GetStatus() const
{
	if (m_Schedule.size() == 0)
		return Status::Scheduled;

	bool one_match_finished = false;
	bool all_matches_finished = true;

	for (auto match : m_Schedule)
	{
		if (!match->HasConcluded())
			all_matches_finished = false;

		if (match->IsRunning() || match->HasConcluded())
			one_match_finished = true;
	}

	if (all_matches_finished)
		return Status::Concluded;
	if (one_match_finished)
		return Status::Running;
	return Status::Scheduled;
}



std::string Weightclass::GetHTMLForm()
{
	std::string ret = R"(
<div>
    <label style="width:150px;float:left;margin-top:5px;" id="label_min_weight">Min Weight</label>
    <input style="margin-bottom:20px;" type="number" id="minWeight" value="" min="10" max="500" />
</div>

<div>
    <label style="width:150px;float:left;margin-top:5px;" id="label_max_weight">Max Weight</label>
    <input style="margin-bottom:20px;" type="number" id="maxWeight" value="" min="10" max="500" />
</div>

<div>
  <label style="width:150px;float:left;margin-top:5px;" id="label_gender">Gender</label>
  <select style="margin-bottom:10px;" id="gender">
    <option selected value="2" id="all">All</option>
    <option value="0" id="male">Male</option>
    <option value="1" id="female">Female</option>
  </select>
</div>
)";

	return ret;
}



bool Weightclass::IsElgiable(const Judoka& Fighter) const
{
	if (m_MinWeight > Fighter.GetWeight() || Fighter.GetWeight() > m_MaxWeight)
		return false;

	if (m_AgeEnforced)
		if (m_MinAge > Fighter.GetAge() || Fighter.GetAge() > m_MaxAge)
			return false;

	if (m_GenderEnforced)
		if (m_Gender != Fighter.GetGender())
			return false;

	return true;
}



void Weightclass::GenerateSchedule()
{
	m_Schedule.clear();

	if (GetParticipants().size() <= 3)
		m_RecommendedNumMatches_Before_Break = 1;
	else
		m_RecommendedNumMatches_Before_Break = 2;

	if (GetParticipants().size() == 4)
	{
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(0), GetParticipant(1), GetMatID()));
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(2), GetParticipant(3), GetMatID()));

		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(0), GetParticipant(2), GetMatID()));
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(1), GetParticipant(3), GetMatID()));

		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(0), GetParticipant(3), GetMatID()));
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(1), GetParticipant(2), GetMatID()));
	}

	else if (GetParticipants().size() == 5)
	{
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(0), GetParticipant(1), GetMatID()));
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(2), GetParticipant(3), GetMatID()));
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(0), GetParticipant(4), GetMatID()));

		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(1), GetParticipant(2), GetMatID()));
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(3), GetParticipant(4), GetMatID()));
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(0), GetParticipant(2), GetMatID()));

		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(1), GetParticipant(3), GetMatID()));
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(2), GetParticipant(4), GetMatID()));
		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(0), GetParticipant(3), GetMatID()));

		m_Schedule.push_back(new Match(GetTournament(), GetParticipant(1), GetParticipant(4), GetMatID()));
	}

	else
	{
		for (auto white = GetParticipants().cbegin(); white != GetParticipants().cend(); ++white)
			for (auto blue = white + 1; blue != GetParticipants().cend(); ++blue)
			{
				if (white != blue)
					m_Schedule.push_back(new Match(GetTournament(), *white, *blue, GetMatID()));
			}

		auto rng = std::default_random_engine{};
		std::shuffle(std::begin(m_Schedule), std::end(m_Schedule), rng);
	}

	for (auto match : m_Schedule)
		match->SetMatchTable(this);
}



const std::string Weightclass::ToHTML() const
{
	std::string ret;

	ret += "<a href=\"#matchtable_add.html?id=" + std::to_string(GetID()) + "\">" + GetName() + "</a><br/>";

	ret += GetDescription() + " / " + "Mat " + std::to_string(GetMatID()) + " / " + GetRuleSet().GetName() + "<br/>";

	ret += R"(<table width="50%" border="1" rules="all"><tr><th style="text-align: center;">No.</th><th style="width: 5.0cm;">Name</th>)";

	for (uint32_t j = 0; j < GetParticipants().size(); j++)//Number of fights + 1
		ret += "<th>vs " + GetParticipants()[j]->GetName() + "</th>";

	ret += "<th style=\"text-align: center; width: 2.0cm;\">Total</th>";
	ret += "</tr>";

	FighterScore* pScores = new FighterScore[GetParticipants().size()];

	for (size_t i = 0; i < GetParticipants().size();i++)
	{
		auto fighter = GetParticipant(i);

		if (!fighter)
			continue;

		ret += "<tr>";
		ret += "<td style=\"text-align: center;\">" + std::to_string(i+1) + "</td>";
		ret += "<td>" + fighter->GetName() + "<br/>(" + std::to_string(fighter->GetWeight()) + " kg)</td>";

		pScores[i].Set(fighter, this);

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
					ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + std::to_string(matches[0]->GetID()) + "\">In Progress</a></td>";
				else if (!matches[0]->HasConcluded())
					ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + std::to_string(matches[0]->GetID()) + "\">- - -</a></td>";
				else if (matches[0]->GetWinningJudoka()->GetID() == fighter->GetID())
				{
					pScores[i].Wins++;
					const auto& result = matches[0]->GetMatchResult();
					pScores[i].Score += (unsigned int)result.m_Score;
					pScores[i].Time  += result.m_Time;
					ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + std::to_string(matches[0]->GetID()) + "\">" + std::to_string((int)result.m_Score) + " (" + Timer::TimestampToString(result.m_Time) + ")</a></td>";
				}
				else
				{
					const auto& result = matches[0]->GetMatchResult();
					pScores[i].Time += result.m_Time;
					ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + std::to_string(matches[0]->GetID()) + "\">0 (" + Timer::TimestampToString(result.m_Time) + ")</a></td>";
				}
			}
		}

		ret += "<td style=\"text-align: center;\">" + std::to_string(pScores[i].Wins) + " : " + std::to_string(pScores[i].Score) + "<br/>(" + Timer::TimestampToString(pScores[i].Time) + ")</td>";

		ret += "</tr>";
	}



	ret += "</table><br/><br/><table border=\"1\" rules=\"all\">";
	ret += "<tr><th style=\"width: 0.5cm; text-align: center;\">#</th><th style=\"width: 5.0cm;\">Name</th><th style=\"width: 1.0cm;\">Wins</th><th style=\"width: 1.0cm;\">Score</th><th style=\"width: 1.3cm;\">Time</th></tr>";

	std::qsort(pScores, GetParticipants().size(), sizeof(FighterScore), MatchTable::CompareFighterScore);

	for (size_t i = 0; i < GetParticipants().size(); i++)
	{
		const auto& score = pScores[i];

		ret += "<tr><td style=\"text-align: center;\">" + std::to_string(i+1) + "</td>";
		ret += "<td>" + score.Judoka->GetName() + "</td>";

		ret += "<td>" + std::to_string(score.Wins)  + "</td>";
		ret += "<td>" + std::to_string(score.Score) + "</td>";
		ret += "<td>" + Timer::TimestampToString(score.Time);

		if (score.NotSortable && score.Time > 0)
			ret += " Not sortable!";//DEBUG

		ret += "</td>";

		ret += "</tr>";
	}

	ret += "</table>";
	delete[] pScores;

	return ret;
}



const std::string Weightclass::ToString() const
{
	ZED::CSV ret(MatchTable::ToString());
	ret << m_MinWeight << m_MaxWeight;

	if (!m_GenderEnforced)
		ret << 2;
	else
		ret << m_Gender;

	return ret;
}