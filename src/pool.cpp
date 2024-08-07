#include <algorithm>
#include <random>
#include "pool.h"
#include "round_robin.h"
#include "weightclass.h"
#include "splitter.h"
#include "take_top_ranks.h"
#include "reverser.h"
#include "fuser.h"
#include "mixer.h"
#include "fixed.h"
#include "rule_set.h"
#include "localizer.h"
#include "match.h"



using namespace Judoboard;



Pool::Pool(std::shared_ptr<IFilter> Filter, const ITournament* Tournament)
	: MatchTable(Filter, Tournament)
{
	m_Finals = std::make_shared<SingleElimination>(nullptr, Tournament);
	m_Finals->SetParent(this);
	//GenerateSchedule();
}



Pool::Pool(Weight MinWeight, Weight MaxWeight, Gender Gender, const ITournament* Tournament)
	: Pool(std::make_shared<Weightclass>(MinWeight, MaxWeight, Gender, this), Tournament)
{
}



void Pool::LoadYaml(const YAML::Node& Yaml)
{
	MatchTable::LoadYaml(Yaml);

	m_Finals = std::make_shared<SingleElimination>(nullptr, m_Tournament, m_Parent);

	if (Yaml["pool_count"])
		m_PoolCount = Yaml["pool_count"].as<uint32_t>();
	if (Yaml["take_top"])
		m_TakeTop   = Yaml["take_top"].as<uint32_t>();

	if (Yaml["pools"] && Yaml["pools"].IsSequence())
	{
		for (const auto& node : Yaml["pools"])
		{
			auto new_pool = std::make_shared<RoundRobin>(nullptr, m_Tournament);
			new_pool->LoadYaml(node);
			m_Pools.push_back(new_pool);
		}
	}

	if (Yaml["finals"] && Yaml["finals"].IsMap())
	{
		m_Finals = std::make_shared<SingleElimination>(nullptr, m_Tournament, this);
		m_Finals->LoadYaml(Yaml["finals"]);
	}

	CopyMatchesFromSubtables();
}



void Pool::operator >> (YAML::Emitter& Yaml) const
{
	if (!IsSubMatchTable())
		Yaml << YAML::BeginMap;

	auto schedule_copy = std::move(SetSchedule());

	MatchTable::operator >>(Yaml);

	Yaml << YAML::Key << "pool_count" << YAML::Value << m_PoolCount;
	Yaml << YAML::Key << "take_top"   << YAML::Value << m_TakeTop;

	//Serialize pools
	Yaml << YAML::Key << "pools" << YAML::Value;
	Yaml << YAML::BeginSeq;
	for (auto pool : m_Pools)
	{
		Yaml << YAML::BeginMap;
		*pool >> Yaml;
		Yaml << YAML::EndMap;
	}
	Yaml << YAML::EndSeq;

	Yaml << YAML::Key << "finals" << YAML::Value;
	Yaml << YAML::BeginMap;
	*m_Finals >> Yaml;
	Yaml << YAML::EndMap;

	SetSchedule(std::move(schedule_copy));

	if (!IsSubMatchTable())
		Yaml << YAML::EndMap;
}



void Pool::ToString(YAML::Emitter& Yaml) const
{
	MatchTable::ToString(Yaml);

	Yaml << YAML::Key << "third_place" << YAML::Value << IsThirdPlaceMatch();
	Yaml << YAML::Key << "fifth_place" << YAML::Value << IsFifthPlaceMatch();
}



std::string Pool::GetHTMLForm()
{
	std::string ret = R"(
<div>
  <label style="width:150px;float:left;margin-top:5px;" id="label_pool">#Pools</label>
  <select style="margin-bottom:20px;" id="pool_count">
    <option selected value="0" id="auto">Auto</option>
    <option value="2">2</option>
	<option value="4">4</option>
	<option value="8">8</option>
  </select>
</div>
)";

	ret += SingleElimination::GetHTMLForm();

	return ret;
}



size_t Pool::GetMaxStartPositions() const
{
	if (!GetFilter())
		return 0;

	const size_t pool_count = CalculatePoolCount();

	return ( (size_t)std::floor(GetFilter()->GetParticipants().size() / pool_count) ) * pool_count;
}



std::shared_ptr<Match> Pool::FindMatch(const UUID& UUID) const
{
	for (auto pool : m_Pools)
	{
		auto ret = pool->FindMatch(UUID);
		if (ret)
			return ret;
	}

	return m_Finals->FindMatch(UUID);
}



std::shared_ptr<const MatchTable> Pool::FindMatchTable(const UUID& UUID) const
{
	for (auto pool : m_Pools)
		if (*pool == UUID)
			return pool;

	if (*m_Finals == UUID)
		return m_Finals;

	return nullptr;
}



bool Pool::DeleteMatch(const UUID& UUID)
{
	bool success = MatchTable::DeleteMatch(UUID);

	for (auto pool : m_Pools)
		success |= pool->DeleteMatch(UUID);

	success |= m_Finals->DeleteMatch(UUID);
	return success;
}



void Pool::GenerateSchedule()
{
	if (!IsAutoGenerateSchedule())
		return;
	if (!IsSubMatchTable() && GetStatus() != Status::Scheduled)
		return;

	SetSchedule().clear();

	m_RecommendedNumMatches_Before_Break = 4;//TODO

	auto old_pools = std::move(m_Pools);
	assert(m_Pools.empty());

	const auto pool_count = CalculatePoolCount();
	m_Pools.resize(pool_count);

	const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	//Distribute participants to pools
	for (int i = 0; i < pool_count; ++i)
	{
		if (GetFilter())
			m_Pools[i] = std::make_shared<RoundRobin>(std::make_shared<Splitter>(GetFilter(), pool_count, i));
		else
			m_Pools[i] = std::make_shared<RoundRobin>(nullptr);

		std::string name = Localizer::Translate("Pool") + " ";
		name.append(&letters[i % 26], 1);
		m_Pools[i]->SetName(name);
		m_Pools[i]->SetParent(this);

		//Convert from old pools
		if (i < old_pools.size())
		{
			m_Pools[i]->SetName(old_pools[i]->GetName());
			m_Pools[i]->SetColor(old_pools[i]->GetColor());
			m_Pools[i]->SetMatID(old_pools[i]->GetMatID());
			m_Pools[i]->IsBestOfThree(old_pools[i]->IsBestOfThree());
		}
	}

	//Create filter(s) for final round
	std::shared_ptr<IFilter> final_input;

	if (pool_count == 2)
	{
		auto topA = std::make_shared<TakeTopRanks>(m_Pools[0], m_TakeTop);
		auto topB = std::make_shared<TakeTopRanks>(m_Pools[1], m_TakeTop);

		auto mixer = std::make_shared<Mixer>();
		mixer->AddSource(topA);
		mixer->AddSource(topB);

		final_input = std::make_shared<Fixed>(mixer);

		if (m_TakeTop == 3)
		{
			auto temp = final_input->GetJudokaByStartPosition(4);
			if (temp)
				final_input->SetStartPosition(temp, 7);
			temp = final_input->GetJudokaByStartPosition(5);
			if (temp)
				final_input->SetStartPosition(temp, 6);
		}

		else
		{
			//Swap the two incorrect positions manually
			auto temp = final_input->GetJudokaByStartPosition(2);
			if (temp)
				final_input->SetStartPosition(temp, 3);
		}

#ifdef _DEBUG
		final_input->Dump();
#endif
	}

	else if (pool_count == 4)
	{
		auto mixer = std::make_shared<Mixer>();

		auto topA = std::make_shared<TakeTopRanks>(m_Pools[0], m_TakeTop);
		auto topB = std::make_shared<TakeTopRanks>(m_Pools[1], m_TakeTop);
		auto topC = std::make_shared<TakeTopRanks>(m_Pools[2], m_TakeTop);
		auto topD = std::make_shared<TakeTopRanks>(m_Pools[3], m_TakeTop);

		mixer->AddSource(topA);
		mixer->AddSource(topB);
		mixer->AddSource(topC);
		mixer->AddSource(topD);

		final_input = std::make_shared<Fixed>(mixer);

		auto temp = final_input->GetJudokaByStartPosition(4);
		if (temp)
			final_input->SetStartPosition(temp, 5);
		temp = final_input->GetJudokaByStartPosition(6);
		if (temp)
			final_input->SetStartPosition(temp, 7);

		//final_input->Dump();
	}

	else
	{
		auto mixer = std::make_shared<Mixer>();

		for (int i = 0; i < pool_count; ++i)
		{
			auto take_top_placed = std::make_shared<TakeTopRanks>(m_Pools[i], m_TakeTop);
			mixer->AddSource(take_top_placed);
		}

		final_input = std::make_shared<Fixed>(mixer);
	}


	assert(final_input);
	bool third_place = m_Finals->IsThirdPlaceMatch();
	bool fifth_place = m_Finals->IsFifthPlaceMatch();
	auto color  = m_Finals->GetColor();
	auto name   = m_Finals->GetName();
	auto mat_id = m_Finals->GetMatID();
	auto bo3    = m_Finals->IsBestOfThree();

	m_Finals = std::make_shared<SingleElimination>(final_input);
	m_Finals->SetName(Localizer::Translate("Finals"));
	m_Finals->SetParent(this);
	m_Finals->IsThirdPlaceMatch(third_place);
	m_Finals->IsFifthPlaceMatch(fifth_place);
	m_Finals->SetColor(color);
	if (!name.empty())
		m_Finals->SetName(name);
	m_Finals->SetMatID(mat_id);
	m_Finals->IsBestOfThree(bo3);
	m_Finals->GenerateSchedule();

	//Four or less get forwarded, need to a fifth place match manually
	if (pool_count * m_TakeTop <= 4 && IsFifthPlaceMatch())
	{
		assert(pool_count == 2);//TODO
		assert(m_TakeTop == 2);//TODO

		auto fifth_place_match = std::make_shared<Match>(DependentJudoka(DependencyType::TakeRank3, m_Pools[0]),
														 DependentJudoka(DependencyType::TakeRank3, m_Pools[1]), GetTournament());
		fifth_place_match->SetTag(Match::Tag::Fifth() && Match::Tag::Finals());

		m_Finals->AddMatch(fifth_place_match);

		//TODO do the more general case
	}


	CopyMatchesFromSubtables();
}



void Pool::CopyMatchesFromSubtables()
{
	SetSchedule().clear();

	//Add matches from pools
	size_t index = 0;
	bool added;
	do
	{
		added = false;
		for (int pool = 0; pool < m_Pools.size(); ++pool)
		{
			auto schedule = GetPool(pool)->GetSchedule();

			if (index < schedule.size())
			{
				AddMatch(schedule[index]);
				added = true;
			}
		}

		++index;
	} while (added);//Still matches to add?


	//Add matches for single elimination phase
	auto final_schedule = m_Finals->GetSchedule();
	for (auto match : final_schedule)
		AddMatch(match);
}



const std::string Pool::ToHTML() const
{
	std::string ret = GetHTMLTop();

	for (auto pool : m_Pools)
		ret += pool->ToHTML() + "<br/><br/>";

	ret += m_Finals->ToHTML();

	if (!IsSubMatchTable())
		ret += ResultsToHTML() + "</div>";

	return ret;
}