#include <random>
#include "../ZED/include/log.h"
#include "error.h"
#include "localizer.h"



using namespace Judoboard;



Error::operator std::string () const
{
	switch (m_Type)
	{
		case Type::NoError:
			return "ok";

		case Type::NotLoggedIn:
			return Localizer::Translate("Not logged in");
		case Type::NotEnoughPermissions:
			return Localizer::Translate("Not enough permissions");
		case Type::InvalidID:
			return Localizer::Translate("Invalid id");
		case Type::MatNotFound:
			return Localizer::Translate("Could not find mat");

		case Type::ApplicationShuttingDown:
			return Localizer::Translate("Application is shutting down");

		case Type::InternalError:
			return Localizer::Translate("Internal error");

		case Type::ItemNotFound:
			return Localizer::Translate("Not found");

		case Type::TournamentNotOpen:
			return Localizer::Translate("No tournament is open");

		case Type::OperationFailed:
			return Localizer::Translate("Operation failed");
		case Type::InvalidFormat:
			return Localizer::Translate("Invalid format");
		case Type::InvalidInput:
			return Localizer::Translate("Invalid input");
	}

	return Localizer::Translate("Unknown error");
}