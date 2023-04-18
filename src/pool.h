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
		Pool(IFilter* Filter, const ITournament* Tournament = nullptr);
		Pool(Weight MinWeight, Weight MaxWeight, Gender Gender = Gender::Unknown, const ITournament* Tournament = nullptr);
		Pool(const YAML::Node& Yaml, ITournament* Tournament = nullptr, const MatchTable* Parent = nullptr);

		~Pool() {
			SetSchedule().clear();
			for (auto pool : m_Pools)
				delete pool;
		}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Pool; }

		virtual size_t GetMaxStartPositions() const override;

		virtual Results CalculateResults() const override { return m_Finals.CalculateResults(); }
		virtual void GenerateSchedule() override;

		virtual Match* FindMatch(const UUID& UUID) const override;
		virtual const MatchTable* FindMatchTable(const UUID& UUID) const override;

		bool IsThirdPlaceMatch() const { return m_Finals.IsThirdPlaceMatch(); }
		bool IsFifthPlaceMatch() const { return m_Finals.IsFifthPlaceMatch(); }

		void IsThirdPlaceMatch(bool Enable) { m_Finals.IsThirdPlaceMatch(Enable); GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_Finals.IsFifthPlaceMatch(Enable); GenerateSchedule(); }

		void SetPoolCount(uint32_t PoolCount) { m_PoolCount = PoolCount; GenerateSchedule(); }
		size_t GetPoolCount() const { return m_Pools.size(); }

		RoundRobin* GetPool(size_t Index) {
			if (Index >= m_Pools.size())
				return nullptr;
			return m_Pools[Index];
		}

		const RoundRobin* GetPool(size_t Index) const {
			if (Index >= m_Pools.size())
				return nullptr;
			return m_Pools[Index];
		}

		SingleElimination& GetFinals() { return m_Finals; }
		const SingleElimination& GetFinals() const { return m_Finals; }

		auto GetTakeTop() const { return m_TakeTop; }
		void SetTakeTop(uint32_t NumJudoka) { m_TakeTop = NumJudoka; GenerateSchedule(); }

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	protected:
		virtual void DeleteSchedule() override {
			for (auto pool : m_Pools)
				pool->DeleteSchedule();
			m_Finals.DeleteSchedule();
			MatchTable::DeleteSchedule();
		}


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

		void CopyMatchesFromSubtables();

		uint32_t m_PoolCount = 0;//0 for auto, otherwise number of pools
		uint32_t m_TakeTop   = 2;//Number of judoka to go to the next round from each pool

		std::vector<RoundRobin*> m_Pools;
		SingleElimination m_Finals;
	};
}