#pragma once
#include "weightclass.h"
#include "single_elimination.h"
#include "loser_bracket.h"



namespace Judoboard
{
	class DoubleElimination : public MatchTable
	{
		friend class Tournament;

	public:
		DoubleElimination(IFilter* Filter, const ITournament* Tournament = nullptr);
		DoubleElimination(Weight MinWeight, Weight MaxWeight, Gender Gender = Gender::Unknown, const ITournament* Tournament = nullptr);
		DoubleElimination(const YAML::Node& Yaml, ITournament* Tournament = nullptr);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::DoubleElimination; }

		virtual size_t GetMaxStartPositions() const override;

		virtual Results CalculateResults() const override { return m_WinnerBracket.CalculateResults(); }
		virtual void GenerateSchedule() override;

		bool IsThirdPlaceMatch() const { return m_WinnerBracket.IsThirdPlaceMatch(); }
		bool IsFifthPlaceMatch() const { return m_WinnerBracket.IsFifthPlaceMatch(); }

		void IsThirdPlaceMatch(bool Enable) { m_WinnerBracket.IsThirdPlaceMatch(Enable); GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_WinnerBracket.IsFifthPlaceMatch(Enable); GenerateSchedule(); }

		SingleElimination& GetWinnerBracket() { return m_WinnerBracket; }
		const SingleElimination& GetWinnerBracket() const { return m_WinnerBracket; }

		SingleElimination& GetLoserBracket() { return m_LoserBracket; }
		const SingleElimination& GetLoserBracket() const { return m_LoserBracket; }

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	private:
		SingleElimination m_WinnerBracket;
		LoserBracket      m_LoserBracket;
	};
}