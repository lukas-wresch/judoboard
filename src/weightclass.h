#pragma once
#include "matchtable.h"



namespace Judoboard
{
	class Weightclass : public MatchTable
	{
		friend class Tournament;

	public:
		Weightclass(const Tournament* Tournament, uint16_t MinWeight, uint16_t MaxWeight);
		Weightclass(const Tournament* Tournament, uint16_t MinWeight, uint16_t MaxWeight, Gender Gender);
		Weightclass(ZED::CSV& Stream, const Tournament* Tournament);

		static std::string GetHTMLForm();

		//WeightClass(WeightClass&& rhs) noexcept : MatchTable(std::move(rhs)) {}

		Type GetType() const override { return Type::Weightclass; }

		const std::string GetDescription() const;
		virtual Status GetStatus() const override;

		void SetMinWeight(uint16_t MinWeight) { m_MinWeight = MinWeight; }
		void SetMaxWeight(uint16_t MaxWeight) { m_MaxWeight = MaxWeight; }
		void SetGender(Gender Gender) { m_Gender = Gender; m_GenderEnforced = true; }
		void EnforceGender(bool Enabled = true) { m_GenderEnforced = Enabled; }

		bool IsElgiable(const Judoka& Fighter) const override;
		void GenerateSchedule() override;

		//Serialization
		const std::string ToHTML() const override;
		const std::string ToString() const override;

		void operator >> (ZED::CSV& Stream) const override;


	private:
		uint16_t m_MinWeight = 10;
		uint16_t m_MaxWeight = 100;

		uint16_t m_MinAge = 0;
		uint16_t m_MaxAge = 100;
		bool m_AgeEnforced = false;

		Gender m_Gender = Gender::Male;
		bool m_GenderEnforced = false;
	};
}