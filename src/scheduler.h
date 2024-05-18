#pragma once
#include "judoboard.h"
#include "matchtable.h"



namespace Judoboard
{
	class MatchTable;
	class Match;


	struct MatchParcel
	{
	public:
		MatchParcel(const MatchTable& Table, size_t Start, size_t Count)
			: m_Table(Table), m_StartIndex(Start), m_Count(Count)
		{}
		/*MatchParcel(const MatchParcel& Previous, size_t Count)
			: m_Table(Previous), m_StartIndex(Previous.Start), m_Count(Count)
		{}*/
		MatchParcel(MatchParcel& rhs) = delete;
		MatchParcel(const MatchParcel& rhs) = delete;
		MatchParcel(MatchParcel&& rhs) noexcept
			: m_Table(rhs.m_Table), m_StartIndex(rhs.m_StartIndex), m_Count(rhs.m_Count)
		{
			rhs.m_Count = 0;
		}

		void operator += (MatchParcel& rhs);

		MatchParcel Split();

	private:
		//MatchParcel() = default;

		const MatchTable& m_Table;

		size_t m_StartIndex = 0;
		size_t m_Count = 0;
	};



	struct Delivery
	{
		Delivery(const MatchTable& Table)
			: m_Table(Table) {}

		Delivery& Append(size_t Count) {
			m_Parcels.emplace_back(m_Table, m_CurrentIndex, Count);
			m_CurrentIndex += Count;
			return *this;
		}

		Delivery operator << (size_t Count) {
			return Append(Count);
		}

		Delivery AddRemainder();

		size_t GetSize() const { return m_CurrentIndex; }

		std::vector<MatchParcel> m_Parcels;

		const MatchTable& m_Table;
		size_t m_CurrentIndex = 0;
	};



	struct Scheduler
	{
		void Start(std::vector<const MatchTable*> Plan);

		std::vector<Match*> m_Schedule;
	};
}