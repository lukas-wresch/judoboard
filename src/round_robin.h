#pragma once
#include "matchtable.h"
#include "md5.h"
#include "filter.h"



namespace Judoboard
{
	class RoundRobin : public MatchTable
	{
		friend class Tournament;

	public:
		RoundRobin(IFilter* Filter, const ITournament* Tournament = nullptr);
		RoundRobin(const YAML::Node& Yaml, ITournament* Tournament = nullptr);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Weightclass; }

		virtual std::string GetDescription() const override;

		virtual std::vector<Result> CalculateResults() const override;
		virtual bool IsElgiable(const Judoka& Fighter) const override;
		virtual void GenerateSchedule() override;

		auto IsBestOfThree() const { return m_BestOfThree; }
		void IsBestOfThree(bool Enable) { m_BestOfThree = Enable; GenerateSchedule(); }

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	private:
		bool m_BestOfThree = false;
	};
}