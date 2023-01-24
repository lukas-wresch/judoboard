#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../ZED/include/log.h"
#include "filter.h"
#include "tournament.h"



using namespace Judoboard;



IFilter::IFilter(const YAML::Node& Yaml, const MatchTable* Parent)
	: IFilter(Parent)
{
	if (Yaml["age_group"] && GetTournament())
		SetAgeGroup(GetTournament()->FindAgeGroup(Yaml["age_group"].as<std::string>()));

	if (Yaml["participants"] && Yaml["participants"].IsMap() && GetTournament())
	{
		for (const auto& node : Yaml["participants"])
		{
			auto& data = node.second;

			if (data.IsSequence())
			{
				if (data[0].as<int>() == 0)//Type 0
					m_Participants.insert({ node.first.as<int>(), GetTournament()->FindParticipant(data[1].as<std::string>()) });
				else if (data[0].as<int>() == 1)//Type 1
				{
					UUID uuid = data[2].as<std::string>();
					const MatchTable* match_table = nullptr;

					if (GetParent())
						match_table = GetParent()->FindMatchTable(uuid);
					else
						match_table = GetTournament()->FindMatchTable(uuid);

					assert(match_table);

					if (match_table)
						m_Participants.insert({ node.first.as<int>(), DependentJudoka((DependencyType)data[1].as<int>(), *match_table) });
				}
				else if (data[0].as<int>() == 2)//Type 2
				{
					auto match = GetTournament()->FindMatch(data[2].as<std::string>());
					assert(match);

					if (match)
						m_Participants.insert({ node.first.as<int>(), DependentJudoka((DependencyType)data[1].as<int>(), *match) });
				}
				else
					assert(false);
			}
		}
	}
}



void IFilter::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "type" << YAML::Value << (int)GetType();

	if (GetAgeGroup())
		Yaml << YAML::Key << "age_group" << YAML::Value << (std::string)GetAgeGroup()->GetUUID();

	Yaml << YAML::Key << "participants";

	Yaml << YAML::BeginMap;
	for (const auto [start_pos, judoka] : GetParticipants())
	{
		Yaml << YAML::Key << start_pos << YAML::Value;
		Yaml << YAML::BeginSeq;

		if (judoka.GetJudoka())
		{
			Yaml << 0;//Type 0
			Yaml << (std::string)judoka.GetJudoka()->GetUUID();
		}
		else if (judoka.GetDependentMatchTable())
		{
			Yaml << 1;//Type 1
			Yaml << (int)judoka.GetDependency();
			Yaml << (std::string)judoka.GetDependentMatchTable()->GetUUID();
		}
		else if (judoka.GetDependentMatch())
		{
			Yaml << 2;//Type 2
			Yaml << (int)judoka.GetDependency();
			Yaml << (std::string)judoka.GetDependentMatch()->GetUUID();
		}
		else
			assert(false);

		Yaml << YAML::EndSeq;
	}
	Yaml << YAML::EndMap;
}



void IFilter::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "filter_type" << YAML::Value << (int)GetType();

	/*Yaml << YAML::Key << "participants";

	Yaml << YAML::BeginMap;
	for (const auto [start_pos, judoka] : GetParticipants())
	{
		if (judoka.GetJudoka())//TODO handle other cases
			Yaml << YAML::Key << start_pos << YAML::Value << (std::string)judoka.GetJudoka()->GetUUID();
	}
	Yaml << YAML::EndMap;*/
}



void IFilter::FindFreeStartPos(const DependentJudoka NewParticipant)
{
	auto old_pos = GetStartPosition(NewParticipant);

	if (old_pos != SIZE_MAX)
		m_Participants.erase(old_pos);

	for (size_t startPos = 0; true; startPos++)
	{
		if (!IsStartPositionTaken(startPos))
		{
			m_Participants.insert({ startPos, NewParticipant });
			break;
		}
	}
}



bool IFilter::AddParticipant(const Judoka* NewParticipant, bool Force)
{
	if (!NewParticipant)
		return false;

	if (!Force)//Don't check if we are adding we are forcing the judoka in this match table
		if (!IsElgiable(*NewParticipant))//Is the judoka allowed in this match table?
			return false;

	//Is the judoka already a participant?
	for (const auto& [pos, judoka] : GetParticipants())
		if (judoka == NewParticipant)
			return false;

	FindFreeStartPos(NewParticipant);

	return true;
}



bool IFilter::RemoveParticipant(const Judoka* Participant)
{
	if (!Participant)
		return false;
	return RemoveParticipant(DependentJudoka(Participant));
}



bool IFilter::RemoveParticipant(const DependentJudoka Participant)
{
	auto pos = GetStartPosition(Participant);

	if (pos != SIZE_MAX)
	{
		m_Participants.erase(pos);
		return true;
	}

	return false;
}



const ITournament* IFilter::GetTournament() const
{
	if (!m_Parent)
		return nullptr;
	return m_Parent->GetTournament();
}



size_t IFilter::GetStartPosition(const DependentJudoka Judoka) const
{
	for (auto [pos, participant] : m_Participants)
	{
		if (participant == Judoka)
			return pos;
	}

	return SIZE_MAX;
}



size_t IFilter::GetStartPosition(const Judoka* Judoka) const
{
	if (!Judoka)
		return SIZE_MAX;

	for (auto [pos, participant] : m_Participants)
	{
		if (participant.GetJudoka() && *participant.GetJudoka() == *Judoka)
			return pos;
	}

	return SIZE_MAX;
}



void IFilter::SetStartPosition(const DependentJudoka Judoka, size_t NewStartPosition)
{
	auto my_old_pos = GetStartPosition(Judoka);
	m_Participants.erase(my_old_pos);

	if (IsStartPositionTaken(NewStartPosition))
	{
		auto judoka_on_slot = GetJudokaByStartPosition(NewStartPosition);

		m_Participants.erase(my_old_pos);
		m_Participants.erase(NewStartPosition);
		m_Participants.insert({ my_old_pos, judoka_on_slot });
		m_Participants.insert({ NewStartPosition, Judoka });
	}

	else
	{
		m_Participants.erase(GetStartPosition(Judoka));
		m_Participants.insert({ NewStartPosition, Judoka });
	}
}



void IFilter::SetStartPosition(const Judoka* Judoka, size_t NewStartPosition)
{
	if (!Judoka)
		return;

	SetStartPosition(DependentJudoka(Judoka), NewStartPosition);
}



void IFilter::Dump() const
{
	ZED::Log::Info("- - -");
	
	//for (auto [pos, participant] : m_Participants)
	for (size_t pos = 0; pos < m_Participants.size()*2; ++pos)
	{
		if (!IsStartPositionTaken(pos))
			continue;

		auto participant = GetJudokaByStartPosition(pos);

		if (participant.GetJudoka())
			ZED::Log::Info(std::to_string(pos) + " - " + participant.GetJudoka()->GetFirstname());
		else if (participant.GetDependency() == DependencyType::TakeRank1)
			ZED::Log::Info(std::to_string(pos) + " - Rank1 of " + participant.GetDependentMatchTable()->GetName());
		else if (participant.GetDependency() == DependencyType::TakeRank2)
			ZED::Log::Info(std::to_string(pos) + " - Rank2 of " + participant.GetDependentMatchTable()->GetName());
		else if (participant.GetDependency() == DependencyType::TakeRank3)
			ZED::Log::Info(std::to_string(pos) + " - Rank3 of " + participant.GetDependentMatchTable()->GetName());
		else if (participant.GetDependency() == DependencyType::TakeRank4)
			ZED::Log::Info(std::to_string(pos) + " - Rank4 of " + participant.GetDependentMatchTable()->GetName());
		else
			ZED::Log::Info(std::to_string(pos) + " - ???");
	}

	ZED::Log::Info("- - -");
}