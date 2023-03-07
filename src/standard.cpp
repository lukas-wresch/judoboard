#include "standard.h"



using namespace Judoboard;



Standard::Standard(const MatchTable* Parent) :
	IFilter(Parent)
{
}



Standard::Standard(const YAML::Node& Yaml, const MatchTable* Parent) : IFilter(Yaml, Parent)
{
}



void Standard::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	IFilter::operator>>(Yaml);

	Yaml << YAML::EndMap;
}



/*void Standard::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();
}*/



std::string Standard::GetHTMLForm() const
{
	std::string ret;

	return ret;
}