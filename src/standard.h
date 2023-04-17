#pragma once
#include "filter.h"



namespace Judoboard
{
	class Standard : public IFilter
	{
	public:
		Standard(const MatchTable* Parent = nullptr);
		Standard(const YAML::Node& Yaml, const MatchTable* Parent = nullptr);

		virtual Type GetType() const override { return Type::Standard; }

		virtual std::string GetHTMLForm() const;

		virtual bool IsElgiable(const Judoka& Fighter) const override { return true; }

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;
		//virtual void ToString(YAML::Emitter& Yaml) const override;

	private:
	};
}