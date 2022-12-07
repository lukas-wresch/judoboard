#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "filter.h"
#include "tournament.h"



using namespace Judoboard;



IFilter::IFilter(const YAML::Node& Yaml, const ITournament* Tournament)
	: IFilter(Tournament)
{
	if (Yaml["age_group"] && Tournament)
		SetAgeGroup(Tournament->FindAgeGroup(Yaml["age_group"].as<std::string>()));

	if (Yaml["participants"] && Yaml["participants"].IsMap() && Tournament)
	{
		for (const auto& node : Yaml["participants"])
			m_Participants.insert({ node.first.as<int>(), Tournament->FindParticipant(node.second.as<std::string>()) });
	}
}



void IFilter::operator >> (YAML::Emitter& Yaml) const
{
	if (GetAgeGroup())
		Yaml << YAML::Key << "age_group" << YAML::Value << (std::string)GetAgeGroup()->GetUUID();

	Yaml << YAML::Key << "participants";

	Yaml << YAML::BeginMap;
	for (const auto [start_pos, judoka] : GetParticipants())
	{
		if (judoka.GetJudoka())//TODO handle other cases
			Yaml << YAML::Key << start_pos << YAML::Value << (std::string)judoka.GetJudoka()->GetUUID();
	}
	Yaml << YAML::EndMap;
}



void IFilter::ToString(YAML::Emitter& Yaml) const
{
	Yaml << YAML::Key << "participants";

	Yaml << YAML::BeginMap;
	for (const auto [start_pos, judoka] : GetParticipants())
	{
		if (judoka.GetJudoka())//TODO handle other cases
			Yaml << YAML::Key << start_pos << YAML::Value << (std::string)judoka.GetJudoka()->GetUUID();
	}
	Yaml << YAML::EndMap;
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

	SortParticipantsByStartPosition();
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
	auto old_max = GetMaxStartPositions();

	auto pos = GetStartPosition(Participant);
	if (pos != SIZE_MAX)
	{
		m_Participants.erase(pos);

		auto new_max = GetMaxStartPositions(); 

		if (new_max < old_max)
		{
			for (size_t i = new_max; i < old_max; ++i)
			{
				auto judoka = GetJudokaByStartPosition(i);
				if (judoka)
					FindFreeStartPos(*judoka);
			}
		}

		return true;
	}

	return false;
}



size_t IFilter::GetStartPosition(const DependentJudoka& Judoka) const
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



void IFilter::SetStartPosition(const Judoka* Judoka, size_t NewStartPosition)
{
	if (!Judoka)
		return;

	auto my_old_pos = GetStartPosition(Judoka);
	m_Participants.erase(my_old_pos);

	if (IsStartPositionTaken(NewStartPosition))
	{
		auto judoka_on_slot = GetJudokaByStartPosition(NewStartPosition);

		assert(judoka_on_slot);

		auto copy = *judoka_on_slot;

		m_Participants.erase(my_old_pos);
		m_Participants.erase(NewStartPosition);
		if (judoka_on_slot)
			m_Participants.insert({ my_old_pos, copy });
		m_Participants.insert({ NewStartPosition, Judoka });
	}

	else
	{
		m_Participants.erase(GetStartPosition(Judoka));
		m_Participants.insert({ NewStartPosition, Judoka });
	}

	SortParticipantsByStartPosition();
}