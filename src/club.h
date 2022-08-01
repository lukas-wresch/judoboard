#pragma once
#include "association.h"
#include "md5.h"



namespace Judoboard
{
	class Club : public Association
	{
	public:
		Club(const std::string& Name);
		Club(const YAML::Node& Yaml);//Load club from file
		Club(const MD5::Club& Club);

	private:
	};
}