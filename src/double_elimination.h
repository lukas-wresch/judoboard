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
		DoubleElimination(const YAML::Node& Yaml, ITournament* Tournament, const MatchTable* Parent = nullptr);
		DoubleElimination(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr)
			: DoubleElimination(new Weightclass(Weightclass_, this), Tournament) {}

		~DoubleElimination() {
			DeleteSchedule();
		}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::DoubleElimination; }

		virtual size_t GetMaxStartPositions() const override;

		virtual void SetMatID(int32_t MatID) override {
			MatchTable::SetMatID(MatID);
			m_WinnerBracket.SetMatID(MatID);
			m_LoserBracket.SetMatID(MatID);
		}

		virtual Results CalculateResults() const override { return m_WinnerBracket.CalculateResults(); }
		virtual void GenerateSchedule() override;

		bool IsThirdPlaceMatch() const { return m_WinnerBracket.IsThirdPlaceMatch(); }
		bool IsFifthPlaceMatch() const { return m_WinnerBracket.IsFifthPlaceMatch(); }

		void IsThirdPlaceMatch(bool Enable) { m_WinnerBracket.IsThirdPlaceMatch(Enable); SetSchedule().clear(); GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_WinnerBracket.IsFifthPlaceMatch(Enable); SetSchedule().clear(); GenerateSchedule(); }

		SingleElimination& GetWinnerBracket() { return m_WinnerBracket; }
		const SingleElimination& GetWinnerBracket() const { return m_WinnerBracket; }

		SingleElimination& GetLoserBracket() { return m_LoserBracket; }
		const SingleElimination& GetLoserBracket() const { return m_LoserBracket; }

		void AddMatchToWinnerBracket(Match* NewMatch);
		void AddMatchToLoserBracket(Match* NewMatch);

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	protected:
		virtual void DeleteSchedule() override {
			m_WinnerBracket.DeleteSchedule();
			m_LoserBracket.DeleteSchedule();
			MatchTable::DeleteSchedule();
		}

	private:
		SingleElimination m_WinnerBracket;
		LoserBracket      m_LoserBracket;
	};
}