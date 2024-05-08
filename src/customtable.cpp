#include <algorithm>
#include <random>
#include "customtable.h"
#include "tournament.h"
#include "localizer.h"
#include "match.h"
#include "timer.h"



using namespace Judoboard;



CustomTable::CustomTable(const ITournament* Tournament) : MatchTable(nullptr, Tournament)
{
	SetName(Localizer::Translate("Custom"));
}



std::string CustomTable::GetHTMLForm()
{
	std::string ret = R"()";

	return ret;
}



void CustomTable::operator >> (YAML::Emitter& Yaml) const
{
	if (!IsSubMatchTable())
		Yaml << YAML::BeginMap;

	MatchTable::operator >>(Yaml);

	if (!IsSubMatchTable())
		Yaml << YAML::EndMap;
}



const std::string CustomTable::ToHTML() const
{
	std::string ret;

	ret += "<a href=\"#matchtable_add.html?id=" + (std::string)GetUUID() + "\">" + GetName() + "</a><br/>";

	ret += Localizer::Translate("Mat") + " " + std::to_string(GetMatID()) + " / " + GetRuleSet().GetName() + "<br/>";

	ret += R"(<table width="50%" border="1" rules="all"><tr><th style="text-align: center;">)" + Localizer::Translate("No.");

	ret += "<th style=\"text-align: center; width: 4.0cm;\">" + Localizer::Translate("White")  + "</th>";
	ret += "<th style=\"text-align: center; width: 4.0cm;\">" + Localizer::Translate("Blue")   + "</th>";
	ret += "<th style=\"text-align: center; width: 2.0cm;\">" + Localizer::Translate("Result") + "</th>";
	ret += "</tr>";

	int no = 1;
	for (auto match : GetSchedule())
	{
		ret += "<tr>";
		ret += "<td style=\"text-align: center;\">" + std::to_string(no++) + "</td>";
		ret += "<td>" + match->GetFighter(Fighter::White)->GetName(NameStyle::GivenName) + "</td>";
		ret += "<td>" + match->GetFighter(Fighter::Blue )->GetName(NameStyle::GivenName) + "</td>";

		
		if (match->IsRunning())
			ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + (std::string)match->GetUUID() + "\">In Progress</a></td>";
		else if (!match->HasConcluded())
			ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + (std::string)match->GetUUID() + "\">- - -</a></td>";
		else
		{
			const auto& result = match->GetResult();
			ret += "<td style=\"text-align: center;\"><a href=\"#edit_match.html?id=" + (std::string)match->GetUUID() + "\">" + std::to_string((int)result.m_Score) + " (" + Timer::TimestampToString(result.m_Time) + ")</a></td>";
		}

		ret += "</tr>";
	}



	auto results = CalculateResults();

	ret += "</table><br/><br/><table border=\"1\" rules=\"all\">";
	ret += "<tr><th style=\"width: 0.5cm; text-align: center;\">#</th><th style=\"width: 5.0cm;\">" + Localizer::Translate("Name")
		+ "</th><th style=\"width: 1.0cm;\">" + Localizer::Translate("Wins") + "</th><th style=\"width: 1.0cm;\">"
		+ Localizer::Translate("Score") + "</th><th style=\"width: 1.3cm;\">" + Localizer::Translate("Time") + "</th></tr>";

	for (size_t i = 0; i < results.GetSize(); i++)
	{
		const auto& score = results[i];

		ret += "<tr><td style=\"text-align: center;\">" + std::to_string(i+1) + "</td>";
		ret += "<td>" + score.Judoka->GetName(NameStyle::GivenName) + "</td>";

		ret += "<td>" + std::to_string(score.Wins)  + "</td>";
		ret += "<td>" + std::to_string(score.Score) + "</td>";
		ret += "<td>" + Timer::TimestampToString(score.Time);

		if (score.NotSortable && score.Time > 0)
			ret += " Not sortable!";//TODO make proper message

		ret += "</td>";

		ret += "</tr>";
	}

	ret += "</table>";

	return ret;
}



void CustomTable::ToString(YAML::Emitter& Yaml) const
{
}