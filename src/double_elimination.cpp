#include <algorithm>
#include <random>
#include "double_elimination.h"
#include "weightclass.h"
#include "losersof.h"
#include "fixed.h"
#include "rule_set.h"
#include "localizer.h"
#include "match.h"



using namespace Judoboard;



DoubleElimination::DoubleElimination(IFilter* Filter, const ITournament* Tournament)
	: MatchTable(Filter, Tournament), m_WinnerBracket(nullptr, Tournament), m_LoserBracket(nullptr, Tournament)
{
	m_WinnerBracket.IsSubMatchTable(true);
	m_LoserBracket.IsSubMatchTable(true);
}



DoubleElimination::DoubleElimination(Weight MinWeight, Weight MaxWeight, Gender Gender, const ITournament* Tournament)
	: DoubleElimination(new Weightclass(MinWeight, MaxWeight, Gender, Tournament), Tournament)
{
}



DoubleElimination::DoubleElimination(const YAML::Node& Yaml, ITournament* Tournament)
	: MatchTable(Yaml, Tournament), m_WinnerBracket(nullptr, Tournament), m_LoserBracket(nullptr, Tournament)
{
	m_WinnerBracket.IsSubMatchTable(true);
	m_LoserBracket.IsSubMatchTable(true);

	if (Yaml["third_place_match"])
		m_WinnerBracket.IsThirdPlaceMatch(Yaml["third_place_match"].as<bool>());
	if (Yaml["fifth_place_match"])
		m_WinnerBracket.IsFifthPlaceMatch(Yaml["fifth_place_match"].as<bool>());
	
	
	if (Yaml["mat_of_winner_bracket"])
		m_WinnerBracket.SetMatID(Yaml["mat_of_winner_bracket"].as<uint32_t>());
	if (Yaml["name_of_winner_bracket"])
		m_WinnerBracket.SetName(Yaml["name_of_winner_bracket"].as<std::string>());

	if (Yaml["mat_of_loser_bracket"])
		m_LoserBracket.SetMatID(Yaml["mat_of_loser_bracket"].as<uint32_t>());
	if (Yaml["name_of_loser_bracket"])
		m_LoserBracket.SetName(Yaml["name_of_loser_bracket"].as<std::string>());
}



void DoubleElimination::operator >> (YAML::Emitter& Yaml) const
{
	if (!IsSubMatchTable())
		Yaml << YAML::BeginMap;

	MatchTable::operator >>(Yaml);

	if (IsThirdPlaceMatch())
		Yaml << YAML::Key << "third_place_match" << YAML::Value << IsThirdPlaceMatch();
	if (IsFifthPlaceMatch())
		Yaml << YAML::Key << "fifth_place_match" << YAML::Value << IsFifthPlaceMatch();

	//Serialize mats of pools if they differ
	if (m_WinnerBracket.GetMatID() != 0)
		Yaml << YAML::Key << "mat_of_winner_bracket" << YAML::Value << m_WinnerBracket.GetMatID();
	Yaml << YAML::Key << "name_of_winner_bracket" << YAML::Value << m_WinnerBracket.GetName();

	if (m_LoserBracket.GetMatID() != 0)
		Yaml << YAML::Key << "mat_of_loser_bracket" << YAML::Value << m_LoserBracket.GetMatID();
	Yaml << YAML::Key << "name_of_loserbracket" << YAML::Value << m_LoserBracket.GetName();

	if (!IsSubMatchTable())
		Yaml << YAML::EndMap;
}



void DoubleElimination::ToString(YAML::Emitter& Yaml) const
{
	MatchTable::ToString(Yaml);

	Yaml << YAML::Key << "third_place" << YAML::Value << IsThirdPlaceMatch();
	Yaml << YAML::Key << "fifth_place" << YAML::Value << IsFifthPlaceMatch();
}



std::string DoubleElimination::GetHTMLForm()
{
	std::string ret = SingleElimination::GetHTMLForm();

	return ret;
}



size_t DoubleElimination::GetMaxStartPositions() const
{
	if (!GetFilter())
		return 0;

	return m_WinnerBracket.GetMaxStartPositions();
}



void DoubleElimination::GenerateSchedule()
{
	if (GetStatus() != Status::Scheduled)
		return;

	/*for (auto it = m_Schedule.begin(); it != m_Schedule.end();)
	{
		if ((*it)->IsAutoGenerated())
			it = m_Schedule.erase(it);
		else
			++it;
	}*/
	DeleteSchedule();

	m_RecommendedNumMatches_Before_Break = 4;//TODO

	if (!GetFilter() || GetParticipants().size() <= 1)
		return;


	m_WinnerBracket.SetFilter(this->GetFilter());
	m_WinnerBracket.GenerateSchedule();

	delete m_LoserBracket.GetFilter();
	
	/*LosersOf losers_of(m_WinnerBracket);

	//Remove the last one, second place doesn't go to the loser bracket
	auto num_participants = this->GetFilter()->GetParticipants().size();
	auto last = losers_of.GetJudokaByStartPosition(num_participants - 2);
	losers_of.RemoveParticipant(last);*/

	auto losers = new LosersOf(m_WinnerBracket);
	losers->RemoveLast();
	m_LoserBracket.SetFilter(losers);

	m_LoserBracket.GenerateSchedule();

	//Add matches
	for (auto match : m_WinnerBracket.GetSchedule())
		AddMatch(match);
	for (auto match : m_LoserBracket.GetSchedule())
		AddMatch(match);
}



const std::string DoubleElimination::ToHTML() const
{
	std::string ret;

	ret += "<a href=\"#matchtable_add.html?id=" + (std::string)GetUUID() + "\">" + GetDescription() + "</a>";

	ret += " / " + Localizer::Translate("Mat") + " " + std::to_string(GetMatID()) + " / " + GetRuleSet().GetName() + "<br/>";

	ret += "<br/>";

	ret += m_WinnerBracket.ToHTML() + "<br/><br/>";

	ret += m_LoserBracket.ToHTML();

	//ret += ResultsToHTML();

	return ret;
}