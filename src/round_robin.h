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
		RoundRobin(const MD5::Weightclass& Weightclass, const ITournament* Tournament = nullptr);

		std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::RoundRobin; }

		virtual std::string GetDescription() const override;

		virtual std::vector<Result> CalculateResults() const override;
		virtual void GenerateSchedule() override;

		//Serialization
		virtual const std::string ToHTML() const override;

	private:
	};
}