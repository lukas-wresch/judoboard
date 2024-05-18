#pragma once
#include "judoboard.h"
#include "matchtable.h"
#include "weightclass.h"



namespace Judoboard
{
	class SingleElimination : public MatchTable
	{
		friend class Tournament;

	public:
		SingleElimination(IFilter* Filter, const ITournament* Tournament = nullptr, const MatchTable* Parent = nullptr);
		SingleElimination(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament = nullptr);
		SingleElimination(const YAML::Node& Yaml, const ITournament* Tournament, const MatchTable* Parent = nullptr);
		SingleElimination(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr)
			: SingleElimination(new Weightclass(Weightclass_, this), Tournament) {}

		void operator =(const SingleElimination& rhs) = delete;
		void operator =(SingleElimination&& rhs) noexcept {
			ID::operator=(rhs);
			m_ThirdPlaceMatch = rhs.m_ThirdPlaceMatch;
			m_FifthPlaceMatch = rhs.m_FifthPlaceMatch;
			SetRuleSet(rhs.GetOwnRuleSet());
			SetName(rhs.GetName());
			SetFilter(rhs.GetFilter());
			SetTournament(rhs.GetTournament());
			SetScheduleIndex(rhs.GetScheduleIndex());
			SetMatID(rhs.GetMatID());
			SetColor(rhs.GetColor());

			SetParent(rhs.GetParent());
			DeleteSchedule();
			IsBestOfThree(rhs.IsBestOfThree());

			SetSchedule(std::move(rhs.SetSchedule()));
			m_ThirdPlaceMatches = std::move(rhs.m_ThirdPlaceMatches);
			m_FifthPlaceMatches = std::move(rhs.m_FifthPlaceMatches);

			for (auto match : GetSchedule())
				match->SetMatchTable(this);
		}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::SingleElimination; }

		virtual size_t GetMaxStartPositions() const override {
			const auto rounds = GetNumberOfRounds();
			return (int)pow(2, rounds);
		}

		virtual void DeleteSchedule() override {
			if (!IsSubMatchTable())
			{
				for (auto match : m_ThirdPlaceMatches)
					delete match;
				for (auto match : m_FifthPlaceMatches)
					delete match;
			}
			m_ThirdPlaceMatches.clear();
			m_FifthPlaceMatches.clear();
			MatchTable::DeleteSchedule();
		}

		virtual bool DeleteMatch(const UUID& UUID) override;

		virtual const std::vector<Match*> GetSchedule() const override;

		virtual Results CalculateResults() const override;
		virtual size_t ResultsCount() const override {
			if (IsThirdPlaceMatch() && IsFifthPlaceMatch())
				return std::min((size_t)6, GetParticipants().size());
			else if (!IsThirdPlaceMatch() && !IsFifthPlaceMatch())
				return std::min((size_t)2, GetParticipants().size());
			return std::min((size_t)4, GetParticipants().size());
		}
		virtual void GenerateSchedule() override;

		virtual bool AddMatch(Match* NewMatch) override;

		bool IsThirdPlaceMatch() const { return m_ThirdPlaceMatch; }
		bool IsFifthPlaceMatch() const { return m_FifthPlaceMatch; }

		void IsThirdPlaceMatch(bool Enable) { m_ThirdPlaceMatch = Enable; GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_FifthPlaceMatch = Enable; GenerateSchedule(); }
    
		size_t GetNumberOfRounds() const;

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	protected:
		std::string RenderMatch(const Match& Match, const std::string& Style = "") const;

		std::vector<Match*> m_ThirdPlaceMatches;
		std::vector<Match*> m_FifthPlaceMatches;

		bool m_ThirdPlaceMatch = false;
		bool m_FifthPlaceMatch = false;
	};
}