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
		DoubleElimination(const ITournament* Tournament = nullptr);
		DoubleElimination(Weight MinWeight, Weight MaxWeight, Gender Gender = Gender::Unknown, const ITournament* Tournament = nullptr);
		DoubleElimination(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr)
			: DoubleElimination(Tournament) {
			SetFilter(std::make_shared<Weightclass>(Weightclass_, this));
		}

		virtual void LoadYaml(const YAML::Node& Yaml) override;

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::DoubleElimination; }

		virtual size_t GetMaxStartPositions() const override;

		virtual void SetMatID(int32_t MatID) override {
			MatchTable::SetMatID(MatID);
			m_WinnerBracket->SetMatID(MatID);
			m_LoserBracket->SetMatID(MatID);
		}

		virtual std::shared_ptr<Match> FindMatch(const UUID& UUID) const override;

		virtual bool DeleteMatch(const UUID& UUID) override;

		virtual Results CalculateResults() const override;
		virtual size_t ResultsCount() const override {
			if (IsThirdPlaceMatch() && IsFifthPlaceMatch())
				return std::min((size_t)6, GetParticipants().size());
			else if (!IsThirdPlaceMatch() && !IsFifthPlaceMatch())
				return std::min((size_t)2, GetParticipants().size());
			return std::min((size_t)4, GetParticipants().size());
		}
		virtual void GenerateSchedule() override;

		bool IsThirdPlaceMatch() const { return m_LoserBracket->IsFinalMatch(); }
		bool IsFifthPlaceMatch() const { return m_LoserBracket->IsThirdPlaceMatch(); }

		void IsThirdPlaceMatch(bool Enable) { m_LoserBracket->IsFinalMatch(Enable); SetSchedule().clear(); GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_LoserBracket->IsThirdPlaceMatch(Enable); SetSchedule().clear(); GenerateSchedule(); }

		auto GetWinnerBracket() { return m_WinnerBracket; }
		auto GetWinnerBracket() const { return m_WinnerBracket; }

		auto GetLoserBracket() { return m_LoserBracket; }
		auto GetLoserBracket() const { return m_LoserBracket; }

		void AddMatchToWinnerBracket(std::shared_ptr<Match> NewMatch);
		void AddMatchToLoserBracket( std::shared_ptr<Match> NewMatch);

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	protected:
		void BuildSchedule();
		virtual void DeleteSchedule() override {
			m_WinnerBracket->DeleteSchedule();
			m_LoserBracket->DeleteSchedule();
			MatchTable::DeleteSchedule();
		}

	private:
		std::shared_ptr<SingleElimination> m_WinnerBracket;
		std::shared_ptr<LoserBracket>      m_LoserBracket;
	};
}