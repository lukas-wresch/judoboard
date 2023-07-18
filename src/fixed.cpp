#include "fixed.h"



using namespace Judoboard;



Fixed::Fixed(const IFilter& Source) :
	IFilter(Source.GetParent())
{
	SetParticipants(Source.GetParticipants());
	SetAgeGroup(Source.GetAgeGroup());
}



Fixed::Fixed(const YAML::Node& Yaml, const MatchTable* Parent) : IFilter(Yaml, Parent)
{
}



void Fixed::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	IFilter::operator>>(Yaml);

	Yaml << YAML::EndMap;
}



void Fixed::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();
}



std::string Fixed::GetHTMLForm() const
{
	std::string ret;

	return ret;
}