#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "filter.h"
#include "tournament.h"



using namespace Judoboard;



IFilter::IFilter(const YAML::Node& Yaml, const ITournament* Tournament)
	: IFilter(Tournament)
{
	if (Yaml["starting_positions"] && Yaml["starting_positions"].IsMap() && Tournament)
	{
		for (const auto& node : Yaml["starting_positions"])
			m_Participants.insert({ node.first.as<int>(), Tournament->FindParticipant(node.second.as<std::string>()) });
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

	for (size_t startPos = 0; true; startPos++)
	{
		if (!IsStartPositionTaken(startPos))
		{
			m_Participants.insert({ startPos, NewParticipant });
			break;
		}
	}

	SortParticipantsByStartingPosition();
	return true;
}



bool IFilter::RemoveParticipant(const Judoka* Participant)
{
	if (!Participant)
		return false;

	auto pos = GetStartingPosition(Participant);
	if (pos != SIZE_MAX)
	{
		m_Participants.erase(pos);
		return true;
	}

	return false;
}



size_t IFilter::GetStartingPosition(const Judoka* Judoka) const
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



void IFilter::SetStartingPosition(const Judoka* Judoka, size_t NewStartingPosition)
{
	if (!Judoka)
		return;

	auto my_old_pos = GetStartingPosition(Judoka);
	m_Participants.erase(my_old_pos);

	if (IsStartPositionTaken(NewStartingPosition))
	{
		auto judoka_on_slot = GetJudokaByStartingPosition(NewStartingPosition);

		m_Participants.erase(my_old_pos);
		m_Participants.erase(NewStartingPosition);
		m_Participants.insert({ my_old_pos, judoka_on_slot });
		m_Participants.insert({ NewStartingPosition, Judoka });
	}

	else
	{
		m_Participants.erase(GetStartingPosition(Judoka));
		m_Participants.insert({ NewStartingPosition, Judoka });
	}

	SortParticipantsByStartingPosition();
}