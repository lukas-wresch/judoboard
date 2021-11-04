#pragma once
#include "schedule_entry.h"



class Pause : public ScheduleEntry
{
	friend class Tournament;

public:
	Pause(uint32_t ID) : ScheduleEntry(ID) {}

	const std::string GetName() const override { return "Pause"; }

	bool IsElgiable(const Judoka& Fighter) const override { return false; }
	void GenerateSchedule() override {}

	//Serialization
	const std::string ToHTML() const override { return ""; }//DEBUG
	const std::string ToString() const override { return ""; }//DEBUG
	const std::string ToSaveFile() const override { return ""; }//DEBUG

private:
	Pause(uint32_t ID, std::stringstream Stream);
};