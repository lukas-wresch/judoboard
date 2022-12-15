#include "splitter.h"



using namespace Judoboard;



Splitter::Splitter(const IFilter& pSource, size_t Divisor, size_t Remainder) :
	m_pSource(pSource), IFilter(pSource.GetTournament())
{
	m_Divisor   = Divisor;
	m_Remainder = Remainder;
}



/*Splitter::Splitter(const YAML::Node& Yaml, const ITournament* Tournament) : IFilter(Yaml, Tournament)
{
	if (Yaml["divisor"])
		m_Divisor = Yaml["divisor"].as<size_t>();
	if (Yaml["Remainder"])
		m_Remainder = Yaml["Remainder"].as<size_t>();
}*/



void Splitter::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();

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