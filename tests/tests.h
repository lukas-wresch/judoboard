#include "gtest/gtest.h"

#include <signal.h>


#include "../src/account.h"
#include "../src/app.h"
#include "../src/database.h"
#include "../src/imat.h"
#include "../src/club.h"
#include "../src/judoka.h"
#include "../src/mat.h"
#include "../src/match.h"
#include "../src/matchlog.h"
//#include "../src/pause.h"
#include "../src/remote_mat.h"
#include "../src/rule_set.h"
#include "../src/schedule_entry.h"
#include "../src/timer.h"
#include "../src/tournament.h"
//#include "../src/virtual_mat.h"
#include "../src/weightclass.h"
#include "../src/weightclass_generator.h"
#include "../src/single_elimination.h"
#include "../src/window.h"
#include "../src/dm4.h"
#include "../src/md5.h"
#include "../src/dmf.h"
#include "../ZED/include/http_client.h"
#include "../ZED/include/sha256.h"
#include "../ZED/include/sha512.h"
#include "../ZED/include/log.h"
#include "../ZED/include/file.h"


using namespace Judoboard;



inline void my_abort_function(int signo)
{
	ASSERT_TRUE(false);
}



inline void initialize() noexcept
{
#ifdef _WIN32
	_set_error_mode(_OUT_TO_STDERR);
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
	signal(SIGABRT, my_abort_function);
	signal(SIGILL,  my_abort_function);
	signal(SIGINT,  my_abort_function);
	signal(SIGSEGV, my_abort_function);
#endif

	Judoboard::ID::Reset();
	Application::NoWindow = true;
}



inline bool m_RandSeeded = false;


inline std::string GetRandomName()
{
	if (!m_RandSeeded)
	{
		srand(ZED::Core::CurrentTimestamp());
		m_RandSeeded = true;
	}

	const char alphanum[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::string name;
	for (int i = 0; i < 5; i++)
		name += alphanum[rand() % (sizeof(alphanum) - 1)];

	return name + std::to_string(rand()%1000);
}



inline std::string GetFakeFirstname()
{
	if (!m_RandSeeded)
	{
		srand(ZED::Core::CurrentTimestamp());
		m_RandSeeded = true;
	}

	const std::string firstname[] =
	{ "Ben", "Stephanie", "Phillipp", "Tim", "Lukas", "Julia", "Marco", "Peter", "Martin", "Jana", "Detlef", "Andreas", "Uta", "Dominik", "Petra", "Mathias", "Sophie", "Kerstin", "Lena", "Jennifer", "Kathrin", "Stephan", "Eric" };

	std::string name;
	name += firstname[rand() % (sizeof(firstname)/sizeof(std::string) - 1)] + std::to_string(rand() % 10) + " ";
	//name += lastname [rand() % (sizeof(lastname)/sizeof(std::string)  - 1)];

	return name;
}



inline std::string GetFakeLastname()
{
	if (!m_RandSeeded)
	{
		srand(ZED::Core::CurrentTimestamp());
		m_RandSeeded = true;
	}

	const std::string lastname[] =
	{ "Ehrlichmann", "Dresdner", "Biermann", "Fisher", "Vogler", "Pfaff", "Eberhart", "Frankfurter", "Konig", "Pabst", "Ziegler", "Hartmann", "Pabst", "Kortig", "Schweitzer", "Luft", "Wexler", "Kaufmann", "Fruehauf", "Bieber", "Schumacher", "Muencher", "Schmidt", "Meier" };

	std::string name;
	//name += firstname[rand() % (sizeof(firstname) / sizeof(std::string) - 1)] + std::to_string(rand() % 10) + " ";
	name += lastname[rand() % (sizeof(lastname) / sizeof(std::string) - 1)];

	return name;
}



inline Judoka Test_CreateRandomJudoka(const StandingData* db)
{
	const std::string firstname_male[] =
	{ "Ben", "Friedrich", "Phillipp", "Tim", "Lukas", "Marco", "Peter", "Martin", "Detlef", "Andreas", "Dominik", "Mathias", "Stephan", u8"Sören", "Eric", "Finn", "Felix", "Julian", "Maximilian", "Jannik"};
	const std::string firstname_female[] =
	{ "Emma", "Stephanie", "Julia", "Jana", "Uta", "Petra", "Sophie", "Kerstin", "Lena", "Jennifer", "Kathrin", "Katherina", "Anna", "Carla", "Paulina", "Clara", "Hanna" };
	const std::string lastname[] =
	{ "Ehrlichmann", "Dresdner", "Biermann", "Fisher", "Vogler", "Pfaff", "Eberhart", "Frankfurter", u8"König", "Pabst", "Ziegler", "Hartmann", "Pabst", "Kortig", "Schweitzer", "Luft", "Wexler", "Kaufmann", u8"Frühauf", "Bieber", "Schumacher", u8"Müncher", "Schmidt", "Meier", "Fischer", "Weber", "Meyer", "Wagner", "Becker", "Schulz", "Hoffmann" };

	Judoboard::Judoka ret("", "");

	if (rand() & 1)
	{
		auto fname = firstname_male[rand() % (sizeof(firstname_male) / sizeof(firstname_male[0]) - 1)];
		auto lname = lastname[rand() % (sizeof(lastname) / sizeof(std::string) - 1)];
		ret = Judoboard::Judoka(fname, lname, 25 + rand() % 60, Judoboard::Gender::Male);
	}
	else
	{
		auto fname = firstname_female[rand() % (sizeof(firstname_female) / sizeof(firstname_female[0]) - 1)];
		auto lname = lastname[rand() % (sizeof(lastname) / sizeof(std::string) - 1)];
		ret = Judoboard::Judoka(fname, lname, 25 + rand() % 60, Judoboard::Gender::Female);
	}

	ret.SetBirthyear(1990 + rand()%25);

	if (db && db->GetAllClubs().size() >= 1)
	{
		int club_index = rand() % db->GetAllClubs().size();
		auto club = db->GetAllClubs()[club_index];
		ret.SetClub(club);
	}

	return ret;
}