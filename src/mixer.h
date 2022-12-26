#pragma once
#include "filter.h"



namespace Judoboard
{
	class Mixer : public IFilter
	{
	public:
		Mixer(const ITournament* Tournament) : IFilter(Tournament) {}
		Mixer(const IFilter& pSource1, const IFilter& pSource2);
		Mixer(const YAML::Node& Yaml, const ITournament* Tournament);

		virtual Type GetType() const override { return Type::Mixer; }

		virtual std::string GetHTMLForm() const;

		virtual bool IsElgiable(const Judoka& Fighter) const override;

		virtual std::unordered_map<size_t, const DependentJudoka> GetParticipants() const override;

		virtual const DependentJudoka GetJudokaByStartPosition(size_t StartPosition) const override;

		void AddSource(const IFilter& Source) {
			m_pSources.emplace_back(&Source);
		}

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;
		//virtual void ToString(YAML::Emitter& Yaml) const override;

	private:
		std::vector<const IFilter*> m_pSources;
	};
}