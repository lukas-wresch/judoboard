#include <fstream>
#include <random>
#include <ZED/include/log.h>
#include <ZED/include/sha512.h>
#include "standing_data.h"



using namespace Judoboard;



void StandingData::Reset()
{
	for (auto [id, judoka] : m_Judokas)
		delete judoka;
	m_Judokas.clear();

	for (auto* rule : m_RuleSets)
		delete rule;
	m_RuleSets.clear();
}



void StandingData::operator << (ZED::CSV& Stream)
{
	uint32_t judokaCount = 0;
	Stream >> judokaCount;

	for (uint32_t i = 0; i < judokaCount; i++)
	{
		Judoka* newJudoka = new Judoka(Stream);
		m_Judokas.insert({ newJudoka->GetID(), newJudoka });
	}

	uint32_t ruleSetsCount = 0;
	Stream >> ruleSetsCount;

	for (uint32_t i = 0; i < ruleSetsCount; i++)
	{
		auto new_rule_set = new RuleSet(Stream);
		m_RuleSets.emplace_back(new_rule_set);
	}
}



void StandingData::operator >> (ZED::CSV& Stream) const
{
	Stream << m_Judokas.size();

	for (auto [id, judoka] : m_Judokas)
	{
		if (judoka)
			*judoka >> Stream;
	}

	Stream << m_RuleSets.size();

	for (auto rule : m_RuleSets)
		if (rule)
			*rule >> Stream;
}



bool StandingData::AddJudoka(Judoka* NewJudoka)
{
	if (!NewJudoka)
		return false;

	m_Judokas.insert({ NewJudoka->GetID(), NewJudoka });
	return true;
}



bool StandingData::DeleteJudoka(uint32_t ID)
{
	for (auto it = m_Judokas.begin(); it != m_Judokas.end(); ++it)
	{
		if (it->first == ID)
		{
			m_Judokas.erase(it);
			return true;
		}
	}

	return false;
}



const std::string StandingData::JudokaToJSON() const
{
	std::string ret = "[";

	for (auto [id, judoka] : m_Judokas)
	{
		if (ret.length() > 1)
			ret += ",";
		ret += "{\"label\":\"" + judoka->GetName() + " (" + std::to_string(judoka->GetWeight()) +" kg)\", \"value\":\"" + std::to_string(id) + "\"}";
	}

	ret += "]";
	return ret;
}



bool StandingData::AddClub(Club* NewClub)
{
	if (!NewClub)
		return false;

	m_Clubs.emplace_back(NewClub);
	return true;
}



Club* StandingData::FindClub(uint32_t ID)
{
	for (auto club : m_Clubs)
		if (club && club->GetID() == ID)
			return club;

	return nullptr;
}



const Club* StandingData::FindClub(uint32_t ID) const
{
	for (auto club : m_Clubs)
		if (club && club->GetID() == ID)
			return club;

	return nullptr;
}



Club* StandingData::FindClub(const UUID& UUID)
{
	for (auto club : m_Clubs)
		if (club && club->GetUUID() == UUID)
			return club;

	return nullptr;
}



const Club* StandingData::FindClub(const UUID& UUID) const
{
	for (auto club : m_Clubs)
		if (club && club->GetUUID() == UUID)
			return club;

	return nullptr;
}



RuleSet* StandingData::FindRuleSet(const std::string& RuleSetName)
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetName() == RuleSetName)
			return rule;

	return nullptr;
}



const RuleSet* StandingData::FindRuleSet(const std::string& RuleSetName) const
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetName() == RuleSetName)
			return rule;

	return nullptr;
}



RuleSet* StandingData::FindRuleSet(const UUID& UUID)
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetUUID() == UUID)
			return rule;

	return nullptr;
}



const RuleSet* StandingData::FindRuleSet(const UUID& UUID) const
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetUUID() == UUID)
			return rule;

	return nullptr;
}



RuleSet* StandingData::FindRuleSet(uint32_t ID)
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetID() == ID)
			return rule;

	return nullptr;
}



const RuleSet* StandingData::FindRuleSet(uint32_t ID) const
{
	for (auto rule : m_RuleSets)
		if (rule && rule->GetID() == ID)
			return rule;

	return nullptr;
}



bool StandingData::AddRuleSet(RuleSet* NewRuleSet)
{
	if (!NewRuleSet || FindRuleSet(NewRuleSet->GetName()))
		return false;

	m_RuleSets.emplace_back(NewRuleSet);
	return true;
}



Judoka* StandingData::FindJudoka(uint32_t ID)
{
	auto item = m_Judokas.find(ID);
	if (item == m_Judokas.end())
		return nullptr;

	return item->second;
}



const Judoka* StandingData::FindJudoka(uint32_t ID) const
{
	auto item = m_Judokas.find(ID);
	if (item == m_Judokas.end())
		return nullptr;

	return item->second;
}



Judoka* StandingData::FindJudoka(const UUID& UUID)
{
	for (auto [id, judoka] : m_Judokas)
	{
		if (judoka && judoka->GetUUID() == UUID)
			return judoka;
	}

	return nullptr;
}



const Judoka* StandingData::FindJudoka(const UUID& UUID) const
{
	for (auto [id, judoka] : m_Judokas)
	{
		if (judoka && judoka->GetUUID() == UUID)
			return judoka;
	}

	return nullptr;
}