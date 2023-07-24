#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "matchlog.h"



using namespace Judoboard;



void MatchLog::operator << (const YAML::Node& Yaml)
{
	if (!Yaml.IsSequence())
		return;

	std::lock_guard<std::mutex> lock(m_Mutex);

	m_Events.clear();

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

	std::lock_guard<std::mutex> lock(m_Mutex);

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