#pragma once
#include "weightclass.h"



namespace Judoboard
{
	class SingleElimination : public Weightclass
	{
		friend class Tournament;

	public:
		SingleElimination(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament = nullptr);
		SingleElimination(const YAML::Node& Yaml, ITournament* Tournament = nullptr);
		SingleElimination(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::SingleElimination; }

		virtual bool AddParticipant(Judoka* NewParticipant, bool Force = false) override;
		virtual bool RemoveParticipant(const Judoka* Participant) override;
		virtual void RemoveAllParticipants() override {
			MatchTable::RemoveAllParticipants();
			m_StartingPositions.clear();
		}

		virtual std::vector<Result> CalculateResults() const override;
		virtual void GenerateSchedule() override;

		bool IsThirdPlaceMatch() const { return m_ThirdPlaceMatch; }
		bool IsFifthPlaceMatch() const { return m_FifthPlaceMatch; }

		void IsThirdPlaceMatch(bool Enable) { m_ThirdPlaceMatch = Enable; GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_FifthPlaceMatch = Enable; GenerateSchedule(); }

		virtual size_t GetStartingPosition(const Judoka* Judoka) const override;
		virtual void   SetStartingPosition(const Judoka* Judoka, size_t NewStartingPosition) override;

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	protected:
		const Judoka* GetJudokaByStartPosition(size_t StartPosition) const
		{
			auto result = m_StartingPositions.find(StartPosition);
			if (result == m_StartingPositions.end())
				return nullptr;
			return result->second;
		}

		bool IsStartPositionTaken(size_t StartPosition) const
		{
			return m_StartingPositions.find(StartPosition) != m_StartingPositions.end();
		}


	private:
		void FindFreeStartPos(const Judoka* NewParticipant);
		size_t GetMaxStartPos() const {
			const auto rounds = GetNumberOfRounds();
			return (int)pow(2, rounds);
		}

		size_t GetNumberOfRounds() const {
			if (GetParticipants().size() == 0)
				return 0;

			return (size_t)std::ceil(std::log2(GetParticipants().size()));
		}

		std::unordered_map<size_t, const Judoka*> m_StartingPositions;

		bool m_ThirdPlaceMatch = false;
		bool m_FifthPlaceMatch = false;
	};
}