#pragma once
#include "filter.h"



namespace Judoboard
{
	class AgeGroup;


	class Fuser : public IFilter
	{
	public:
		Fuser(const ITournament* Tournament) : IFilter(Tournament) {}
		Fuser(const IFilter& pSource1, const IFilter& pSource2);
		Fuser(const YAML::Node& Yaml, const ITournament* Tournament);

		virtual Type GetType() const override { return Type::Fuser; }

		virtual std::string GetHTMLForm() const;

		virtual bool IsElgiable(const Judoka& Fighter) const override;

		virtual std::unordered_map<size_t, const DependentJudoka> GetParticipants() const override;

		virtual const DependentJudoka GetJudokaByStartPosition(size_t StartPosition) const override;

		void AddSource(const IFilter& Source) {
			m_pSources.emplace_back(&Source);
		}

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	private:
		std::vector<const IFilter*> m_pSources;
	};
}