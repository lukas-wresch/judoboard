#pragma once
#include "matchtable.h"
#include "md5.h"
#include "filter.h"



namespace Judoboard
{
	class RoundRobin : public MatchTable
	{
		friend class Tournament;

	public:
		RoundRobin(std::shared_ptr<IFilter> Filter, const ITournament* Tournament = nullptr);
		RoundRobin(Weight MinWeight, Weight MaxWeight, Gender Gender = Gender::Unknown, const ITournament* Tournament = nullptr);
		RoundRobin(const YAML::Node& Yaml, const ITournament* Tournament, const MatchTable* Parent = nullptr);
		RoundRobin(const MD5::Weightclass& Weightclass_, const ITournament* Tournament = nullptr);

		void operator =(const RoundRobin& rhs) = delete;

		static std::string GetHTMLForm();

		virtual Type GetType() const override { return Type::RoundRobin; }

		virtual size_t GetMaxStartPositions() const override { return GetParticipants().size(); }

		virtual MatchTable::Results CalculateResults() const override;
		virtual size_t ResultsCount() const override { return GetParticipants().size(); }
		virtual void GenerateSchedule() override;

		//Serialization
		virtual void operator >> (YAML::Emitter& Yaml) const override;
		virtual const std::string ToHTML() const override;

	private:
	};
}