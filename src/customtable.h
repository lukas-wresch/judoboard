#pragma once
#include "round_robin.h"



namespace Judoboard
{
	class CustomTable : public RoundRobin
	{
		friend class Tournament;

	public:
		CustomTable(const ITournament* Tournament);
		CustomTable(const YAML::Node& Yaml, const ITournament* Tournament) : RoundRobin(Yaml, Tournament) {}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Custom; }
		virtual std::string GetDescription() const override { return GetName() + " Custom"; }

		virtual void GenerateSchedule() override {}

		//Serialization
		virtual const std::string ToHTML() const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	private:
	};
}