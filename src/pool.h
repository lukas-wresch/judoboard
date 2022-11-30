#pragma once
#include "weightclass.h"
#include "single_elimination.h"



namespace Judoboard
{
	class Pool : public Weightclass
	{
		friend class Tournament;

	public:
		Pool(Weight MinWeight, Weight MaxWeight, const ITournament* Tournament = nullptr);
		Pool(const YAML::Node& Yaml, ITournament* Tournament = nullptr);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::SingleElimination; }

		virtual bool AddParticipant(const Judoka* NewParticipant, bool Force = false) override;
		virtual void RemoveAllParticipants() override {
			MatchTable::RemoveAllParticipants();
			m_StartingPositions.clear();
		}

		virtual std::vector<Result> CalculateResults() const override {std::vector<Result> ret; return ret;};//DUMMY
		virtual void GenerateSchedule() override;

		bool IsThirdPlaceMatch() const { return m_Finals.IsThirdPlaceMatch(); }
		bool IsFifthPlaceMatch() const { return m_Finals.IsFifthPlaceMatch(); }

		void IsThirdPlaceMatch(bool Enable) { m_Finals.IsThirdPlaceMatch(Enable); GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_Finals.IsFifthPlaceMatch(Enable); GenerateSchedule(); }

		virtual size_t GetStartingPosition(const Judoka* Judoka) const override;
		virtual void   SetStartingPosition(const Judoka* Judoka, size_t NewStartingPosition) override;

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	protected:
		const Judoka* GetJudokaByStartPosition(size_t StartPosition)
		{
			auto result = m_StartingPositions.find(StartPosition);
			if (result == m_StartingPositions.end())
				return nullptr;
			return result->second;
		}
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
		std::unordered_map<size_t, const Judoka*> m_StartingPositions;
		int m_PoolCount = 2;

		std::vector<Weightclass*> m_Pools;
		SingleElimination m_Finals;
	};
}