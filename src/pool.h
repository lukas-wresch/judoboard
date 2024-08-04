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
		Pool(std::shared_ptr<IFilter> Filter, const ITournament* Tournament = nullptr);
		Pool(Weight MinWeight, Weight MaxWeight, Gender Gender = Gender::Unknown, const ITournament* Tournament = nullptr);
		Pool(const YAML::Node& Yaml, const ITournament* Tournament = nullptr, const MatchTable* Parent = nullptr);
		Pool(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr)
			: Pool(std::make_shared<Weightclass>(Weightclass_, this), Tournament) {}

		~Pool() {
			SetSchedule().clear();
		}

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Pool; }

		virtual size_t GetMaxStartPositions() const override;

		virtual Results CalculateResults() const override { return m_Finals->CalculateResults(); }
		virtual size_t ResultsCount() const override  { return m_Finals->ResultsCount(); }
		virtual void GenerateSchedule() override;

		virtual std::shared_ptr<Match> FindMatch(const UUID& UUID) const override;
		virtual std::shared_ptr<const MatchTable> FindMatchTable(const UUID& UUID) const override;

		virtual bool DeleteMatch(const UUID& UUID) override;

		bool IsThirdPlaceMatch() const { return m_Finals->IsThirdPlaceMatch(); }
		bool IsFifthPlaceMatch() const { return m_Finals->IsFifthPlaceMatch(); }

		void IsThirdPlaceMatch(bool Enable) { m_Finals->IsThirdPlaceMatch(Enable); SetSchedule().clear(); GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_Finals->IsFifthPlaceMatch(Enable); SetSchedule().clear(); GenerateSchedule(); }

		void SetPoolCount(uint32_t PoolCount) { m_PoolCount = PoolCount; GenerateSchedule(); }
		size_t GetPoolCount() const { return m_Pools.size(); }

		std::shared_ptr<RoundRobin> GetPool(size_t Index) {
			if (Index >= m_Pools.size())
				return nullptr;
			return m_Pools[Index];
		}

		std::shared_ptr<const RoundRobin> GetPool(size_t Index) const {
			if (Index >= m_Pools.size())
				return nullptr;
			return m_Pools[Index];
		}

		std::shared_ptr<SingleElimination> GetFinals() { return m_Finals; }
		std::shared_ptr<const SingleElimination> GetFinals() const { return m_Finals; }

		auto GetTakeTop() const { return m_TakeTop; }
		void SetTakeTop(uint32_t NumJudoka) { m_TakeTop = NumJudoka; GenerateSchedule(); }

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	protected:
		virtual void DeleteSchedule() override {
			for (auto pool : m_Pools)
				if (pool)
					pool->DeleteSchedule();
			if (m_Finals)
				m_Finals->DeleteSchedule();
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

		std::vector<std::shared_ptr<RoundRobin>> m_Pools;
		std::shared_ptr<SingleElimination> m_Finals;
	};
}