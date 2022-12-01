#pragma once
#include "filter.h"
#include "md5.h"



namespace Judoboard
{
	class Weightclass : public IFilter
	{
		friend class Tournament;

	public:
		Weightclass(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament = nullptr);
		Weightclass(Weight MinWeight, Weight MaxWeight, Gender Gender, const ITournament* Tournament = nullptr);
		Weightclass(const YAML::Node& Yaml, ITournament* Tournament = nullptr);
		Weightclass(const MD5::Weightclass& Weightclass, const ITournament* Tournament = nullptr);

		virtual Type GetType() const override { return Type::Weightclass; }

		virtual std::string GetHTMLForm() const;

		virtual bool IsElgiable(const Judoka& Fighter) const override;

		std::string GetDescription() const;

		auto GetMinWeight() const { return m_MinWeight; }
		auto GetMaxWeight() const { return m_MaxWeight; }
		auto GetGender()    const { return m_Gender; }
		void SetMinWeight(Weight MinWeight) { m_MinWeight = MinWeight; }
		void SetMaxWeight(Weight MaxWeight) { m_MaxWeight = MaxWeight; }
		void SetGender(Gender Gender) { m_Gender = Gender; }

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	private:
		Weight m_MinWeight = 10*1000;//In gramms
		Weight m_MaxWeight = 100*1000;//In gramms

		Gender m_Gender = Gender::Unknown;

		const ITournament* m_Tournament = nullptr;
	};
}