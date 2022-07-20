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
		Weightclass(const YAML::Node& Yaml, ITournament* Tournament);
		Weightclass(const MD5::Weightclass& Weightclass, const ITournament* Tournament);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Weightclass; }

		virtual std::string GetDescription() const override;
		virtual Status GetStatus() const override;

		virtual std::vector<Result> CalculateResults() const override;

		auto GetMinWeight() const { return m_MinWeight; }
		auto GetMaxWeight() const { return m_MaxWeight; }
		auto GetGender()    const { return m_Gender; }
		void SetMinWeight(Weight MinWeight) { m_MinWeight = MinWeight; }
		void SetMaxWeight(Weight MaxWeight) { m_MaxWeight = MaxWeight; }
		void SetGender(Gender Gender) { m_Gender = Gender; }

		virtual bool IsElgiable(const Judoka& Fighter) const override;
		void GenerateSchedule() override;

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	private:
		Weight m_MinWeight = 10*1000;//In gramms
		Weight m_MaxWeight = 100*1000;//In gramms

		Gender m_Gender = Gender::Unknown;
	};
}