#pragma once
#include "association.h"
#include "md5.h"



namespace Judoboard
{
	class Club : public Association
	{
	public:
		Club(const std::string& Name);
		Club(const YAML::Node& Yaml, const StandingData* StandingData = nullptr);//Load club from file
		Club(const MD5::Club& Club);

		virtual int GetLevel() const override { return -1; }

	private:
	};
}