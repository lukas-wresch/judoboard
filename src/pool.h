#pragma once
#include "weightclass.h"
#include "round_robin.h"
#include "single_elimination.h"



namespace Judoboard
{
	class Pool : public MatchTable
	{
		friend class Tournament;

	public:
		Pool(IFilter* Filter, const ITournament* Tournament = nullptr) : MatchTable(Filter, Tournament), m_Finals(nullptr, Tournament) {}
		Pool(Weight MinWeight, Weight MaxWeight, Gender Gender = Gender::Unknown, const ITournament* Tournament = nullptr);
		Pool(const YAML::Node& Yaml, ITournament* Tournament = nullptr);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Pool; }

		virtual size_t GetMaxStartPositions() const override;

		virtual Results CalculateResults() const override {Results ret; return ret;};//DUMMY
		virtual void GenerateSchedule() override;

		bool IsThirdPlaceMatch() const { return m_Finals.IsThirdPlaceMatch(); }
		bool IsFifthPlaceMatch() const { return m_Finals.IsFifthPlaceMatch(); }

		void IsThirdPlaceMatch(bool Enable) { m_Finals.IsThirdPlaceMatch(Enable); GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_Finals.IsFifthPlaceMatch(Enable); GenerateSchedule(); }

		void SetPoolCount(uint32_t PoolCount) { m_PoolCount = PoolCount; GenerateSchedule(); }
		size_t GetPoolCount() const { return m_Pools.size(); }

		const RoundRobin* GetPool(size_t Index) const {
			if (Index >= m_Pools.size())
				return nullptr;
			return m_Pools[Index];
		}

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	private:
		size_t CalculatePoolCount() const {
			size_t pool_count = m_PoolCount;
			if (m_PoolCount == 0)//Auto
			{
				pool_count = 2;
				if (GetFilter()->GetParticipants().size() > 10)
					pool_count = 4;
			}
			return pool_count;
		}

		uint32_t m_PoolCount = 0;//0 for auto, otherwise number of pools

		std::vector<RoundRobin*> m_Pools;
		SingleElimination m_Finals;
	};
}