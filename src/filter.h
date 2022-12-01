#pragma once
#include "judoboard.h"
#include "judoka.h"



namespace Judoboard
{
	class IFilter : public ID
	{
	public:
		enum class Type
		{
			Weightclass, Custom
		};

		IFilter(const YAML::Node& Yaml, const ITournament* Tournament);

		virtual Type GetType() const = 0;

		virtual std::string GetDescription() const { return ""; }

		virtual std::string GetHTMLForm() const = 0;

		virtual bool IsElgiable(const Judoka& Fighter) const = 0;

		virtual bool AddParticipant(const Judoka* NewParticipant, bool Force = false);
		virtual bool RemoveParticipant(const Judoka* Participant);

		virtual void operator >> (YAML::Emitter& Yaml) const = 0;
		virtual void ToString(YAML::Emitter& Yaml) const = 0;

		virtual size_t GetStartingPosition(const Judoka* Judoka) const;
		virtual void   SetStartingPosition(const Judoka* Judoka, size_t NewStartingPosition);

		//Age groups
		const AgeGroup* GetAgeGroup() const { return m_pAgeGroup;}
		void SetAgeGroup(const AgeGroup* NewAgeGroup) { m_pAgeGroup = NewAgeGroup; }

		auto& GetParticipants() const { return m_Participants; }


	protected:
		IFilter(const ITournament* Tournament) : m_Tournament(Tournament) {}

		const DependentJudoka& GetJudokaByStartingPosition(size_t StartPosition) const {
			auto result = m_Participants.find(StartPosition);
			if (result == m_Participants.end())
				return nullptr;
			return result->second;
		}

		bool IsStartPositionTaken(size_t StartPosition) const {
			return m_Participants.find(StartPosition) != m_Participants.end();
		}

		void SortParticipantsByStartingPosition() {
			//std::sort(m_Participants.begin(), m_Participants.end(), [this](const size_t a, const size_t b) {
				//return a < b;
			//});
		}

		const ITournament* GetTournament() const { return m_Tournament; }


	private:
		std::unordered_map<size_t, const DependentJudoka> m_Participants;//List of all participants that are in the match table

		const AgeGroup* m_pAgeGroup = nullptr;//Age group for the matches (if available)

		const ITournament* m_Tournament = nullptr;
	};
}