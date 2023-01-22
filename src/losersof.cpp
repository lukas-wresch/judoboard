#include "losersof.h"



using namespace Judoboard;



LosersOf::LosersOf(const MatchTable& Table) :
	IFilter(Table.GetTournament()), m_MatchTable(Table)
{
	if (Table.GetFilter())
	{
		SetAgeGroup(Table.GetFilter()->GetAgeGroup());
		Recalculate();
	}
}



//LosersOf::LosersOf(const YAML::Node& Yaml, const ITournament* Tournament) : IFilter(Yaml, Tournament)
//{
//}



void LosersOf::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	IFilter::operator>>(Yaml);

	Yaml << YAML::EndMap;
}



/*void LosersOf::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();
}*/



std::string LosersOf::GetHTMLForm() const
{
	std::string ret;

	return ret;
}



std::unordered_map<size_t, const DependentJudoka> LosersOf::GetParticipants() const
{
	Recalculate();//Re-calculates the participants
	return IFilter::GetParticipants();
}



const DependentJudoka LosersOf::GetJudokaByStartPosition(size_t StartPosition) const
{
	Recalculate();//Re-calculates the participants
	return IFilter::GetJudokaByStartPosition(StartPosition);
}



void LosersOf::Recalculate() const
{
	std::unordered_map<size_t, const DependentJudoka> ret;
	size_t i = 0;

	for (auto match : m_MatchTable.GetSchedule())
		ret.insert({ i++, DependentJudoka(DependencyType::TakeLoser, *match) });

	if (m_RemoveLast)
		ret.erase(i-1);

	SetParticipants(std::move(ret));//Save to cache
}