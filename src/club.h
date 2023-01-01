#pragma once
#include "association.h"
#include "md5.h"



namespace Judoboard
{
	class Club : public Association
	{
	public:
		Club(const std::string& Name, const Association* ParentAssociation = nullptr);
		Club(const YAML::Node& Yaml, const StandingData* StandingData = nullptr);//Load club from file
		Club(const MD5::Club& Club) : Association(Club.Name, nullptr) {
			if (Club.Association)
				SetParent(new Association(*Club.Association));
		}

		//virtual int GetLevel() const override { return -1; }

	private:
	};
}