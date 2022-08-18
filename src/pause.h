#pragma once
#include "matchtable.h"



namespace Judoboard
{
	class Pause : public MatchTable
	{
		friend class Tournament;

	public:
		Pause(const ITournament* Tournament) : MatchTable(Tournament) {}
		Pause(YAML::Node& Yaml, ITournament* Tournament) : MatchTable(Yaml, Tournament) {}

		static std::string GetHTMLForm() { return ""; }

		virtual Type GetType() const { return Type::Pause; }
		virtual std::string GetDescription() const override { return "Pause"; }

		virtual bool IsElgiable(const Judoka& Fighter) const override { return false; }
		virtual void GenerateSchedule() override {}

		//Serialization
		const std::string ToHTML() const override { return ""; }//DEBUG

	private:
		Pause(uint32_t ID, std::stringstream Stream);
	};
}