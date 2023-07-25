#pragma once
#include <vector>
#include <mutex>


namespace YAML
{
	class Emitter;
	class Node;
}



namespace Judoboard
{
	enum class Fighter
	{
		White = 0, Blue
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
		friend class Match;

	public:
		enum class EventGroup
		{
			White, Blue, Neutral = 2
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
			AddNoDisqualification,
			RemoveDisqualification,
			RemoveNoDisqualification,
			AddGachi,
			RemoveGachi,
			HanteiRevoked
		};


		MatchLog() = default;
		MatchLog(const MatchLog& Copy) {
			std::lock_guard<std::mutex> lock(Copy.m_Mutex);
			m_Events = Copy.m_Events;
		}
		MatchLog(const YAML::Node& Yaml) {
			*this << Yaml;
		}

		void AddEvent(NeutralEvent NewEvent, uint32_t Timestamp) {
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Events.emplace_back(Event(NewEvent, Timestamp));
		}
		void AddEvent(Fighter Whom, BiasedEvent NewEvent, uint32_t Timestamp) {
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Events.emplace_back(Event(Whom, NewEvent, Timestamp));
		}

		auto& GetEvents() const
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			return m_Events;
		}
		size_t GetNumEvent() const {
			std::lock_guard<std::mutex> lock(m_Mutex);
			return m_Events.size();
		}
		
		void SwapEvents();

		void operator << (const YAML::Node& Yaml);
		void operator >> (YAML::Emitter& Yaml) const;

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
		mutable std::mutex m_Mutex;
	};



	inline bool operator == (Fighter f, MatchLog::EventGroup g)
	{
		return (int)f == (int)g;
	}
	inline bool operator == (MatchLog::EventGroup g, Fighter f)
	{
		return (int)f == (int)g;
	}

	inline MatchLog::EventGroup operator ! (MatchLog::EventGroup g)
	{
		if (g == MatchLog::EventGroup::White)
			return MatchLog::EventGroup::Blue;
		else if (g == MatchLog::EventGroup::Blue)
			return MatchLog::EventGroup::White;
		return MatchLog::EventGroup::Neutral;
	}
}