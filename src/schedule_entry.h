#pragma once
#define YAML_CPP_STATIC_DEFINE
#include "yaml-cpp/yaml.h"
#include "../ZED/include/csv.h"
#include "id.h"



namespace Judoboard
{
	class Match;
	class ITournament;
	class Tournament;



	



	class Schedulable : public ID
	{
		friend class Tournament;

	public:
		/*Schedulable(const ITournament* Tournament) : m_Tournament(Tournament) {}
		Schedulable(const YAML::Node& Yaml, const ITournament* Tournament) : m_Tournament(Tournament)
		{
			if (!Yaml.IsMap())
				return;

			if (Yaml["uuid"])
				SetUUID(Yaml["uuid"].as<std::string>());
			if (Yaml["schedule_index"])
				m_ScheduleIndex = Yaml["schedule_index"].as<int>();
			if (Yaml["mat_id"])
				m_MatID = Yaml["mat_id"].as<int>();
			if (Yaml["color"])
				m_Color = Yaml["color"].as<int>();
		}

		

		

	protected:
		


		void operator >> (YAML::Emitter& Yaml) const {
			Yaml << YAML::Key << "uuid" << YAML::Value << (std::string)GetUUID();
			Yaml << YAML::Key << "schedule_index" << YAML::Value << m_ScheduleIndex;
			Yaml << YAML::Key << "mat_id" << YAML::Value << m_MatID;
			Yaml << YAML::Key << "color"  << YAML::Value << (int)m_Color;
		}*/

	private:
		/*void SetScheduleIndex(int32_t ScheduleIndex) { m_ScheduleIndex = ScheduleIndex; }
		void SetTournament(const ITournament* Tournament) { m_Tournament = Tournament; }

		const ITournament* m_Tournament = nullptr;

		int32_t m_ScheduleIndex = -1;//Index when this entry should be in the schedule
		uint32_t m_MatID = 0;
		Color m_Color;*/
	};
}