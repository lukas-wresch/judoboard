#pragma once
#include "filter.h"



namespace Judoboard
{
	class MatchTable;


	class TakeTopRanks : public IFilter
	{
	public:
		TakeTopRanks(const MatchTable& pSource, size_t NumRanks);
		TakeTopRanks(const YAML::Node& Yaml, const ITournament* Tournament);

		virtual Type GetType() const override { return Type::TakeTopRanks; }

		virtual std::string GetHTMLForm() const;

		virtual bool IsElgiable(const Judoka& Fighter) const override;

		virtual std::unordered_map<size_t, const DependentJudoka> GetParticipants() const override;

		virtual const DependentJudoka GetJudokaByStartPosition(size_t StartPosition) const override;

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual void ToString(YAML::Emitter& Yaml) const override;

	private:
		const MatchTable& m_pSource;
		size_t m_NumRanks;//Take this number of judoka from the top
	};
}