#pragma once
#include "matchtable.h"



namespace Judoboard
{
	class Pause : public MatchTable
	{
		friend class Tournament;

	public:
		Pause(const ITournament* Tournament) : MatchTable(Tournament) {}
		Pause(ZED::CSV& Stream, ITournament* Tournament) : MatchTable(Stream, Tournament) {}

		static std::string GetHTMLForm() { return ""; }

		virtual Type GetType() const { return Type::Pause; }
		virtual const std::string GetName() const override { return "Pause"; }

		virtual bool IsElgiable(const Judoka& Fighter) const override { return false; }
		virtual void GenerateSchedule() override {}

		//Serialization
		const std::string ToHTML() const override { return ""; }//DEBUG
		const std::string ToString() const override { return ""; }//DEBUG

	private:
		Pause(uint32_t ID, std::stringstream Stream);
	};
}