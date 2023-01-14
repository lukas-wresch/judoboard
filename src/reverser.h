#pragma once
#include "filter.h"



namespace Judoboard
{
	class Reverser : public IFilter
	{
	public:
		Reverser(const IFilter& pSource);
		Reverser(const YAML::Node& Yaml, const ITournament* Tournament);

		virtual Type GetType() const override { return Type::Reverser; }

		virtual std::string GetHTMLForm() const;

		virtual bool IsElgiable(const Judoka& Fighter) const override;

		virtual std::unordered_map<size_t, const DependentJudoka> GetParticipants() const override;

		virtual const DependentJudoka GetJudokaByStartPosition(size_t StartPosition) const override;

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;

	private:
		const IFilter& m_pSource;
	};
}