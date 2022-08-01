#pragma once
#include "id.h"
#include "md5.h"


namespace YAML
{
	class Emitter;
	class Node;
}



namespace Judoboard
{
	class StandingData;

	class Club : public ID
	{
	public:
		Club(const std::string& Name);
		Club(const YAML::Node& Yaml);//Load club from file
		Club(const MD5::Club& Club);

		std::string GetName() const { return m_Name; }

		void operator >> (YAML::Emitter& Yaml) const;

	private:
		std::string m_Name;
	};
}