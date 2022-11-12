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
		Association(const std::string& Name, const Association* Parent) {}
		Association(const YAML::Node&  Yaml, const StandingData* StandingData);

		//0 for the highest level, +1 for every level below
		virtual int GetLevel() const {
			if (m_pParent)
				return m_pParent->GetLevel() + 1;
			return 0;//We are on the highest level
		}

		std::string GetName()   const { return m_Name; }
		auto        GetParent() const { return m_pParent; }

		void SetName(std::string NewName) { m_Name = NewName; }
		void SetParent(const Association* NewParent) { m_pParent = NewParent; }

		void operator >> (YAML::Emitter& Yaml) const;
		void ToString(YAML::Emitter& Yaml) const;

		bool operator < (const Association& rhs) const;		

	private:
		std::string m_Name;

		const Association* m_pParent = nullptr;
	};
}