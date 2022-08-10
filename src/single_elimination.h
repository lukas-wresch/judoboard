#pragma once
#include "weightclass.h"



namespace Judoboard
{
	class SingleElimination : public Weightclass
	{
		friend class Tournament;

	public:
		SingleElimination(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament = nullptr);
		SingleElimination(const YAML::Node& Yaml, ITournament* Tournament);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::SingleElimination; }

		virtual bool AddParticipant(Judoka* NewParticipant, bool Force = false) override;
		virtual void RemoveAllParticipants() override {
			MatchTable::RemoveAllParticipants();
			m_StartingPositions.clear();
		}

		virtual std::vector<Result> CalculateResults() const override;
		virtual void GenerateSchedule() override;

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	protected:
		Judoka* GetJudokaByStartPosition(size_t StartPosition) const
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
		size_t GetNumberOfRounds() const {
			if (GetParticipants().size() == 0)
				return 0;

			return (size_t)std::floor(std::log2(GetParticipants().size()));
		}

		std::unordered_map<size_t, Judoka*> m_StartingPositions;

		bool m_ThirdPlaceMatch = false;
		bool m_FifthPlaceMatch = false;
	};
}