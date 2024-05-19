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
		DoubleElimination(const YAML::Node& Yaml, const ITournament* Tournament, const MatchTable* Parent = nullptr);
		DoubleElimination(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr)
			: DoubleElimination(new Weightclass(Weightclass_, this), Tournament) {}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::DoubleElimination; }

		virtual size_t GetMaxStartPositions() const override;

		virtual void SetMatID(int32_t MatID) override {
			MatchTable::SetMatID(MatID);
			m_WinnerBracket.SetMatID(MatID);
			m_LoserBracket.SetMatID(MatID);
		}

		virtual Match* FindMatch(const UUID& UUID) const override;

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

		virtual Delivery GetMatchParcels() const override;

		bool IsThirdPlaceMatch() const { return m_LoserBracket.IsFinalMatch(); }
		bool IsFifthPlaceMatch() const { return m_LoserBracket.IsThirdPlaceMatch(); }

		void IsThirdPlaceMatch(bool Enable) { m_LoserBracket.IsFinalMatch(Enable); SetSchedule().clear(); GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_LoserBracket.IsThirdPlaceMatch(Enable); SetSchedule().clear(); GenerateSchedule(); }

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
		void BuildSchedule();
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