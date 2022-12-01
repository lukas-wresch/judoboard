#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "filter.h"



using namespace Judoboard;



bool IFilter::AddParticipant(const Judoka* NewParticipant, bool Force)
{
	if (!NewParticipant)
		return false;

	if (!Force)//Don't check if we are adding we are forcing the judoka in this match table
		if (!IsElgiable(*NewParticipant))//Is the judoka allowed in this match table?
			return false;

	//Is the judoka already a participant?
	for (auto& [pos, judoka] : GetParticipants())
		if (judoka && *judoka == *NewParticipant)
			return false;

	m_Participants.emplace_back(NewParticipant);
	return true;
}



bool IFilter::RemoveParticipant(const Judoka* Participant)
{
	if (!Participant)
		return false;

	for (auto it = GetParticipants().begin(); it != GetParticipants().end(); ++it)
	{
		if (*it && *(*it) == *Participant)
		{
			m_Participants.erase(it);
			return true;
		}
	}

	return false;
}