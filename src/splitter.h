#pragma once
#include "filter.h"



namespace Judoboard
{
	class AgeGroup;


	class Splitter : public IFilter
	{
	public:
		Splitter(const IFilter& pSource, size_t Divisor, size_t Remainder);
		Splitter(const YAML::Node& Yaml, const ITournament* Tournament);

		virtual Type GetType() const override { return Type::Splitter; }

		virtual std::string GetHTMLForm() const;

		virtual bool IsElgiable(const Judoka& Fighter) const override;

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	private:
		const IFilter& m_pSource;
		size_t m_Divisor   = 1;//Split in how many ways?
		size_t m_Remainder = 0;//Take which part?
	};
}