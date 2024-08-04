#include <algorithm>
#include <random>
#include "loser_bracket.h"
#include "weightclass.h"
#include "tournament.h"
#include "localizer.h"
#include "match.h"



using namespace Judoboard;



LoserBracket::LoserBracket(std::shared_ptr<IFilter> Filter, const ITournament* Tournament, const MatchTable* Parent)
	: SingleElimination(Filter, Tournament, Parent)
{
}



LoserBracket::LoserBracket(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament)
	: LoserBracket(std::make_shared<Weightclass>(MinWeight, MaxWeight, this), Tournament)
{
}



LoserBracket::LoserBracket(const YAML::Node& Yaml, const ITournament* Tournament, const MatchTable* Parent)
	: SingleElimination(Yaml, Tournament, Parent)
{
	assert(Yaml.IsMap());

	if (Yaml["final_match"])
		m_FinalMatch = Yaml["final_match"].as<bool>();
}



LoserBracket::LoserBracket(const MD5::Weightclass& Weightclass_, const ITournament* Tournament)
	: LoserBracket(std::make_shared<Weightclass>(Weightclass_, this), Tournament)
{
}



void LoserBracket::operator >> (YAML::Emitter& Yaml) const
{
	if (!IsSubMatchTable())
		Yaml << YAML::BeginMap;

	MatchTable::operator >>(Yaml);

	if (m_FinalMatch)
		Yaml << YAML::Key << "final_match" << YAML::Value << m_FinalMatch;
	if (IsThirdPlaceMatch())
		Yaml << YAML::Key << "third_place_match" << YAML::Value << IsThirdPlaceMatch();

	if (!IsSubMatchTable())
		Yaml << YAML::EndMap;
}



std::string LoserBracket::GetHTMLForm()
{
	return SingleElimination::GetHTMLForm();
}



void LoserBracket::GenerateSchedule()
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
	const auto max_initial_start_pos = (max_start_pos + 2) / 2;
	
	//Round 1
	std::vector<std::shared_ptr<Match>> lastRound;
	std::vector<std::shared_ptr<Match>> nextRound;

	size_t current_start_pos = 0;

	auto create_pair = [&](size_t i) {
		auto new_match = CreateAutoMatch(GetFilter()->GetJudokaByStartPosition(i-1),
										 GetFilter()->GetJudokaByStartPosition(i+1-1));
		nextRound.emplace_back(new_match);
	};

	//Hardcoded starting positions
	if (max_initial_start_pos == 4)//For double elimination with 8 participants
	{
		create_pair(3);
		create_pair(1);
		current_start_pos = max_initial_start_pos;
	}
	else if (max_initial_start_pos == 16)//For double elimination with 32 participants
	{
		create_pair(13);
		create_pair(15);
		create_pair(9);
		create_pair(11);
		create_pair(5);
		create_pair(7);
		create_pair(1);
		create_pair(3);
		current_start_pos = max_initial_start_pos;
	}
	//Hardcoded starting positions
	else if (max_initial_start_pos == 32)//For double elimination with 64 participants
	{
		create_pair(29);
		create_pair(31);
		create_pair(25);
		create_pair(27);
		create_pair(21);
		create_pair(23);
		create_pair(17);
		create_pair(19);
		create_pair(13);
		create_pair(15);
		create_pair(9);
		create_pair(11);
		create_pair(5);
		create_pair(7);
		create_pair(1);
		create_pair(3);
		current_start_pos = max_initial_start_pos;
	}
	else//Default
	{
		for (; current_start_pos < max_initial_start_pos; current_start_pos += 2)
			create_pair(current_start_pos + 1);
	}

	//Additional rounds
	bool infuse = true;
	bool swap   = true;
	bool special_swap = max_initial_start_pos == 32;//Special swap only used for 64 system

	if (max_initial_start_pos == 4)//For double elimination with 8 participants
		swap = false;

	for (int round = 1; round < rounds; ++round)
	{
		lastRound = std::move(nextRound);

		for (size_t i = 0; i < lastRound.size(); ++i)
		{
			std::shared_ptr<Match> new_match;

			if (infuse)//Infuse further participants
			{
				if (special_swap)
				{
					const size_t list[] = {37, 38, 39, 40, 33, 34, 35, 36, 45, 46, 47, 48, 41, 42, 43, 44};

					new_match = CreateAutoMatch(DependentJudoka(DependencyType::TakeWinner, lastRound[i]),
						GetFilter()->GetJudokaByStartPosition(list[i] - 1));
					current_start_pos++;
				}

				else if (swap)
				{
					if (i < lastRound.size()/2)
						new_match = CreateAutoMatch(DependentJudoka(DependencyType::TakeWinner, lastRound[i]),
													GetFilter()->GetJudokaByStartPosition(current_start_pos++  + lastRound.size()/2));
					else
						new_match = CreateAutoMatch(DependentJudoka(DependencyType::TakeWinner, lastRound[i]),
													GetFilter()->GetJudokaByStartPosition(current_start_pos++  - lastRound.size()/2));
				}

				else
				{
					new_match = CreateAutoMatch(DependentJudoka(DependencyType::TakeWinner, lastRound[i]),
												GetFilter()->GetJudokaByStartPosition(current_start_pos++));
				}
			}
			else//Only take winners from previous round
			{
				if (i+1 >= lastRound.size())
					break;

				auto match1 = lastRound[i];
				auto match2 = lastRound[++i];

				new_match = AddMatchForWinners(match1, match2);
			}

			if (new_match)
				nextRound.emplace_back(new_match);
		}

		if (infuse)
		{
			swap = !swap;
			if (special_swap)
			{
				special_swap = false;
				swap = true;
			}
		}

		infuse = !infuse;//Switch for next round
	}

	
	//Add additional match for 3rd place
	if (IsThirdPlaceMatch() && GetSchedule().size() >= 2)
	{
		auto match1 = GetSchedule()[GetSchedule().size() - 2];
		auto match2 = GetSchedule()[GetSchedule().size() - 1];

		if (IsFinalMatch() && GetSchedule().size() >= 3)
		{
			match1 = GetSchedule()[GetSchedule().size() - 3];
			match2 = GetSchedule()[GetSchedule().size() - 2];
		}

		auto third_place = CreateAutoMatch(nullptr, nullptr);
		third_place->SetDependency(Fighter::White, DependencyType::TakeLoser, match1);
		third_place->SetDependency(Fighter::Blue,  DependencyType::TakeLoser, match2);
	}


	//Add additional matches for best of three
	if (IsBestOfThree())
		AddMatchesForBestOfThree();
}



MatchTable::Results LoserBracket::CalculateResults() const
{
	Results ret;

	if (GetSchedule().size() == 0)
		return ret;

	auto& schedule = GetSchedule();

	//Get final match
	if (IsFinalMatch())
	{
		auto lastMatch = schedule[schedule.size() - 1];
		if (IsThirdPlaceMatch() && schedule.size() >= 2)
			lastMatch = schedule[schedule.size() - 2];

		if (lastMatch->HasConcluded())
		{
			ret.Add(lastMatch->GetWinner(), this);
			ret.Add(lastMatch->GetLoser(),  this);
		}
	}

	if (IsThirdPlaceMatch())
	{
		auto third_place_match = schedule[schedule.size() - 1];
		if (third_place_match->HasConcluded())
		{
			ret.Add(third_place_match->GetWinner(), this);
			ret.Add(third_place_match->GetLoser(), this);
		}
	}

	return ret;
}



const std::string LoserBracket::ToHTML() const
{
	std::string ret = GetHTMLTop();

	ret += "<table border='1' rules='all'>";

	const auto rounds = GetNumberOfRounds();
	const auto max_start_pos = GetMaxStartPositions();
	const auto max_initial_start_pos = (max_start_pos + 2) / 2;
	const auto N = max_initial_start_pos;


	auto renderMatchIndex = [this, N, max_initial_start_pos](size_t roundIndex, int matchOfRound) -> std::string {
		size_t matchIndex = 0;

		size_t no_matches = max_initial_start_pos;
		for (size_t i = 0; i < roundIndex; i++)
		{
			if (i%2 == 0)
				no_matches /= 2;
			matchIndex += no_matches;
		}

		matchIndex += matchOfRound;

		if (IsBestOfThree())
			matchIndex = matchIndex * 3;

		std::string style;

		if (roundIndex == 0)
			style = "border-left-style: hidden;";
		if (matchOfRound % 2 == 0)
			style += "border-right-style: hidden;";

		//return "<td>INDEX " + std::to_string(matchIndex) + "</td>";

		if (matchIndex < GetSchedule().size())
			return RenderMatch(*GetSchedule()[matchIndex], style);
		return "<td>NOT FOUND</td>";
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
			bool infuse = round % 2 == 1;

			size_t no_matches = max_initial_start_pos / 2;//For round 0
			for (size_t i = 1; i < round; i+=2)
				no_matches /= 2;

			assert(no_matches != 0);

			const size_t split = N / no_matches;
			const size_t offset = split - round;

			if ( (y + offset) % split != 0)
			{
				std::string style;
				if (round == 0)
					style += "border-left-style: hidden;";
				else if (infuse)
					style += "border-left-style: hidden;";
				else if ((y + offset) % split >= round)//For not infuse stage
					style += "border-left-style: hidden;";
				if (round+1 == rounds)
					style += "border-right-style: hidden;";

//#ifdef _DEBUG
				//ret += "<td style=\"" + style + "border-bottom-style: hidden; \">" + std::to_string((y + offset) % split) + "</td>";
//#else
				ret += "<td style=\"" + style + "border-bottom-style: hidden; \"></td>";
//#endif
				continue;
			}

			const int matchOfRound = y / (int)split;
			ret += renderMatchIndex(round, matchOfRound);
		}

		ret += "</tr>";
	}

	ret += "</table>";


	if (IsThirdPlaceMatch())//3rd or 5th place match
	{
		ret += "<table width=\"" + std::to_string(width) + "%\" border='1' rules='all' style=\"margin-top: 10mm;\">";

		ret += "<tr style='height: 5mm; text-align: center'>";
		if (!IsSubMatchTable())
			ret += "<th>" + Localizer::Translate("3rd Place Match") + "</th>";
		else
			ret += "<th>" + Localizer::Translate("5th Place Match") + "</th>";
		ret += "</tr>";

		ret += "<tr style='height: 5mm; text-align: center'>";

		int matchIndex = (int)GetSchedule().size() - 1;

		if (IsBestOfThree())
			matchIndex = matchIndex * 3;

		if (matchIndex < GetSchedule().size())
			ret += RenderMatch(*GetSchedule()[matchIndex]);

		ret += "</tr>";

		ret += "</table>";
	}

	if (!IsSubMatchTable())
		ret += ResultsToHTML();	

	return ret;
}