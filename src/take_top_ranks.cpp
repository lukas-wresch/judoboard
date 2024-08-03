#include "take_top_ranks.h"
#include "matchtable.h"



using namespace Judoboard;



TakeTopRanks::TakeTopRanks(std::shared_ptr<const MatchTable> pSource, size_t NumRanks, const MatchTable* Parent) :
	m_pSource(pSource), IFilter(Parent)
{
	m_NumRanks = NumRanks;

	assert(NumRanks > 0);
	assert(NumRanks <= 10);//Not supported
}



/*TakeTopRanks::TakeTopRanks(const YAML::Node& Yaml, const MatchTable* Parent) :
	m_pSource(Tournament.FindFilter(Yaml["source"])), IFilter(Yaml, Parent)
{
	if (Yaml["num_ranks"])
		m_NumRanks = Yaml["num_ranks"].as<size_t>();
}*/



void TakeTopRanks::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginMap;

	IFilter::operator>>(Yaml);

	Yaml << YAML::Key << "source" << YAML::Value << (std::string)m_pSource->GetUUID();

	Yaml << YAML::EndMap;
}



void TakeTopRanks::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();
	Yaml << YAML::Key << "source" << YAML::Value << (std::string)m_pSource->GetUUID();
}



std::string TakeTopRanks::GetHTMLForm() const
{
	std::string ret;

	return ret;
}



bool TakeTopRanks::IsElgiable(const Judoka& Fighter) const
{
	return m_pSource->IsElgiable(Fighter);
}



std::unordered_map<size_t, const DependentJudoka> TakeTopRanks::GetParticipants() const
{
	std::unordered_map<size_t, const DependentJudoka> ret;

	if (!m_pSource->HasConcluded())
	{
		for (int i = 0; i < m_NumRanks; ++i)
			ret.insert({ i, DependentJudoka( (DependencyType)((int)DependencyType::TakeRank1 + i), m_pSource ) });
	}

	else
	{
		auto results = m_pSource->CalculateResults();
		for (int i = 0; i < m_NumRanks; ++i)
		{
			if (i < results.GetSize())
				ret.insert({ i, results[i].Judoka });
			else
				ret.insert({ i, DependentJudoka( (DependencyType)((int)DependencyType::TakeRank1 + i), m_pSource ) });
		}
	}

	SetParticipants(std::move(ret));//Save to cache

	return IFilter::GetParticipants();//Return the cache
}



const DependentJudoka TakeTopRanks::GetJudokaByStartPosition(size_t StartPosition) const
{
	GetParticipants();//Re-calculates the participants
	return IFilter::GetJudokaByStartPosition(StartPosition);
}