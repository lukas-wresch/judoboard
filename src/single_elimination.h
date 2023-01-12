#pragma once
#include "judoboard.h"
#include "matchtable.h"



namespace Judoboard
{
	class SingleElimination : public MatchTable
	{
		friend class Tournament;

	public:
		SingleElimination(IFilter* Filter, const ITournament* Tournament = nullptr);
		SingleElimination(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament = nullptr);
		SingleElimination(const YAML::Node& Yaml, const ITournament* Tournament = nullptr);
		SingleElimination(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr);

		void operator =(const SingleElimination& rhs) = delete;
		void operator =(SingleElimination&& rhs) noexcept {
			m_ThirdPlaceMatch = rhs.m_ThirdPlaceMatch;
			m_FifthPlaceMatch = rhs.m_FifthPlaceMatch;
			SetRuleSet(rhs.GetOwnRuleSet());
			SetName(rhs.GetName());
			SetFilter(rhs.GetFilter());
			SetTournament(rhs.GetTournament());
			SetScheduleIndex(rhs.GetScheduleIndex());
			SetMatID(rhs.GetMatID());
			SetColor(rhs.GetColor());
			SetSchedule() = std::move(rhs.SetSchedule());
			IsBestOfThree(rhs.IsBestOfThree());
		}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::SingleElimination; }

		virtual size_t GetMaxStartPositions() const override {
			const auto rounds = GetNumberOfRounds();
			return (int)pow(2, rounds);
		}

		virtual Results CalculateResults() const override;
		virtual void GenerateSchedule() override;

		bool IsThirdPlaceMatch() const { return m_ThirdPlaceMatch; }
		bool IsFifthPlaceMatch() const { return m_FifthPlaceMatch; }

		void IsThirdPlaceMatch(bool Enable) { m_ThirdPlaceMatch = Enable; GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_FifthPlaceMatch = Enable; GenerateSchedule(); }

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	protected:
		std::string RenderMatch(const Match& match, std::string style = "") const;

	private:
		size_t GetNumberOfRounds() const;

		bool m_ThirdPlaceMatch = false;
		bool m_FifthPlaceMatch = false;
	};
}