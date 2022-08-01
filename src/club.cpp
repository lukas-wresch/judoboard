#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "club.h"



using namespace Judoboard;



Club::Club(const std::string& Name) : Association(Name)
{	
}



Club::Club(const YAML::Node& Yaml) : Association(Yaml)
{
}



Club::Club(const MD5::Club& Club) : Association(Club.Name)
{
}