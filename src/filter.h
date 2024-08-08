#pragma once
#include "judoboard.h"
#include "judoka.h"



namespace Judoboard
{
	class AgeGroup;
	class Tournament;


	class IFilter : public ID
	{
		friend class MatchTable;

	public:
		enum class Type
		{
			Weightclass, Fixed, Splitter, TakeTopRanks, Fuser, Mixer, Reverser,
			Standard,//Deprecated, use Fixed
			LosersOf
		};

		void LoadYaml(const YAML::Node& Yaml);

		virtual Type GetType() const = 0;

		virtual std::string GetDescription() const { return ""; }

		virtual bool IsElgiable(const Judoka& Fighter) const { return false; };

		virtual bool AddParticipant(const Judoka* NewParticipant, bool Force = false);
		virtual bool RemoveParticipant(const Judoka* Participant);
		bool RemoveParticipant(const DependentJudoka Participant);

		//const MatchTable*  GetParent() const { return m_Parent; }
		const ITournament* GetTournament() const;

		//Age groups
		std::shared_ptr<const AgeGroup> GetAgeGroup() const { return m_pAgeGroup;}
		void SetAgeGroup(std::shared_ptr<const AgeGroup> NewAgeGroup) { m_pAgeGroup = NewAgeGroup; }

		//Participants + Start Positions
		virtual std::unordered_map<size_t, const DependentJudoka> GetParticipants() const {
			m_Mutex.lock();
			auto ret = m_Participants;
			m_Mutex.unlock();
			return ret;
		}

		virtual size_t GetStartPosition(const Judoka* Judoka) const;
		virtual void   SetStartPosition(const DependentJudoka Judoka, size_t NewStartPosition);
		virtual void   SetStartPosition(const Judoka* Judoka, size_t NewStartPosition);

		virtual const DependentJudoka GetJudokaByStartPosition(size_t StartPosition) const {
			m_Mutex.lock();
			auto result = m_Participants.find(StartPosition);
			if (result == m_Participants.end())
			{
				m_Mutex.unlock();
				return nullptr;
			}
			m_Mutex.unlock();
			return result->second;
		}

		virtual bool IsStartPositionTaken(size_t StartPosition) const {
			m_Mutex.lock();
			auto ret = m_Participants.find(StartPosition) != m_Participants.end();
			m_Mutex.unlock();
			return ret;
		}

		virtual void FindFreeStartPos(const DependentJudoka NewParticipant);

		virtual void operator >> (YAML::Emitter& Yaml) const;
		virtual void ToString(YAML::Emitter& Yaml) const;

		void Dump() const;


	protected:
		IFilter() = default;
		//IFilter(const MatchTable* Parent) : m_Parent(Parent) {}

		size_t GetStartPosition(const DependentJudoka Judoka) const;

		void SetParticipants(std::unordered_map<size_t, const DependentJudoka>&& NewParticipants) const {
			m_Mutex.lock();
			m_Participants = std::move(NewParticipants);
			m_Mutex.unlock();
		}

		void RemoveAllParticipants() {
			m_Mutex.lock();
			m_Participants.clear();
			m_Mutex.unlock();
		}

	protected:
		mutable std::recursive_mutex m_Mutex;

	private:
		//void SetParent(const MatchTable* Parent) { m_Parent = Parent; }


		mutable std::unordered_map<size_t, const DependentJudoka> m_Participants;//List of all participants that are in the match table

		std::shared_ptr<const AgeGroup> m_pAgeGroup = nullptr;//Age group for the matches (if available)

		//const MatchTable* m_Parent = nullptr;
	};
}