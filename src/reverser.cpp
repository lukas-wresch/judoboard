#include "reverser.h"



using namespace Judoboard;



Reverser::Reverser(const IFilter& pSource) :
	m_pSource(pSource)
{
}



void Reverser::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	IFilter::operator>>(Yaml);

	Yaml << YAML::EndMap;
}



std::string Reverser::GetHTMLForm() const
{
	std::string ret;

	return ret;
}



bool Reverser::IsElgiable(const Judoka& Fighter) const
{
	return m_pSource.IsElgiable(Fighter);
}



std::unordered_map<size_t, const DependentJudoka> Reverser::GetParticipants() const
{
	std::unordered_map<size_t, const DependentJudoka> ret;

	for (auto [pos, judoka] : m_pSource.GetParticipants())
		ret.insert({ m_pSource.GetParticipants().size() - 1 - pos, judoka });

	SetParticipants(std::move(ret));//Save to cache

	return IFilter::GetParticipants();//Return the cache
}



const DependentJudoka Reverser::GetJudokaByStartPosition(size_t StartPosition) const
{
	GetParticipants();//Re-calculates the participants
	return IFilter::GetJudokaByStartPosition(StartPosition);
}