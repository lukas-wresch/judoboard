#pragma once
#include "round_robin.h"



namespace Judoboard
{
	class CustomTable : public MatchTable
	{
		friend class Tournament;

	public:
		CustomTable(const ITournament* Tournament = nullptr);
		CustomTable(const YAML::Node& Yaml, const ITournament* Tournament, const MatchTable* Parent = nullptr) : MatchTable(Yaml, Tournament, Parent) {}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Custom; }
		virtual std::string GetDescription() const override { return GetName(); }

		virtual void GenerateSchedule() override {}

		virtual Results CalculateResults() const override {
			Results ret;
			return ret;
		}

		virtual size_t GetMaxStartPositions() const { return 0; }

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const;
		virtual const std::string ToHTML() const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	private:
	};
}