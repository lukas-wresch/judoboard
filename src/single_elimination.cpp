#include <algorithm>
#include <random>
#include "single_elimination.h"
#include "weightclass.h"
#include "tournament.h"
#include "localizer.h"
#include "match.h"



using namespace Judoboard;



SingleElimination::SingleElimination(IFilter* Filter, const ITournament* Tournament, const MatchTable* Parent)
	: MatchTable(Filter, Tournament, Parent)
{
	if (Filter)
		SetTournament(Filter->GetTournament());
	GenerateSchedule();
}



SingleElimination::SingleElimination(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament)
	: MatchTable(new Weightclass(MinWeight, MaxWeight, this), Tournament)
{
}



SingleElimination::SingleElimination(const YAML::Node& Yaml, const ITournament* Tournament, const MatchTable* Parent)
	: MatchTable(Yaml, Tournament, Parent)
{
	assert(Yaml.IsMap());

	if (Yaml["third_place_match"])
		m_ThirdPlaceMatch = Yaml["third_place_match"].as<bool>();
	if (Yaml["fifth_place_match"])
		m_FifthPlaceMatch = Yaml["fifth_place_match"].as<bool>();

	auto& schedule = SetSchedule();
	for (auto it = schedule.begin(); it != schedule.end();)
	{
		if ((*it)->GetTag().third)
		{
			m_ThirdPlaceMatches.emplace_back(*it);
			it = schedule.erase(it);
		}
		else if ((*it)->GetTag().fifth)
		{
			m_FifthPlaceMatches.emplace_back(*it);
			it = schedule.erase(it);
		}
		else
			++it;
	}
}



void SingleElimination::operator >> (YAML::Emitter& Yaml) const
{
	if (!IsSubMatchTable())
		Yaml << YAML::BeginMap;

	MatchTable::operator >>(Yaml);

	if (m_ThirdPlaceMatch)
		Yaml << YAML::Key << "third_place_match" << YAML::Value << m_ThirdPlaceMatch;
	if (m_FifthPlaceMatch)
		Yaml << YAML::Key << "fifth_place_match" << YAML::Value << m_FifthPlaceMatch;

	if (!IsSubMatchTable())
		Yaml << YAML::EndMap;
}



void SingleElimination::ToString(YAML::Emitter& Yaml) const
{
	MatchTable::ToString(Yaml);

	Yaml << YAML::Key << "third_place" << YAML::Value << IsThirdPlaceMatch();
	Yaml << YAML::Key << "fifth_place" << YAML::Value << IsFifthPlaceMatch();
}



size_t SingleElimination::GetNumberOfRounds() const
{
	if (GetSchedule().size() >= 3 && GetSchedule().size() <= 5)
	{
		auto count = GetSchedule().size();
		if (IsThirdPlaceMatch())
			count--;
		if (IsFifthPlaceMatch())
			count--;
		return (size_t)std::ceil(std::log2(count + 1));
	}

	if (!GetFilter() || GetFilter()->GetParticipants().size() == 0)
		return 0;

	return (size_t)std::ceil(std::log2(GetFilter()->GetParticipants().size()));
}



const std::vector<Match*> SingleElimination::GetSchedule() const
{
	std::vector<Match*> ret = MatchTable::GetSchedule();

	//Move last match to the end
	Match* final_match = nullptr;
	if (!ret.empty())
	{
		final_match = ret.back();
		ret.pop_back();
	}

	for (auto match : m_FifthPlaceMatches)
		ret.emplace_back(match);
	for (auto match : m_ThirdPlaceMatches)
		ret.emplace_back(match);

	if (final_match)
		ret.push_back(final_match);

	return ret;
}



std::string SingleElimination::GetHTMLForm()
{
	std::string ret = R"(
<div>
	<label style="width:150px;float:left;margin-top:5px;" id="label_mf3">Match for 3rd place</label>
		<input type="checkbox" id="mf3" class="switch-input">
		<label style="padding-top:0px;padding-bottom:0px;margin-top:5px;margin-bottom:20px;" class="switch-label" for="mf3">
		<span class="toggle-on" id="mf3_enabled"></span><span class="toggle-off" id="mf3_disabled"></span>
	</label>
</div>

<div>
	<label style="width:150px;float:left;margin-top:5px;" id="label_mf5">Match for 5th place</label>
		<input type="checkbox" id="mf5" class="switch-input">
		<label style="padding-top:0px;padding-bottom:0px;margin-top:5px;margin-bottom:20px;" class="switch-label" for="mf5">
		<span class="toggle-on" id="mf5_enabled"></span><span class="toggle-off" id="mf5_disabled"></span>
	</label>
</div>
)";

	ret += MatchTable::GetHTMLForm();

	return ret;
}



void SingleElimination::GenerateSchedule()
{
	if (!IsAutoGenerateSchedule())
		return;
	if (!IsSubMatchTable() && GetStatus() != Status::Scheduled)
		return;

	DeleteSchedule();

	if (!GetFilter())
		return;

	if (GetFilter()->GetParticipants().size() <= 3)
		m_RecommendedNumMatches_Before_Break = 1;
	else
		m_RecommendedNumMatches_Before_Break = 2;

	if (GetFilter()->GetParticipants().size() <= 1)
		return;

	const auto rounds = GetNumberOfRounds();
	const auto max_start_pos = GetMaxStartPositions();
	
	//Round 1
	std::vector<Match*> lastRound;
	std::vector<Match*> nextRound;

	auto create_pair = [&](int i) {
		auto new_match = CreateAutoMatch(GetFilter()->GetJudokaByStartPosition(i-1),
										 GetFilter()->GetJudokaByStartPosition(i-1 + max_start_pos/2));
		nextRound.emplace_back(new_match);
	};

	if (rounds == 2)//4 participants hardcoded
	{
		for (int j = 0; j < 2; j++)
			create_pair(1 + j);

		SetSchedule()[0]->SetTag(Match::Tag::Semi());
		SetSchedule()[1]->SetTag(Match::Tag::Semi());
	}

	else if (rounds == 3)//8 participants hardcoded
	{
		for (int j = 0; j < 2; j++)
		{
			create_pair(1 + j);
			create_pair(3 + j);
		}
	}

	else if (rounds == 4)//16 participants hardcoded
	{
		for (int j = 0; j < 2; j++)
		{
			create_pair(1 + j);
			create_pair(5 + j);
			create_pair(3 + j);
			create_pair(7 + j);
		}
	}

	else if (rounds == 5)//32 participants hardcoded
	{
		for (int j = 0; j < 2; j++)
		{
			create_pair(1  + j);
			create_pair(9  + j);
			create_pair(5  + j);
			create_pair(13 + j);
			create_pair(3  + j);
			create_pair(11 + j);
			create_pair(7  + j);
			create_pair(15 + j);
		}
	}

	else if (rounds == 6)//64 participants hardcoded
	{
		for (int j = 0; j < 2; j++)
		{
			create_pair(1  + j);
			create_pair(17 + j);
			create_pair(9  + j);
			create_pair(25 + j);
			create_pair(5  + j);
			create_pair(21 + j);
			create_pair(13 + j);
			create_pair(29 + j);
			create_pair(3  + j);
			create_pair(19 + j);
			create_pair(11 + j);
			create_pair(27 + j);
			create_pair(7  + j);
			create_pair(23 + j);
			create_pair(15 + j);
			create_pair(31 + j);
		}
	}

	else
	{
		for (size_t i = 0; i < max_start_pos/2; ++i)
		{
			auto new_match = CreateAutoMatch(GetFilter()->GetJudokaByStartPosition(i),
											 GetFilter()->GetJudokaByStartPosition(i + max_start_pos/2));
			nextRound.emplace_back(new_match);
		}
	}

	//Additional rounds
	for (int round = 1; round < rounds; ++round)
	{
		lastRound = std::move(nextRound);

		for (size_t i = 0; i < lastRound.size(); i += 2)
		{
			if (i+1 >= lastRound.size())
				break;

			auto match1 = lastRound[i];
			auto match2 = lastRound[i+1];

			auto new_match = AddMatchForWinners(match1, match2);

			if (round == rounds - 1)//Last round
				new_match->SetTag(Match::Tag::Finals());
			else if (round == rounds - 2)
				new_match->SetTag(Match::Tag::Semi());

			nextRound.emplace_back(new_match);
		}
	}


	auto& schedule = SetSchedule();

	//Add additional match for 3rd place
	if (IsThirdPlaceMatch() && schedule.size() >= 3)
	{
		auto match1 = schedule[schedule.size() - 3];
		auto match2 = schedule[schedule.size() - 2];

		//auto third_place = CreateAutoMatch(nullptr, nullptr);
		//third_place->SetDependency(Fighter::White, DependencyType::TakeLoser, match1);
		//third_place->SetDependency(Fighter::Blue,  DependencyType::TakeLoser, match2);

		Match* third_place = new Match(DependentJudoka(DependencyType::TakeLoser, *match1),
									   DependentJudoka(DependencyType::TakeLoser, *match2),
									   GetTournament(), GetMatID());

		third_place->SetMatchTable(this);
		third_place->SetTag(Match::Tag::Third());

		m_ThirdPlaceMatches.emplace_back(third_place);

		//Swap matches so that match for 1st place is still the last one
		//std::swap(schedule[schedule.size() - 1], schedule[schedule.size() - 2]);
	}
	

	//Add additional matches for 5th place
	if (IsFifthPlaceMatch() && schedule.size() >= 7)
	{
		int offset = 3;//Final and two semi finals
		//if (IsThirdPlaceMatch())
			//offset = 4;

		auto match1 = schedule[schedule.size() - 1 - offset - 3];
		auto match2 = schedule[schedule.size() - 1 - offset - 2];
		auto match3 = schedule[schedule.size() - 1 - offset - 1];
		auto match4 = schedule[schedule.size() - 1 - offset];

		//Order gets fixed at the end
		/*auto semi2 = CreateAutoMatch(nullptr, nullptr);
		auto fifth = CreateAutoMatch(nullptr, nullptr);
		auto semi1 = CreateAutoMatch(nullptr, nullptr);*/

		Match* semi1 = new Match(DependentJudoka(DependencyType::TakeLoser, *match1),
								 DependentJudoka(DependencyType::TakeLoser, *match2),
								 GetTournament(), GetMatID());

		Match* semi2 = new Match(DependentJudoka(DependencyType::TakeLoser, *match3),
								 DependentJudoka(DependencyType::TakeLoser, *match4),
								 GetTournament(), GetMatID());

		Match* fifth = new Match(DependentJudoka(DependencyType::TakeWinner, *semi1),
								 DependentJudoka(DependencyType::TakeWinner, *semi2),
								 GetTournament(), GetMatID());

		semi1->SetMatchTable(this);
		semi1->SetTag(Match::Tag::Fifth() & Match::Tag::Semi());

		semi2->SetMatchTable(this);
		semi2->SetTag(Match::Tag::Fifth() & Match::Tag::Semi());

		fifth->SetMatchTable(this);
		fifth->SetTag(Match::Tag::Fifth() & Match::Tag::Finals());

		m_FifthPlaceMatches.emplace_back(semi1);
		m_FifthPlaceMatches.emplace_back(semi2);
		m_FifthPlaceMatches.emplace_back(fifth);

		/*semi1->SetDependency(Fighter::White, DependencyType::TakeLoser, match1);
		semi1->SetDependency(Fighter::Blue,  DependencyType::TakeLoser, match2);

		semi2->SetDependency(Fighter::White, DependencyType::TakeLoser, match3);
		semi2->SetDependency(Fighter::Blue,  DependencyType::TakeLoser, match4);

		fifth->SetDependency(Fighter::White, DependencyType::TakeWinner, semi1);
		fifth->SetDependency(Fighter::Blue,  DependencyType::TakeWinner, semi2);*/

		//Swap matches so that match for 1st place is still the last one
		//offset = 3;

		/*std::swap(schedule[schedule.size() - 1 - offset - 2], schedule[schedule.size() - 1 - 2]);
		std::swap(schedule[schedule.size() - 1 - offset - 1], schedule[schedule.size() - 1 - 1]);
		std::swap(schedule[schedule.size() - 1 - offset],     schedule[schedule.size() - 1]);

		if (IsThirdPlaceMatch())
			std::swap(schedule[schedule.size() - 1 - offset - 3], schedule[schedule.size() - 1 - 3]);*/
	}


	//Add additional matches for best of three
	if (IsBestOfThree())
		AddMatchesForBestOfThree();
}



MatchTable::Results SingleElimination::CalculateResults() const
{
	Results ret;

	if (GetParticipants().size() == 0)
		return ret;

	if (GetParticipants().size() == 1)
	{
		ret.Add(GetParticipants()[0], this);
		return ret;
	}

	auto& schedule = GetSchedule();

	if (schedule.size() == 0)
		return ret;

	//Get final match
	const Match* lastMatch = schedule[schedule.size() - 1];

	if (lastMatch->HasConcluded())
	{
		ret.Add(lastMatch->GetWinner(), this);
		ret.Add(lastMatch->GetLoser(),  this);
	}
	else
		return ret;

	if (IsThirdPlaceMatch() && !m_ThirdPlaceMatches.empty())
	{
		const Match* third_place_match = m_ThirdPlaceMatches[m_ThirdPlaceMatches.size() - 1];
		ret.Add(third_place_match->GetWinner(), this);
		ret.Add(third_place_match->GetLoser(),  this);
	}

	if (IsThirdPlaceMatch() && IsFifthPlaceMatch() && !m_FifthPlaceMatches.empty())
	{
		const Match* fifth_place_match = m_FifthPlaceMatches[m_FifthPlaceMatches.size() - 1];
		ret.Add(fifth_place_match->GetWinner(), this);
		ret.Add(fifth_place_match->GetLoser(), this);
	}

	return ret;
}



void SingleElimination::ReorderLastMatches()
{
	auto& schedule = SetSchedule();

	if (IsFifthPlaceMatch() && schedule.size() >= 6)
	{
		//Swap matches so that match for 1st place is still the last one
		//Expects: S1   S2   1  3 5_1 5_2 5_3
		//Output:  5_1 5_2 5_3  3  S1  S2   1
		int offset = 3;

		std::swap(schedule[schedule.size() - 1 - offset - 2], schedule[schedule.size() - 1 - 2]);
		std::swap(schedule[schedule.size() - 1 - offset - 1], schedule[schedule.size() - 1 - 1]);
		std::swap(schedule[schedule.size() - 1 - offset],     schedule[schedule.size() - 1]);

		if (IsThirdPlaceMatch())
			std::swap(schedule[schedule.size() - 1 - offset - 3], schedule[schedule.size() - 1 - 3]);
	}

	else if (IsFifthPlaceMatch() && !IsThirdPlaceMatch() && schedule.size() >= 4)
	{
		//Swap matches so that match for 1st place is still the last one
		//Expects: S1 S2  1  5
		//Outputs:  5  S1 S2 1

		std::swap(schedule[schedule.size() - 1], schedule[schedule.size() - 3 - 1]);
		//5 S2 1 S1

		std::swap(schedule[schedule.size() - 1], schedule[schedule.size() - 1 - 1]);
		//5 S2 S1 1

		std::swap(schedule[schedule.size() - 2], schedule[schedule.size() - 2 - 1]);
		//5 S1 S2 1
	}

	else if (IsFifthPlaceMatch() && IsThirdPlaceMatch() && schedule.size() >= 5)
	{
		//Swap matches so that match for 1st place is still the last one
		//Expects: S1 S2  1  3 5
		//Outputs:  5  S1 S2 3 1

		std::swap(schedule[schedule.size() - 1], schedule[schedule.size() - 4 - 1]);
		//5 S2 1 3 S1

		std::swap(schedule[schedule.size() - 1], schedule[schedule.size() - 2 - 1]);
		//5 S2 S1 3 1

		std::swap(schedule[schedule.size() - 3], schedule[schedule.size() - 3 - 1]);
		//5 S1 S2 3 1
	}

	else if (IsThirdPlaceMatch() && schedule.size() >= 2)
	{
		//Swap matches so that match for 1st place is still the last one
		//Expects: 1 3
		//Output:  3 1

		std::swap(schedule[schedule.size() - 1], schedule[schedule.size() - 2]);
	}
}



const std::string SingleElimination::ToHTML() const
{
	std::string ret = GetHTMLTop();

	ret += "<table border='1' rules='all'>";

	const auto rounds = GetNumberOfRounds();
	const auto N = pow(2, rounds);


	auto renderMatchIndex = [this, N](size_t roundIndex, int matchOfRound) -> std::string {
		int matchIndex = 0;
		for (int i = 1; i <= roundIndex; ++i)
			matchIndex += (int)(N / pow(2.0, i));

		matchIndex += matchOfRound;

		if (IsThirdPlaceMatch() && matchIndex >= GetSchedule().size() - 2)
			matchIndex++;
		if (IsFifthPlaceMatch() && matchIndex >= GetSchedule().size() - 5)
		{
			if (GetSchedule().size() >= 6)
				matchIndex += 3;
			else
				matchIndex++;//There is only a single match for 5th place
		}

		if (IsBestOfThree())
			matchIndex = matchIndex * 3;

		std::string style;

		if (roundIndex == 0)
			style = "border-left-style: hidden;";
		if (matchOfRound % 2 == 0)
			style += "border-right-style: hidden;";

		if (matchIndex < GetSchedule().size())
			return RenderMatch(*GetSchedule()[matchIndex], style);
		return "";
	};

	size_t width = 100;
	if (rounds > 0)
		width = 100 / rounds;

	ret += "<tr style='height: 5mm; text-align: center'>";
	for (int round = 0; round < rounds; ++round)
		ret += "<th width=\"" + std::to_string(width) + "%\">" + Localizer::Translate("Round") + " " + std::to_string(round + 1) + "</th>";
	ret += "</tr>";

	for (int y = 0; y < N; ++y)
	{
		ret += "<tr style='height: 5mm; text-align: center'>";

		for (size_t round = 0; round < rounds; ++round)
		{
			if ( (y + (int)std::pow(2, round) + 1) % (int)std::pow(2, round+1) != 0)
			{
				std::string style;
				if (round == 0 || (y + (int)std::pow(2, round) + (int)std::pow(2, round-1) ) % (int)std::pow(2, round+1)  >= (int)std::pow(2, round))
					style += "border-left-style: hidden;";
				if (round+1 == rounds)
					style += "border-right-style: hidden;";

				ret += "<td style=\"" + style + "border-bottom-style: hidden; \"></td>";
				continue;
			}

			const int matchOfRound = y / (int)std::pow(2, round+1);
			ret += renderMatchIndex(round, matchOfRound);
		}

		ret += "</tr>";
	}

	ret += "</table>";


	auto& schedule = GetSchedule();

	if (IsThirdPlaceMatch() && schedule.size() >= 2)
	{
		ret += "<table width=\"" + std::to_string(width) + "%\" border='1' rules='all' style=\"margin-bottom: 5mm;\">";

		ret += "<tr style='height: 5mm; text-align: center'>";
		ret += "<th>" + Localizer::Translate("3rd Place Match") + "</th>";
		ret += "</tr>";

		ret += "<tr style='height: 5mm; text-align: center'>";
		ret += RenderMatch(*schedule[schedule.size() - 2]);
		ret += "</tr>";

		ret += "</table>";
	}


	if (IsFifthPlaceMatch() && schedule.size() < 6)
	{
		ret += "<table border='1' rules='all' style=\"margin-bottom: 5mm;\">";

		ret += "<tr style='height: 5mm; text-align: center'>";
		ret += "<th width=\"" + std::to_string(width) + "%\">" + Localizer::Translate("5th Place Match") + "</th>";
		ret += "</tr>";

		int offset = 3;
		if (IsThirdPlaceMatch())
			offset = 4;

		ret += "<tr style='height: 5mm; text-align: center'>";
		ret += RenderMatch(*schedule[schedule.size() - offset - 1]);
		ret += "</tr>";

		ret += "</table>";
	}

	else if (IsFifthPlaceMatch() && schedule.size() >= 6)
	{
		ret += "<table border='1' rules='all' style=\"margin-bottom: 5mm;\">";

		ret += "<tr style='height: 5mm; text-align: center'>";
		ret += "<th colspan=\"2\" width=\"" + std::to_string(width*2) + "%\">" + Localizer::Translate("5th Place Match") + "</th>";
		ret += "</tr>";

		int offset = 4;
		if (IsThirdPlaceMatch())
			offset = 5;

		ret += "<tr style='height: 5mm; text-align: center'>";
		ret += RenderMatch(*schedule[schedule.size() - offset - 2], "border-left-style: hidden; border-right-style: hidden;");
		ret += "<td style=\"border-bottom-style: hidden; border-right-style: hidden;\"></td>";
		ret += "</tr>";

		ret += "<tr style='height: 5mm; text-align: center'>";
		ret += "<td style=\"border-bottom-style: hidden; border-left-style: hidden;\"></td>";
		ret += RenderMatch(*schedule[schedule.size() - offset], "border-right-style: hidden;");
		ret += "</tr>";

		ret += "<tr style='height: 5mm; text-align: center'>";
		ret += RenderMatch(*schedule[schedule.size() - offset - 1], "border-left-style: hidden;");
		ret += "<td style=\"border-bottom-style: hidden; border-right-style: hidden;\"></td>";
		ret += "</tr>";

		ret += "</table>";
	}

	if (!IsSubMatchTable())
		ret += ResultsToHTML() + "</div>";

	return ret;
}



std::string SingleElimination::RenderMatch(const Match& match, std::string style) const
{
	std::string ret = "<td style=\"" + style + "\">";

	if (!match.IsEmptyMatch())
		ret += "<a href='#edit_match.html?id=" + (std::string)match.GetUUID() + "'>";

	//Output name of fighters
	if (match.GetFighter(Fighter::White))
		ret += match.GetFighter(Fighter::White)->GetName(NameStyle::GivenName);
	//else if (match->HasDependentMatches() || match->GetDependencyTypeOf(Fighter::White))
	else if ((match.GetDependencyTypeOf(Fighter::White) != DependencyType::None) &&
		(match.GetDependencyTypeOf(Fighter::White) != DependencyType::BestOfThree))
		ret += "???";
	else
		ret += "- - -";

	ret += " vs. ";

	if (match.GetFighter(Fighter::Blue))
		ret += match.GetFighter(Fighter::Blue)->GetName(NameStyle::GivenName);
	//else if (match->HasDependentMatches())
	else if ((match.GetDependencyTypeOf(Fighter::Blue) != DependencyType::None) &&
		(match.GetDependencyTypeOf(Fighter::Blue) != DependencyType::BestOfThree))
		ret += "???";
	else
		ret += "- - -";

	//Output result
	if (match.IsRunning())
		ret += "<br/>" + Localizer::Translate("In Progress");
	else if (match.HasConcluded() && !match.IsCompletelyEmptyMatch())
	{
		const auto& result = match.GetResult();
		if (result.m_Winner == Winner::White)
			ret += "<br/>"   + std::to_string((int)result.m_Score) + ":0";
		else
			ret += "<br/>0:" + std::to_string((int)result.m_Score);

		ret += " (" + Timer::TimestampToString(result.m_Time) + ")";
	}

	if (!match.IsEmptyMatch())
		ret += "</a>";
	ret += "</a></td>";

	return ret;
};