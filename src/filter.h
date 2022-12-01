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

		virtual std::string GetHTMLForm() const = 0;

		virtual bool IsElgiable(const Judoka& Fighter) const = 0;

		virtual bool AddParticipant(const Judoka* NewParticipant, bool Force = false);
		virtual bool RemoveParticipant(const Judoka* Participant);

		virtual DependentJudoka GetFromPosition(size_t Position) = 0;

		virtual void operator >> (YAML::Emitter& Yaml) const = 0;
		virtual void ToString(YAML::Emitter& Yaml) const = 0;

		virtual size_t GetStartingPosition(const Judoka* Judoka) const { return -1; }
		virtual void   SetStartingPosition(const Judoka* Judoka, size_t NewStartingPosition) {}

		//Age groups
		const AgeGroup* GetAgeGroup() const { return m_pAgeGroup;}
		void SetAgeGroup(const AgeGroup* NewAgeGroup) { m_pAgeGroup = NewAgeGroup; }

		auto& GetParticipants() const { return m_Participants; }

	protected:
		const DependentJudoka* GetParticipantByStartingPosition(size_t StartingPosition) const;

		void SortParticipantsByStartingPosition() {
			std::sort(m_Participants.begin(), m_Participants.end(), [this](const Judoka* a, const Judoka* b) {
				return GetStartingPosition(a) < GetStartingPosition(b);
			});
		}


	private:
		//std::vector<DependentJudoka*> m_Participants;//List of all participants that are in the match table

		std::unordered_map<size_t, const DependentJudoka*> m_Participants;//List of all participants that are in the match table

		const AgeGroup* m_pAgeGroup = nullptr;//Age group for the matches (if available)
	};
}