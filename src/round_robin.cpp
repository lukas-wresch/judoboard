#include <algorithm>
#include <random>
#include "round_robin.h"
#include "weightclass.h"
#include "tournament.h"
#include "localizer.h"
#include "match.h"
#include "timer.h"



using namespace Judoboard;



RoundRobin::RoundRobin(std::shared_ptr<IFilter> Filter, const ITournament* Tournament)
	: MatchTable(Filter, Tournament)
{
	if (Filter)
		SetTournament(Filter->GetTournament());
	//GenerateSchedule();
}



RoundRobin::RoundRobin(Weight MinWeight, Weight MaxWeight, Gender Gender, const ITournament* Tournament)
	: MatchTable(std::make_shared<Weightclass>(MinWeight, MaxWeight, Gender, this), Tournament)
{
}



RoundRobin::RoundRobin(const YAML::Node& Yaml, const ITournament* Tournament, const MatchTable* Parent)
	: MatchTable(Yaml, Tournament, Parent)
{
}



RoundRobin::RoundRobin(const MD5::Weightclass& Weightclass_, const ITournament* Tournament)
	: MatchTable(std::make_shared<Weightclass>(Weightclass_, this), Tournament)
{
	SetName(Weightclass_.Description);
}



std::string RoundRobin::GetHTMLForm()
{
	return MatchTable::GetHTMLForm();
}



void RoundRobin::GenerateSchedule()
{
	if (!IsAutoGenerateSchedule())
		return;
	if (!IsSubMatchTable() && GetStatus() != Status::Scheduled)
		return;

	DeleteSchedule();

	if (GetParticipants().size() <= 3)
		m_RecommendedNumMatches_Before_Break = 1;
	else if (GetParticipants().size() >= 7)
		m_RecommendedNumMatches_Before_Break = 3;
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
		auto& schedule = SetSchedule();

		for (size_t blue = 0; blue < GetMaxStartPositions(); ++blue)
			for (size_t white = blue + 1; white < GetMaxStartPositions(); ++white)
			{
				AddAutoMatch(white, blue);
			}

		auto rng = std::default_random_engine{};
		std::shuffle(std::begin(schedule), std::end(schedule), rng);
	}

	
	//Add additional matches for best of three
	if (IsBestOfThree())
		AddMatchesForBestOfThree();
}



MatchTable::Results RoundRobin::CalculateResults() const
{
	MatchTable::Results results(*this);

	for (auto match : GetSchedule())
	{
		//Don't count skipped matches (they still count as concluded)
		if (match->GetStatus() == Status::Skipped)
			continue;
		if (!match->HasConcluded())
			continue;

		const auto& result = match->GetResult();

		auto winner = results.GetResultsOf(match->GetWinner());

		if (winner)
		{
			winner->Wins++;
			winner->Score += (uint32_t)result.m_Score;
			winner->Time  += result.m_Time;
		}
	}

	results.Sort();

	return results;
}



void RoundRobin::operator >> (YAML::Emitter& Yaml) const
{
	if (!IsSubMatchTable())
		Yaml << YAML::BeginMap;

	MatchTable::operator >>(Yaml);

	if (!IsSubMatchTable())
		Yaml << YAML::EndMap;
}



const std::string RoundRobin::ToHTML() const
{
	std::string ret = GetHTMLTop();

	if (!GetFilter())
		return ret;

	if (!GetSchedule().empty())
	{
		ret += R"(<table width="50%" border="1" rules="all"><tr><th style="text-align: center;">)" + Localizer::Translate("No.")
			+ "</th><th style=\"width: 5.0cm;\">" + Localizer::Translate("Name") + "</th>";

		for (size_t i = 0; i < GetMaxStartPositions(); ++i)
		{
			auto fighter = GetJudokaByStartPosition(i);

			if (fighter)
				ret += "<th>vs " + fighter->GetName(NameStyle::GivenName) + "</th>";
		}

		ret += "<th style=\"text-align: center; width: 2.0cm;\">Total</th>";
		ret += "</tr>";

		auto results = CalculateResults();

		for (size_t i = 0; i < GetMaxStartPositions(); ++i)
		{
			auto fighter = GetJudokaByStartPosition(i);

			if (!fighter)
				continue;

			ret += "<tr>";
			ret += "<td style=\"text-align: center;\">" + std::to_string(i + 1) + "</td>";
			ret += "<td>" + fighter->GetName(NameStyle::GivenName) + "<br/>(" + fighter->GetWeight().ToString() + " kg)</td>";

			for (size_t j = 0; j < GetMaxStartPositions(); ++j)
			{
				auto enemy = GetJudokaByStartPosition(j);
				if (!enemy)
					continue;

				auto matches = FindMatches(*fighter, *enemy);

				if (matches.empty())
					ret += "<td style=\"background-color: #ccc;\"></td>";
				else
					ret += "<td style=\"text-align: center;\">";

				for (auto match : matches)
				{
					if (match->IsRunning())
						ret += "<a href=\"#edit_match.html?id=" + (std::string)match->GetUUID() + "\">" + Localizer::Translate("Running") + "</a><br/>";
					else if (!match->HasConcluded())
						ret += "<a href=\"#edit_match.html?id=" + (std::string)match->GetUUID() + "\">- - -</a><br/>";
					else if (match->GetWinner()->GetUUID() == fighter->GetUUID())
					{
						const auto& result = match->GetResult();
						if ((int)result.m_Score > 0)
							ret += "<a href=\"#edit_match.html?id=" + (std::string)match->GetUUID() + "\">" + std::to_string((int)result.m_Score) + " (" + Timer::TimestampToString(result.m_Time) + ")</a><br/>";
					}
					else
					{
						const auto& result = match->GetResult();
						if ((int)result.m_Score > 0)
							ret += "<a href=\"#edit_match.html?id=" + (std::string)match->GetUUID() + "\">0 (" + Timer::TimestampToString(result.m_Time) + ")</a><br/>";
					}
				}

				ret += "</td>";
			}

			const auto result = results.GetResultsOf(fighter);
			if (result)
				ret += "<td style=\"text-align: center;\">" + std::to_string(result->Wins) + " : " + std::to_string(result->Score) + "<br/>(" + Timer::TimestampToString(result->Time) + ")</td>";

			ret += "</tr>";
		}

		ret += "</table>";
		ret += ResultsToHTML();
	}

	if (!IsSubMatchTable())
		ret += "</div>";

	return ret;
}