#pragma once
#include "matchtable.h"



namespace Judoboard
{
	class CustomTable : public MatchTable
	{
		friend class Tournament;

	public:
		CustomTable(const ITournament* Tournament);
		CustomTable(ZED::CSV& Stream, ITournament* Tournament);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Custom; }
		virtual std::string GetDescription() const override { return GetName() + " Custom"; }

		virtual Status GetStatus() const override;

		virtual std::vector<Result> CalculateResults() const override;

		virtual bool IsElgiable(const Judoka& Fighter) const override { return false; }//Don't add judoka automatically
		virtual void GenerateSchedule() override {}

		//Serialization
		virtual const std::string ToHTML() const override;
		virtual const std::string ToString() const override;

		virtual void operator >> (ZED::CSV& Stream) const override;


	private:
	};
}