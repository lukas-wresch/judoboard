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
		LoserBracket(IFilter* Filter, const ITournament* Tournament = nullptr, const MatchTable* Parent = nullptr);
		LoserBracket(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament = nullptr);
		LoserBracket(const YAML::Node& Yaml, const ITournament* Tournament = nullptr, const MatchTable* Parent = nullptr);
		LoserBracket(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr);

		void operator =(const LoserBracket& rhs) = delete;
		void operator =(LoserBracket&& rhs) noexcept {
			ID::operator=(rhs);
			m_ThirdPlaceMatch = rhs.m_ThirdPlaceMatch;
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
			for (auto match : GetSchedule())
				match->SetMatchTable(this);
		}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::LoserBracket; }

		virtual size_t GetMaxStartPositions() const override {
			if (!GetFilter())
				return 0;
			if (GetNumberOfBaseRounds() == 1)
				return 2;
			return (size_t)pow(2, GetNumberOfBaseRounds()) - 2;
		}

		virtual Results CalculateResults() const override;
		virtual void GenerateSchedule() override;

		bool IsFinalMatch() const { return m_FinalMatch; }
		bool IsThirdPlaceMatch() const { return m_ThirdPlaceMatch; }
		//bool IsFifthPlaceMatch() const { return m_FifthPlaceMatch; }

		void IsFinalMatch(bool Enable) { m_FinalMatch = Enable; GenerateSchedule(); }
		void IsThirdPlaceMatch(bool Enable) { m_ThirdPlaceMatch = Enable; GenerateSchedule(); }
		//void IsFifthPlaceMatch(bool Enable) { m_FifthPlaceMatch = Enable; GenerateSchedule(); }

		//Serialization
		virtual const std::string ToHTML() const override;


	private:
		size_t GetNumberOfRounds() const
		{
			if (!GetFilter() || GetFilter()->GetParticipants().size() <= 1)
				return 0;

			if (GetFilter()->GetParticipants().size() == 2)
				return 1;

			const auto max_initial_start_pos = (GetMaxStartPositions() + 2) / 2;

			size_t no_matches = max_initial_start_pos;
			size_t round = 0;
			while (no_matches > 2)
			{
				if (round%2 == 0)
					no_matches /= 2;
				round++;
			}

			if (IsFinalMatch())
				round++;

			return round + 1;
		}

		size_t GetNumberOfBaseRounds() const
		{
			auto filter = GetFilter()
			if (!filter || filter->GetParticipants().size() <= 1)
				return 0;

			if (filter->GetParticipants().size() == 2)
				return 1;

			auto rounds = (size_t)std::ceil(std::log2(filter->GetParticipants().size() + 2));

			return rounds;
		}

		bool m_FinalMatch = false;
	};
}