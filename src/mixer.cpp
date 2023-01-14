#include "mixer.h"



using namespace Judoboard;



Mixer::Mixer(const IFilter& pSource1, const IFilter& pSource2) :
	IFilter(pSource1.GetTournament())
{
	m_pSources.push_back(&pSource1);
	m_pSources.push_back(&pSource2);
	Recalculate();
	assert(pSource1.GetTournament() == pSource2.GetTournament());
}



/*Mixer::Mixer(const YAML::Node& Yaml, const ITournament* Tournament) : IFilter(Yaml, Tournament)
{
}*/



void Mixer::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	IFilter::operator>>(Yaml);

	Yaml << YAML::EndMap;
}



std::string Mixer::GetHTMLForm() const
{
	std::string ret;

	return ret;
}



bool Mixer::IsElgiable(const Judoka& Fighter) const
{
	for (auto source : m_pSources)
		if (source && source->IsElgiable(Fighter))
			return true;
	return false;
}



void Mixer::Recalculate()
{
	std::unordered_map<size_t, const DependentJudoka> ret;

	const auto count = m_pSources.size();

	for (size_t i = 0; i < count; ++i)
		for (auto [pos, judoka] : m_pSources[i]->GetParticipants())
			ret.insert({ count*pos + i, judoka });
	
	SetParticipants(std::move(ret));//Save to cache
}



/*const DependentJudoka Mixer::GetJudokaByStartPosition(size_t StartPosition) const
{
	//const auto index = StartPosition % m_pSources.size();
	//return m_pSources[index]->GetJudokaByStartPosition((StartPosition - index) / m_pSources.size());
	GetParticipants();//Re-calculates the participants
	return IFilter::GetJudokaByStartPosition(StartPosition);
}*/