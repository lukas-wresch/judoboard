#include "app.h"
#include "database.h"
#include "weightclass.h"
#include "remote_mat.h"
#include "tournament.h"
#include "../ZED/include/log.h"
#include "../ZED/include/csv.h"
#include "../ZED/include/http_client.h"



using namespace Judoboard;



ZED::CSV Application::Ajax_GetMats() const
{
	ZED::CSV ret;

	if (GetTournament())
	{//Show all mats that are available/used and an additional one
		auto max = std::max(GetHighestMatID(), GetTournament()->GetHighestMatIDUsed()) + 1;
		ret << max;

		for (uint32_t id = 1; id <= max; id++)
		{
			auto mat = FindMat(id);

			if (!mat)
			{
				std::string mat_name = Localizer::Translate("Mat") + " " + std::to_string(id);
				ret << id << IMat::Type::Unknown << false << mat_name << false;
			}
			else
				ret << mat->GetMatID() << mat->GetType() << mat->IsOpen() << mat->GetName() << mat->GetIpponDisplayStyle();
		}
	}

	ret.AddNewline();
	return ret;
}



Error Application::Ajax_OpenMat(const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	if (id <= 0)
		return Error::Type::InvalidID;

	/*if (id == 0)//Create virtual mat
	{
		id = 1;
		while (FindMat(id))//Find first empty id
			id++;

		VirtualMat* new_mat = new VirtualMat(id);
		new_mat->Open();

		SetMats().emplace_back(new_mat);
		return Error();//OK
	}

	else
	{*/
	for (auto mat : SetMats())
	{
		if (mat && mat->GetMatID() == id)
		{
			mat->Open();
			return Error::Type::NoError;//OK
		}
	}
	//}

	return Error::Type::OperationFailed;
}



Error Application::Ajax_CloseMat(const HttpServer::Request& Request)
{
	int id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	if (id <= 0)
		return Error::Type::InvalidID;

	LockTillScopeEnd();
	if (CloseMat(id))
		return Error::Type::NoError;//OK

	return Error::Type::OperationFailed;
}



Error Application::Ajax_UpdateMat(const HttpServer::Request& Request)
{
	int id     = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Query, "id"));
	int new_id = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body,  "id"));
	auto name  = HttpServer::DecodeURLEncoded(Request.m_Body, "name");
	int ipponStyle = ZED::Core::ToInt(HttpServer::DecodeURLEncoded(Request.m_Body, "ipponStyle"));

	if (id <= 0 || new_id <= 0)
		return Error::Type::InvalidID;
	if (ipponStyle <= -1)
		return Error::Type::InvalidID;

	if (id != new_id)//Check if new_id is an unused id
	{
		for (const auto mat : GetMats())
		{
			if (mat && mat->GetMatID() == new_id)
				return Error::Type::InvalidID;
		}
	}

	LockTillScopeEnd();

	for (auto mat : SetMats())
	{
		if (mat && mat->GetMatID() == id)
		{
			if (id != new_id)
				mat->SetMatID(new_id);

			mat->SetName(name);
			mat->SetIpponDisplayStyle((Mat::IpponDisplayStyle)ipponStyle);

			return Error();//OK
		}
	}

	return Error::Type::MatNotFound;
}



ZED::CSV Application::Ajax_ListClubs()
{
	ZED::CSV ret;
	for (auto club : m_Database.GetAllClubs())
	{
		if (club)
			ret << club->GetID() << club->GetName();
	}
	ret.AddNewline();
	return ret;
}



ZED::CSV Application::Ajax_Uptime()
{
	ZED::CSV ret;

	ret << (Timer::GetTimestamp() - m_StartupTimestamp);

	ret.AddNewline();
	return ret;
}