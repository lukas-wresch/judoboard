#pragma once
#include "judoboard.h"
#include "judoka.h"



namespace Judoboard
{
	class AgeGroup;
	class Tournament;


	class IFilter : public ID
	{
	public:
		enum class Type
		{
			Weightclass, Custom, Splitter, Fuser
		};

		IFilter(const YAML::Node& Yaml, const ITournament* Tournament);

		virtual Type GetType() const = 0;

		virtual std::string GetDescription() const { return ""; }

		virtual std::string GetHTMLForm() const = 0;

		virtual bool IsElgiable(const Judoka& Fighter) const = 0;

		virtual bool AddParticipant(const Judoka* NewParticipant, bool Force = false);
		virtual bool RemoveParticipant(const Judoka* Participant);

		const ITournament* GetTournament() const { return m_Tournament; }

		//Age groups
		const AgeGroup* GetAgeGroup() const { return m_pAgeGroup;}
		void SetAgeGroup(const AgeGroup* NewAgeGroup) { m_pAgeGroup = NewAgeGroup; }

		//Participants + Start Positions
		virtual std::unordered_map<size_t, const DependentJudoka> GetParticipants() const { return m_Participants; }

		virtual size_t GetStartPosition(const Judoka* Judoka) const;
		virtual void   SetStartPosition(const Judoka* Judoka, size_t NewStartPosition);

		virtual const DependentJudoka GetJudokaByStartPosition(size_t StartPosition) const {
			auto result = m_Participants.find(StartPosition);
			if (result == m_Participants.end())
				return nullptr;
			return result->second;
		}

		virtual bool IsStartPositionTaken(size_t StartPosition) const {
			return m_Participants.find(StartPosition) != m_Participants.end();
		}

		virtual void FindFreeStartPos(const DependentJudoka NewParticipant);

		virtual void operator >> (YAML::Emitter& Yaml) const;
		virtual void ToString(YAML::Emitter& Yaml) const;


	protected:
		IFilter(const ITournament* Tournament) : m_Tournament(Tournament) {}

		size_t GetStartPosition(const DependentJudoka Judoka) const;
		bool RemoveParticipant(const DependentJudoka Participant);

		void SortParticipantsByStartPosition() {
			//std::sort(m_Participants.begin(), m_Participants.end(), [this](const size_t a, const size_t b) {
				//return a < b;
			//});
		}


	private:
		std::unordered_map<size_t, const DependentJudoka> m_Participants;//List of all participants that are in the match table

		const AgeGroup* m_pAgeGroup = nullptr;//Age group for the matches (if available)

		const ITournament* m_Tournament = nullptr;
	};
}