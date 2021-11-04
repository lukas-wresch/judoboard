#include "tests.h"



TEST(Account, NonceExpiration)
{
	srand(ZED::Core::CurrentTimestamp());
	int ip  = rand();
	int port = rand();

	Account::Nonce nonce(ip, port);

	EXPECT_TRUE(nonce.GetIP() == ip);

	EXPECT_FALSE(nonce.HasExpired());

	ZED::Core::Pause(60 * 1000);
	EXPECT_FALSE(nonce.HasExpired());

	ZED::Core::Pause(60 * 1000);
	EXPECT_FALSE(nonce.HasExpired());

	ZED::Core::Pause(60 * 1000);
	EXPECT_FALSE(nonce.HasExpired());

	ZED::Core::Pause(60 * 1000);
	EXPECT_FALSE(nonce.HasExpired());

	ZED::Core::Pause(61 * 1000);
	EXPECT_TRUE(nonce.HasExpired());
}


TEST(Account, Basics)
{
	Account acc("Username", "Password");

	EXPECT_EQ(acc.GetUsername(), std::string("Username"));
	EXPECT_TRUE(acc.GetAccessLevel() == Account::AccessLevel::User);

	Account acc2("Username2", "Password", Account::AccessLevel::Moderator);
	EXPECT_EQ(acc2.GetUsername(), std::string("Username2"));
	EXPECT_TRUE(acc2.GetAccessLevel() == Account::AccessLevel::Moderator);

	Account acc3("Username3", "Password", Account::AccessLevel::Admin);
	EXPECT_EQ(acc3.GetUsername(), std::string("Username3"));
	EXPECT_TRUE(acc3.GetAccessLevel() == Account::AccessLevel::Admin);
}


TEST(Account, Login)
{
	srand(ZED::Core::CurrentTimestamp());

	for (int i = 0; i < 1000 * 50; i++)
	{
		int ip = rand();
		int port = rand();
		std::string username = "Username" + std::to_string(rand());
		std::string password = "Password" + std::to_string(rand());

		Account acc(username, password);

		Account::Nonce nonce(ip, port);

		std::string response = ZED::SHA512(username + nonce.GetChallenge() + password);

		EXPECT_TRUE(acc.Verify(nonce, response));
	}
}



TEST(Account, LoginBruteforce)
{
	srand(ZED::Core::CurrentTimestamp());

	for (int i = 0; i < 1000 * 100; i++)
	{
		int ip = rand();
		int port = rand();
		std::string username = "Username" + std::to_string(rand());
		std::string password = "Password" + std::to_string(rand());

		Account acc(username, password);

		Account::Nonce nonce(ip, port);

		std::string response = ZED::SHA512( std::to_string(rand()) + std::to_string(rand()) );

		EXPECT_FALSE(acc.Verify(nonce, response));
	}
}