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

	class Association : public ID
	{
	public:
		Association(const std::string& Name);
		Association(const YAML::Node& Yaml);

		std::string GetName()   const { return m_Name; }
		auto        GetParent() const { return m_pParent; }

		void SetName(std::string NewName) { m_Name = NewName; }

		void operator >> (YAML::Emitter& Yaml) const;

	private:
		std::string m_Name;

		const Association* m_pParent = nullptr;
	};
}