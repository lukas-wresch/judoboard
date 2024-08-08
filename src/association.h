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


	class Association : public ID
	{
	public:
		Association(const std::string& Name, std::shared_ptr<const Association> Parent);
		Association(const YAML::Node&  Yaml, const StandingData* StandingData);
		Association(const MD5::Association& MD5Association);

		//0 for the highest level, +1 for every level below
		virtual int GetLevel() const {
			if (m_pParent)
				return m_pParent->GetLevel() + 1;
			return 0;//We are on the highest level
		}

		std::string GetName()      const { return m_Name; }
		std::string GetShortName() const { return m_ShortName; }
		auto        GetParent()    const { return m_pParent; }

		void SetName(const std::string& NewName) {
			m_Name = NewName;
			if (m_ShortName.empty())
				m_ShortName = m_Name.substr(0, 5);
		}
		void SetShortName(const std::string& NewShortName) {
			m_ShortName = NewShortName;
			if (m_Name.empty())
				m_Name = m_ShortName;
		}

		void SetParent(std::shared_ptr<const Association> NewParent) { m_pParent = NewParent; }

		bool IsChildOf(const UUID& UUID) const;

		void operator >> (YAML::Emitter& Yaml) const;
		void ToString(YAML::Emitter& Yaml) const;

		bool operator < (const Association& rhs) const;		

	private:
		std::string m_Name;
		std::string m_ShortName;

		std::shared_ptr<const Association> m_pParent;
	};
}