#pragma once
#include "id.h"


namespace YAML
{
	class Emitter;
	class Node;
}



namespace Judoboard
{
	class StandingData;

	class AgeGroup : public ID
	{
	public:
		AgeGroup(const std::string& Name);
		AgeGroup(const YAML::Node& Yaml);

		std::string GetName() const { return m_Name; }

		void operator >> (YAML::Emitter& Yaml) const;

	private:
		std::string m_Name;

		uint32_t m_MinAge = 0;//Age by birthyear
		uint32_t m_MaxAge = 100;
	};
}