#include "splitter.h"



using namespace Judoboard;



Splitter::Splitter(const IFilter& pSource, size_t Divisor, size_t Remainder, const MatchTable* Parent) :
	m_pSource(pSource), IFilter(Parent)
{
	m_Divisor   = Divisor;
	m_Remainder = Remainder;

	assert(Remainder < Divisor);
	m_Remainder %= Divisor;
}



/*Splitter::Splitter(const YAML::Node& Yaml, const MatchTable* Parent) : IFilter(Yaml, Parent)
{
	if (Yaml["divisor"])
		m_Divisor = Yaml["divisor"].as<size_t>();
	if (Yaml["Remainder"])
		m_Remainder = Yaml["Remainder"].as<size_t>();
}*/



void Splitter::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	IFilter::operator>>(Yaml);

	Yaml << YAML::EndMap;
}



void Splitter::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();
}



std::string Splitter::GetHTMLForm() const
{
	std::string ret;

	return ret;
}



bool Splitter::IsElgiable(const Judoka& Fighter) const
{
	return m_pSource.IsElgiable(Fighter);
}



std::unordered_map<size_t, const DependentJudoka> Splitter::GetParticipants() const
{
	std::unordered_map<size_t, const DependentJudoka> ret;
	for (auto [pos, judoka] : m_pSource.GetParticipants())
	{
		if (pos % m_Divisor == m_Remainder)
			ret.insert({ (pos - m_Remainder) / m_Divisor, judoka });
	}
	return ret;
}



const DependentJudoka Splitter::GetJudokaByStartPosition(size_t StartPosition) const
{
	return m_pSource.GetJudokaByStartPosition(StartPosition * m_Divisor + m_Remainder);
}