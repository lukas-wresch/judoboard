#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "club.h"



using namespace Judoboard;



Club::Club(const std::string& Name, const Association* ParentAssociation) : Association(Name, ParentAssociation)
{
}



Club::Club(const YAML::Node& Yaml, const StandingData* StandingData) : Association(Yaml, StandingData)
{
}