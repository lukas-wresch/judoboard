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
		MatchParcel(const MatchTable& Table, size_t Count);
		/*MatchParcel(const MatchParcel& Previous, size_t Count)
			: m_Table(Previous), m_StartIndex(Previous.Start), m_Count(Count)
		{}*/
		MatchParcel(MatchParcel& rhs)
			: m_Table(rhs.m_Table) {
			m_Matches = rhs.m_Matches;
		}
		MatchParcel(const MatchParcel& rhs)
			: m_Table(rhs.m_Table) {
			m_Matches = rhs.m_Matches;
		}
		MatchParcel(MatchParcel&& rhs) noexcept
			: m_Table(rhs.m_Table) {
			m_Matches = std::move(rhs.m_Matches);
		}

		void operator += (MatchParcel& rhs);

		void operator << (size_t Count);

		MatchParcel Split();

		size_t GetSize() const { return m_Count; }

	private:
		//MatchParcel() = default;

		const MatchTable& m_Table;
		std::vector<UUID> m_Matches;

		size_t m_StartIndex = 0;
		size_t m_Count = 0;
	};



	struct Delivery
	{
		Delivery(const MatchTable& Table)
			: m_Table(Table) {}

		Delivery& Append(MatchParcel& Parcel) {
			m_Parcels.emplace_back(Parcel);
			return *this;
		}

		Delivery& Append(size_t Count) {
			m_Parcels.emplace_back(m_Table, Count);
			return *this;
		}

		bool Append(Delivery& rhs) {//Add a single parcel from rhs
			if (rhs.m_Parcels.empty())
				return false;
			Append(rhs.m_Parcels.front());
			rhs.m_Parcels.erase(rhs.m_Parcels.begin());
			return true;
		}

		void AppendAll(Delivery& rhs) {
			for (auto parcel : rhs.m_Parcels)
				m_Parcels.emplace_back(parcel);
		}

		Delivery& operator << (MatchParcel& Parcel) {
			return Append(Parcel);
		}
		Delivery& operator << (size_t Count) {
			return Append(Count);
		}
		bool operator << (Delivery& rhs) {
			return Append(rhs);
		}

		Delivery& AddRemainder();

		size_t GetSize() const {
			size_t count = 0;
			for (auto& parcel : m_Parcels)
				count += parcel.GetSize();
			return count;
		}
		bool IsEmpty() const {
			return m_Parcels.empty();
		}

	private:
		std::vector<MatchParcel> m_Parcels;

		const MatchTable& m_Table;
		//size_t m_CurrentIndex = 0;
	};



	struct Scheduler
	{
		void Start(std::vector<const MatchTable*> Plan);

		std::vector<Match*> m_Schedule;
	};
}