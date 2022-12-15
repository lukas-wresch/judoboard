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
		Pool(Weight MinWeight, Weight MaxWeight, Gender Gender, const ITournament* Tournament = nullptr);
		Pool(const YAML::Node& Yaml, ITournament* Tournament = nullptr);

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::Pool; }

		virtual size_t GetMaxStartPositions() const override {
			size_t sum = 0;
			for (auto pool : m_Pools)
				sum += pool->GetMaxStartPositions();
			return sum;
		}

		virtual Results CalculateResults() const override {Results ret; return ret;};//DUMMY
		virtual void GenerateSchedule() override;

		bool IsThirdPlaceMatch() const { return m_Finals.IsThirdPlaceMatch(); }
		bool IsFifthPlaceMatch() const { return m_Finals.IsFifthPlaceMatch(); }

		void IsThirdPlaceMatch(bool Enable) { m_Finals.IsThirdPlaceMatch(Enable); GenerateSchedule(); }
		void IsFifthPlaceMatch(bool Enable) { m_Finals.IsFifthPlaceMatch(Enable); GenerateSchedule(); }

		//Serialization
		virtual const std::string ToHTML() const override;

		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;


	private:
		int m_PoolCount = 2;

		std::vector<RoundRobin*> m_Pools;
		SingleElimination m_Finals;
	};
}