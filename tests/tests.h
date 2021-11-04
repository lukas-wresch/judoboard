#include "gtest/gtest.h"

#include <signal.h>


//#define ZED_NO_LOG


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
#include "../src/virtual_mat.h"
#include "../src/weightclass.h"
#include "../src/window.h"

#include <ZED/include/http_client.h>
#include <ZED/include/sha512.h>
#include <ZED/include/log.h>


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
	//signal(SIGSEGV, my_abort_function);
	//signal(SIGINT,  my_abort_function);
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