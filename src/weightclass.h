#pragma once
#include "matchtable.h"
#include "md5.h"



namespace Judoboard
{
	class Weightclass : public MatchTable
	{
		friend class Tournament;

	public:
		Weightclass(const ITournament* Tournament, uint16_t MinWeight, uint16_t MaxWeight);
		Weightclass(const ITournament* Tournament, uint16_t MinWeight, uint16_t MaxWeight, Gender Gender);
		Weightclass(ZED::CSV& Stream, ITournament* Tournament);
		Weightclass(const YAML::Node& Yaml, ITournament* Tournament);
		Weightclass(const MD5::Weightclass& Weightclass, const ITournament* Tournament);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Weightclass; }

		const std::string GetDescription() const;
		virtual Status GetStatus() const override;

		virtual std::vector<Result> CalculateResults() const override;

		void SetMinWeight(uint16_t MinWeight) { m_MinWeight = MinWeight; }
		void SetMaxWeight(uint16_t MaxWeight) { m_MaxWeight = MaxWeight; }
		void SetGender(Gender Gender) { m_Gender = Gender; }

		virtual bool IsElgiable(const Judoka& Fighter) const override;
		void GenerateSchedule() override;

		//Serialization
		virtual const std::string ToHTML() const override;
		virtual const std::string ToString() const override;

		virtual void operator >> (ZED::CSV& Stream) const override;
		virtual void operator >> (YAML::Emitter& Yaml) const override;


	private:
		uint16_t m_MinWeight = 10;
		uint16_t m_MaxWeight = 100;

		Gender m_Gender = Gender::Unknown;
	};
}