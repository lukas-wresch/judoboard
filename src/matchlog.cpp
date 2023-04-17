#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include <sstream>
#include "matchlog.h"



using namespace Judoboard;



const std::string MatchLog::ToString() const
{
	std::stringstream ret;

	bool comma_needed = false;
	for (auto& event : m_Events)
	{
		if (comma_needed)
			ret << ",";
		ret << event.m_Timestamp << "," << std::to_string((int)event.m_Group) << "," << std::to_string((int)event.m_Event);
		comma_needed = true;
	}

	return ret.str();
}



void MatchLog::operator << (const YAML::Node& Yaml)
{
	if (!Yaml.IsSequence())
		return;

	for (const auto& node : Yaml)
	{
		if (!node.IsMap())
			continue;

		if (node["group"] && node["event"] && node["timestamp"])
		{
			if ((EventGroup)node["group"].as<int>() == EventGroup::Neutral)
				m_Events.emplace_back(Event((NeutralEvent)node["event"].as<int>(),
											node["timestamp"].as<int>()));
			else
				m_Events.emplace_back(Event((Fighter)node["group"].as<int>(),
											(BiasedEvent)node["event"].as<int>(),
											node["timestamp"].as<int>()));
		}
	}
}



void MatchLog::operator >> (YAML::Emitter& Yaml) const
{
	Yaml << YAML::BeginSeq;

	for (auto& event : m_Events)
	{
		Yaml << YAML::BeginMap;
		Yaml << YAML::Key << "group"     << YAML::Value << (int)event.m_Group;
		Yaml << YAML::Key << "event"     << YAML::Value << (int)event.m_Event;
		Yaml << YAML::Key << "timestamp" << YAML::Value << event.m_Timestamp;
		Yaml << YAML::EndMap;
	}

	Yaml << YAML::EndSeq;
}



void MatchLog::SwapEvents()
{
	for (auto& event : m_Events)
		event.m_Group = !event.m_Group;
}