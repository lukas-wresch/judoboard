#pragma once
#include "weightclass.h"



namespace Judoboard
{
	class SingleElimination : public Weightclass
	{
		friend class Tournament;

	public:
		SingleElimination(const ITournament* Tournament, Weight MinWeight, Weight MaxWeight);
		SingleElimination(const YAML::Node& Yaml, ITournament* Tournament);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::SingleElimination; }

		virtual std::vector<Result> CalculateResults() const override;
		virtual void GenerateSchedule() override;

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	private:
	};
}