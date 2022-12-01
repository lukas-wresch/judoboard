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

		virtual Type GetType() const = 0;

		virtual bool IsElgiable(const Judoka& Fighter) const = 0;

		virtual DependentJudoka GetFromPosition(size_t Position) = 0;

		virtual void operator >> (YAML::Emitter& Yaml) const = 0;
		virtual void ToString(YAML::Emitter& Yaml) const = 0;

		virtual size_t GetStartingPosition(const Judoka* Judoka) const { return -1; }
		virtual void   SetStartingPosition(const Judoka* Judoka, size_t NewStartingPosition) {}

		//Age groups
		const AgeGroup* GetAgeGroup() const { return m_pAgeGroup;}
		void SetAgeGroup(const AgeGroup* NewAgeGroup) { m_pAgeGroup = NewAgeGroup; }

		const std::vector<Judoka*>& GetParticipants() const { return m_Participants; }

	protected:
		size_t GetIndexOfParticipant(const Judoka* Participant) const;

		Judoka* GetParticipant(size_t Index) { if (Index >= m_Participants.size()) return nullptr; return m_Participants[Index]; }
		const Judoka* GetParticipant(size_t Index) const { if (Index >= m_Participants.size()) return nullptr; return m_Participants[Index]; }

		void SortParticipantsByStartingPosition() {
			std::sort(m_Participants.begin(), m_Participants.end(), [this](const Judoka* a, const Judoka* b) {
				return GetStartingPosition(a) < GetStartingPosition(b);
			});
		}

	
		std::vector<Judoka*>& SetParticipants() { return m_Participants; }

	private:
		std::vector<Judoka*> m_Participants;//List of all participants that are in the match table

		const AgeGroup* m_pAgeGroup = nullptr;//Age group for the matches (if available)
	};
}