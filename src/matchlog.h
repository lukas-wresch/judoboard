#pragma once
#include <vector>
#include "../ZED/include/csv.h"



namespace Judoboard
{
	enum class Fighter
	{
		White, Blue
	};


	enum class Winner
	{
		White, Blue, Draw
	};



	inline Fighter operator! (Fighter f)
	{
		if (f == Fighter::White)
			return Fighter::Blue;
		return Fighter::White;
	}


	inline void operator++ (Fighter& f)
	{
		f = (Fighter)(((int)f) + 1);
	}



	inline void operator++ (Fighter& f, int)
	{
		f = (Fighter)(((int)f) + 1);
	}


	inline bool operator== (Winner w, Fighter f)
	{
		return (int)w == (int)f;
	}


	inline bool operator== (Fighter f, Winner w)
	{
		return (int)w == (int)f;
	}


	inline const std::string Fighter2String(Fighter f)
	{
		if (f == Fighter::White)
			return "white";
		return "blue";
	}

	inline Winner Fighter2Winner(Fighter f)
	{
		switch (f)
		{
		case Fighter::White:
			return Winner::White;
		case Fighter::Blue:
			return Winner::Blue;
		}
		return Winner::White;
	}



	class MatchLog
	{
		friend class Tournament;

	public:
		enum class EventGroup
		{
			White, Blue, Neutral
		};

		enum class NeutralEvent
		{
			Hajime, Mate,
			Tokeda,
			Sonomama, Yoshi,
			OsaekomiSwitched,//Switch osaekomi time from one fighter to the other
			StartMatch, EndMatch,
			EnableGoldenScore, DisableGoldenScore,
			EnableDraw, DisableDraw
		};

		enum class BiasedEvent
		{
			AddIppon, AddWazaari, AddYuko, AddKoka,
			RemoveIppon, RemoveWazaari, RemoveYuko, RemoveKoka,
			WazariAwaseteIppon,
			Osaekomi,
			AddShido, AddHansokuMake_Indirect, AddHansokuMake_Direct,
			RemoveShido, RemoveHansokuMake,
			AddMedicalExamination, RemoveMedicalExamination,
			Hantei,
			AddDisqualification,
			AddNotDisqualification,
			RemoveDisqualification,
			AddGachi,
			RemoveGachi,
		};


		void AddEvent(NeutralEvent NewEvent, uint32_t Timestamp) { m_Events.emplace_back(Event(NewEvent, Timestamp)); }
		void AddEvent(Fighter Whom, BiasedEvent NewEvent, uint32_t Timestamp) { m_Events.emplace_back(Event(Whom, NewEvent, Timestamp)); }

		auto& GetEvents() const { return m_Events; }
		size_t GetNumEvent() const { return m_Events.size(); }
		//const Event* GetEvent(uint32_t Index) const { if (Index < m_Events.size()) return &m_Events[Index]; return nullptr; }

		const std::string ToString() const;

		void operator << (ZED::CSV& Stream);
		void operator >> (ZED::CSV& Stream) const;


	private:
		struct Event
		{
			Event()
			{
				m_Event = NeutralEvent::Hajime;
			}

			Event(NeutralEvent Event, uint32_t Timestamp)
			{
				m_Group = EventGroup::Neutral;
				m_Event = Event;
				m_Timestamp = Timestamp;
			}

			Event(Fighter Whom, BiasedEvent Event, uint32_t Timestamp)
			{
				if (Whom == Fighter::White)
					m_Group = EventGroup::White;
				else
					m_Group = EventGroup::Blue;
				m_BiasedEvent = Event;
				m_Timestamp = Timestamp;
			}

			EventGroup m_Group = EventGroup::Neutral;
			uint32_t m_Timestamp = 0;

			union
			{
				NeutralEvent m_Event;
				BiasedEvent  m_BiasedEvent;
			};
		};


		auto& SetEvents() { return m_Events; }


		std::vector<Event> m_Events;
	};



	inline bool operator == (Fighter f, MatchLog::EventGroup g)
	{
		return (int)f == (int)g;
	}
	inline bool operator == (MatchLog::EventGroup g, Fighter f)
	{
		return (int)f == (int)g;
	}
}