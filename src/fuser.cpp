#include "fuser.h"



using namespace Judoboard;



Fuser::Fuser(const IFilter& pSource1, const IFilter& pSource2) :
	IFilter(pSource1.GetTournament())
{
	m_pSources.push_back(&pSource1);
	m_pSources.push_back(&pSource2);
	assert(pSource1.GetTournament() == pSource2.GetTournament());
}



/*Fuser::Fuser(const YAML::Node& Yaml, const ITournament* Tournament) : IFilter(Yaml, Tournament)
{
}*/



void Fuser::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	IFilter::operator>>(Yaml);

	Yaml << YAML::EndMap;
}



void Fuser::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();
}



std::string Fuser::GetHTMLForm() const
{
	std::string ret;

	return ret;
}



bool Fuser::IsElgiable(const Judoka& Fighter) const
{
	for (auto source : m_pSources)
		if (source && source->IsElgiable(Fighter))
			return true;
	return false;
}



std::unordered_map<size_t, const DependentJudoka> Fuser::GetParticipants() const
{
	std::unordered_map<size_t, const DependentJudoka> ret;

	size_t i = 0;
	for (auto source : m_pSources)
	{
		for (auto [pos, judoka] : source->GetParticipants())
			ret.insert({ i + pos, judoka });
		i += source->GetParticipants().size();
	}
	
	SetParticipants(std::move(ret));//Save to cache

	return IFilter::GetParticipants();//Return the cache
}



const DependentJudoka Fuser::GetJudokaByStartPosition(size_t StartPosition) const
{
	GetParticipants();//Re-calculates the participants
	return IFilter::GetJudokaByStartPosition(StartPosition);
}