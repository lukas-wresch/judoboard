#pragma once
#include "filter.h"
#include "matchtable.h"



namespace Judoboard
{
	class LosersOf : public IFilter
	{
	public:
		LosersOf(std::shared_ptr<const MatchTable> Table);
		//LosersOf(const YAML::Node& Yaml, const MatchTable* Parent);

		virtual Type GetType() const override { return Type::LosersOf; }

		virtual std::string GetHTMLForm() const;

		virtual bool IsElgiable(const Judoka& Fighter) const override { return m_MatchTable->IsElgiable(Fighter); }

		virtual std::unordered_map<size_t, const DependentJudoka> GetParticipants() const override;

		virtual const DependentJudoka GetJudokaByStartPosition(size_t StartPosition) const override;

		bool IsRemoveLast() const { return m_RemoveLast; }
		void RemoveLast(bool Remove = true) { m_RemoveLast = Remove; }

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;
		//virtual void ToString(YAML::Emitter& Yaml) const override;

	private:
		void Recalculate() const;

		std::shared_ptr<const MatchTable> m_MatchTable;

		bool m_RemoveLast = false;
	};
}