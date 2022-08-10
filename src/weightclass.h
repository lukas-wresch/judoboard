#pragma once
#include "matchtable.h"
#include "md5.h"



namespace Judoboard
{
	class Weightclass : public MatchTable
	{
		friend class Tournament;

	public:
		Weightclass(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament = nullptr);
		Weightclass(Weight MinWeight, Weight MaxWeight, Gender Gender, const ITournament* Tournament = nullptr);
		Weightclass(const YAML::Node& Yaml, ITournament* Tournament = nullptr);
		Weightclass(const MD5::Weightclass& Weightclass, const ITournament* Tournament = nullptr);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Weightclass; }

		virtual std::string GetDescription() const override;
		virtual Status GetStatus() const override;

		virtual std::vector<Result> CalculateResults() const override;
		virtual bool IsElgiable(const Judoka& Fighter) const override;
		virtual void GenerateSchedule() override;

		auto GetMinWeight() const { return m_MinWeight; }
		auto GetMaxWeight() const { return m_MaxWeight; }
		auto GetGender()    const { return m_Gender; }
		void SetMinWeight(Weight MinWeight) { m_MinWeight = MinWeight; }
		void SetMaxWeight(Weight MaxWeight) { m_MaxWeight = MaxWeight; }
		void SetGender(Gender Gender) { m_Gender = Gender; }
		auto IsBestOfThree() const { return m_BestOfThree; }
		void IsBestOfThree(bool Enable) { m_BestOfThree = Enable; }

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	private:
		Weight m_MinWeight = 10*1000;//In gramms
		Weight m_MaxWeight = 100*1000;//In gramms

		Gender m_Gender = Gender::Unknown;
		bool m_BestOfThree = false;
	};
}