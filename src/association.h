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
		Association(const std::string& Name, const Association* Parent = nullptr);
		Association(const YAML::Node& Yaml, const StandingData* StandingData);

		virtual int GetLevel() const { return m_Level; }

		std::string GetName()   const { return m_Name; }
		auto        GetParent() const { return m_pParent; }

		void SetName(std::string NewName) { m_Name = NewName; }

		void operator >> (YAML::Emitter& Yaml) const;
		void ToString(YAML::Emitter& Yaml) const;

	protected:
		void SetParent(const Association* NewParent) { m_pParent = NewParent; }

	private:
		std::string m_Name;

		int m_Level = 0;//0 for the highest level, +1 for every level below
		const Association* m_pParent = nullptr;
	};
}