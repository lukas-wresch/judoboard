#pragma once
#include "id.h"
#include "../ZED/include/csv.h"



namespace Judoboard
{
	class StandingData;

	class Club : public ID
	{
	public:
		Club(const std::string& Name);
		Club(ZED::CSV& Stream);//Load club from file

		std::string GetName() const { return m_Name; }

		void operator >> (ZED::CSV& Stream) const;

	private:
		std::string m_Name;
	};
}