#include "tests.h"



TEST(DMF, Read_first_age_group1_m)
{
	initialize();

#ifdef _WIN32
	DMF file("../test-data/firstage group1 (m).dmf");
#else
	DMF file("test-data/firstage group1 (m).dmf");
#endif

	ASSERT_TRUE(file);

	EXPECT_EQ(file.GetTournamentName(), "tournament name");
	EXPECT_EQ(file.GetTournamentDate(), "01.02.2022");
	EXPECT_EQ(file.GetTournamentPlace(), "tournament place");
	EXPECT_EQ(file.GetAgeGroup(), "age group1 (m)");
	EXPECT_EQ(file.GetGender(), Gender::Male);

	EXPECT_EQ(file.GetClub().Name,    "club name");
	EXPECT_EQ(file.GetClub().Kreis,   "club kreis");
	EXPECT_EQ(file.GetClub().Bezirk,  "club bezirk");
	EXPECT_EQ(file.GetClub().Country, "club land ");

	EXPECT_EQ(file.GetSenderName(),   "sender firstname sender lastname");
	EXPECT_EQ(file.GetSenderStreet(), "sender street");
	EXPECT_EQ(file.GetSenderPlace(),  "12345 sender place");
	EXPECT_EQ(file.GetSenderTel(),    "sender private");

	ASSERT_EQ(file.GetParticipants().size(), 1);
	EXPECT_EQ(file.GetParticipants()[0].Firstname, "firstname");
	EXPECT_EQ(file.GetParticipants()[0].Lastname,  "lastname");
	EXPECT_EQ(file.GetParticipants()[0].WeightInGrams, 50 * 1000);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 1990);
}



TEST(DMF, Read_second_age_group1_m)
{
	initialize();

#ifdef _WIN32
	DMF file("../test-data/secondage group1 (m).dmf");
#else
	DMF file("test-data/secondage group1 (m).dmf");
#endif

	ASSERT_TRUE(file);

	EXPECT_EQ(file.GetTournamentName(), "tournament name");
	EXPECT_EQ(file.GetTournamentDate(), "01.02.2022");
	EXPECT_EQ(file.GetTournamentPlace(), "tournament place");
	EXPECT_EQ(file.GetAgeGroup(), "age group1 (m)");
	EXPECT_EQ(file.GetGender(), Gender::Male);

	EXPECT_EQ(file.GetClub().Name,    "club name");
	EXPECT_EQ(file.GetClub().Kreis,   "club kreis");
	EXPECT_EQ(file.GetClub().Bezirk,  "club bezirk");
	EXPECT_EQ(file.GetClub().Country, "club land ");

	EXPECT_EQ(file.GetSenderName(),   "sender firstname sender lastname");
	EXPECT_EQ(file.GetSenderStreet(), "sender street");
	EXPECT_EQ(file.GetSenderPlace(),  "12345 sender place");
	EXPECT_EQ(file.GetSenderTel(),    "sender private");

	ASSERT_EQ(file.GetParticipants().size(), 2);
	EXPECT_EQ(file.GetParticipants()[0].Firstname, "lastname2");
	EXPECT_EQ(file.GetParticipants()[0].Lastname,  "firstname2");
	EXPECT_EQ(file.GetParticipants()[0].WeightInGrams, 60 * 1000);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 1995);

	EXPECT_EQ(file.GetParticipants()[1].Firstname, "firstname");
	EXPECT_EQ(file.GetParticipants()[1].Lastname,  "lastname");
	EXPECT_EQ(file.GetParticipants()[1].WeightInGrams, 50 * 1000);
	EXPECT_EQ(file.GetParticipants()[1].Birthyear, 1990);
}



TEST(DMF, Read_third_age_group_w)
{
	initialize();

#ifdef _WIN32
	DMF file("../test-data/third_name of age group (w).dmf");
#else
	DMF file("test-data/third_name of age group (w).dmf");
#endif

	ASSERT_TRUE(file);

	EXPECT_EQ(file.GetTournamentName(), "tourney");
	EXPECT_EQ(file.GetTournamentDate(), "01.05.1990");
	EXPECT_EQ(file.GetTournamentPlace(), "place of tourney");
	EXPECT_EQ(file.GetAgeGroup(), "name of age group (w)");
	EXPECT_EQ(file.GetGender(), Gender::Female);

	EXPECT_EQ(file.GetClub().Name,    "club name");
	EXPECT_EQ(file.GetClub().Kreis,   "club kreis");
	EXPECT_EQ(file.GetClub().Bezirk,  "club bezirk");
	EXPECT_EQ(file.GetClub().Country, "club land ");

	EXPECT_EQ(file.GetSenderName(),   "sender firstname sender lastname");
	EXPECT_EQ(file.GetSenderStreet(), "sender street");
	EXPECT_EQ(file.GetSenderPlace(),  "12345 sender place");
	EXPECT_EQ(file.GetSenderTel(),    "sender private");

	ASSERT_EQ(file.GetParticipants().size(), 3);
	EXPECT_EQ(file.GetParticipants()[0].Firstname, "first1");
	EXPECT_EQ(file.GetParticipants()[0].Lastname,  "name1");
	EXPECT_EQ(file.GetParticipants()[0].WeightInGrams, 40 * 1000);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 1990);

	EXPECT_EQ(file.GetParticipants()[1].Firstname, "first2");
	EXPECT_EQ(file.GetParticipants()[1].Lastname,  "name2");
	EXPECT_EQ(file.GetParticipants()[1].WeightInGrams, 50 * 1000);
	EXPECT_EQ(file.GetParticipants()[1].Birthyear, 1995);

	EXPECT_EQ(file.GetParticipants()[2].Firstname, "first3");
	EXPECT_EQ(file.GetParticipants()[2].Lastname,  "name3");
	EXPECT_EQ(file.GetParticipants()[2].WeightInGrams, 70 * 1000);
	EXPECT_EQ(file.GetParticipants()[2].Birthyear, 2000);
}