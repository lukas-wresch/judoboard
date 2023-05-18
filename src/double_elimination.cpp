#include <algorithm>
#include <random>
#include <cassert>
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
	m_WinnerBracket.SetParent(this);
	m_LoserBracket.SetParent(this);
}



DoubleElimination::DoubleElimination(Weight MinWeight, Weight MaxWeight, Gender Gender, const ITournament* Tournament)
	: DoubleElimination(new Weightclass(MinWeight, MaxWeight, Gender, this), Tournament)
{
}



DoubleElimination::DoubleElimination(const YAML::Node& Yaml, ITournament* Tournament, const MatchTable* Parent)
	: MatchTable(Yaml, Tournament, Parent), m_WinnerBracket(nullptr, Tournament, this), m_LoserBracket(nullptr, Tournament, this)
{
	if (Yaml["winner_bracket"])
		m_WinnerBracket = SingleElimination(Yaml["winner_bracket"], Tournament, this);
	if (Yaml["loser_bracket"])
		m_LoserBracket = LoserBracket(Yaml["loser_bracket"], Tournament, this);

#ifdef _DEBUG
	for (auto match : m_WinnerBracket.GetSchedule())
		assert(match->GetMatchTable() == &m_WinnerBracket);
	for (auto match : m_LoserBracket.GetSchedule())
		assert(match->GetMatchTable() == &m_LoserBracket);
#endif
}



void DoubleElimination::operator >> (YAML::Emitter& Yaml) const
{
	if (!IsSubMatchTable())
		Yaml << YAML::BeginMap;

	MatchTable::operator >>(Yaml);

	Yaml << YAML::Key << "winner_bracket" << YAML::Value;
	Yaml << YAML::BeginMap;
	m_WinnerBracket >> Yaml;
	Yaml << YAML::EndMap;

	Yaml << YAML::Key << "loser_bracket" << YAML::Value;
	Yaml << YAML::BeginMap;
	m_LoserBracket >> Yaml;
	Yaml << YAML::EndMap;

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
	if (!IsSubMatchTable() && GetStatus() != Status::Scheduled)
		return;

	SetSchedule().clear();

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



void DoubleElimination::AddMatchToWinnerBracket(Match* NewMatch)
{
	if (!NewMatch)
		return;

	if (NewMatch->GetFighter(Fighter::White))
		AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::White)), true);
	if (NewMatch->GetFighter(Fighter::Blue))
		AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::Blue)),  true);

	m_WinnerBracket.AddMatch(NewMatch);
	SetSchedule().emplace_back(NewMatch);
}



void DoubleElimination::AddMatchToLoserBracket(Match* NewMatch)
{
	if (!NewMatch)
		return;

	if (NewMatch->GetFighter(Fighter::White))
		AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::White)), true);
	if (NewMatch->GetFighter(Fighter::Blue))
		AddParticipant(const_cast<Judoka*>(NewMatch->GetFighter(Fighter::Blue)),  true);

	m_LoserBracket.AddMatch(NewMatch);
	SetSchedule().emplace_back(NewMatch);
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