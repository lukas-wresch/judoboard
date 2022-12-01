#pragma once
#include "matchtable.h"



namespace Judoboard
{
	class CustomTable : public MatchTable
	{
		friend class Tournament;

	public:
		CustomTable(const ITournament* Tournament);
		CustomTable(const YAML::Node& Yaml, ITournament* Tournament) : MatchTable(Yaml, Tournament) {}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Custom; }
		virtual std::string GetDescription() const override { return GetName() + " Custom"; }

		virtual Status GetStatus() const override;

		virtual Results CalculateResults() const override;

		virtual void GenerateSchedule() override {}

		//Serialization
		virtual const std::string ToHTML() const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	private:
	};
}