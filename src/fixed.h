#pragma once
#include "filter.h"



namespace Judoboard
{
	class Fixed : public IFilter
	{
	public:
		Fixed(std::shared_ptr<const IFilter> Source);
		Fixed(const MatchTable* Parent = nullptr) : IFilter(Parent) {}
		Fixed(const YAML::Node& Yaml, const MatchTable* Parent);

		virtual Type GetType() const override { return Type::Fixed; }

		virtual std::string GetHTMLForm() const;

		virtual bool IsElgiable(const Judoka& Fighter) const override { return true; }

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	private:
	};
}