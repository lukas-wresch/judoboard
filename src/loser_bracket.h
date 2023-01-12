#pragma once
#include "judoboard.h"
#include "filter.h"
#include "single_elimination.h"



namespace Judoboard
{
	class LoserBracket : public SingleElimination
	{
		friend class Tournament;

	public:
		LoserBracket(IFilter* Filter, const ITournament* Tournament = nullptr);
		LoserBracket(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament = nullptr);
		LoserBracket(const YAML::Node& Yaml, const ITournament* Tournament = nullptr);
		LoserBracket(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr);

		void operator =(const LoserBracket& rhs) = delete;

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::LoserBracket; }

		virtual size_t GetMaxStartPositions() const override {
			if (!GetFilter())
				return 0;
			return (size_t)pow(2, GetNumberOfBaseRounds()) - 2;
		}

		virtual Results CalculateResults() const override;
		virtual void GenerateSchedule() override;

		bool IsThirdPlaceMatch() const { return m_ThirdPlaceMatch; }
		bool IsFifthPlaceMatch() const { return m_FifthPlaceMatch; }

		void IsThirdPlaceMatch(bool Enable) { m_ThirdPlaceMatch = Enable; GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_FifthPlaceMatch = Enable; GenerateSchedule(); }

		//Serialization
		virtual const std::string ToHTML() const override;


	private:
		size_t GetNumberOfRounds() const;

		size_t GetNumberOfBaseRounds() const
		{
			if (!GetFilter() || GetFilter()->GetParticipants().size() == 0)
				return 0;

			return (size_t)std::ceil(std::log2(GetFilter()->GetParticipants().size() + 2));
		}

		bool m_ThirdPlaceMatch = false;
		bool m_FifthPlaceMatch = false;
	};
}