#include "tests.h"



TEST(DM4, Read_Verein1_U13)
{
	initialize();

	DM4 file("test-data/Verein1_U13 (m).dm4");

	EXPECT_EQ(file.GetSenderClubName(), "Verein1");
	EXPECT_EQ(file.GetSenderName(),     "Vereinsleiter Nachname");

	EXPECT_EQ(file.GetTournamentName(), "Turniername");
	EXPECT_EQ(file.GetTournamentDate(), "01.02.2020");
	EXPECT_EQ(file.GetTournamentPlace(), "Ort des Turniers");
	EXPECT_EQ(file.GetAgeGroup(), "U13 (m)");
	EXPECT_EQ(file.GetGender(), Gender::Male);

	ASSERT_EQ(file.GetClubs().size(), 1);
	EXPECT_EQ(file.GetClubs()[0].ID,  1);
	EXPECT_EQ(file.GetClubs()[0].Name, "Verein1");
	EXPECT_EQ(file.GetClubs()[0].Representative_Firstname, "Vereinsleiter");
	EXPECT_EQ(file.GetClubs()[0].Representative_Lastname,  "Nachname");

	ASSERT_EQ(file.GetParticipants().size(), 1);
	EXPECT_EQ(file.GetParticipants()[0].ID,  1);
	EXPECT_EQ(file.GetParticipants()[0].ClubID, 1);
	EXPECT_NE(file.GetParticipants()[0].Club, nullptr);
	EXPECT_EQ(file.GetParticipants()[0].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[0].Firstname, "VornameTN");
	EXPECT_EQ(file.GetParticipants()[0].Lastname,  "NachnameTN");
	EXPECT_EQ(file.GetParticipants()[0].Weight, 50);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 2007);
}



TEST(DM4, Read_Verein1_U15)
{
	initialize();

	DM4 file("test-data/Verein1_U15 (m).dm4");

	EXPECT_EQ(file.GetSenderClubName(), "Verein1");
	EXPECT_EQ(file.GetSenderName(),     "Vereinsleiter Nachname");

	EXPECT_EQ(file.GetTournamentName(), "13. Judoturnier");
	EXPECT_EQ(file.GetTournamentDate(), "31.07.2019");
	EXPECT_EQ(file.GetTournamentPlace(), "Turnierort");
	EXPECT_EQ(file.GetAgeGroup(), "U15 (m)");
	EXPECT_EQ(file.GetGender(), Gender::Male);

	ASSERT_EQ(file.GetClubs().size(), 1);
	EXPECT_EQ(file.GetClubs()[0].ID, 1);
	EXPECT_EQ(file.GetClubs()[0].Name, "Verein1");
	EXPECT_EQ(file.GetClubs()[0].Representative_Firstname, "Vereinsleiter");
	EXPECT_EQ(file.GetClubs()[0].Representative_Lastname, "Nachname");

	ASSERT_EQ(file.GetParticipants().size(), 2);
	EXPECT_EQ(file.GetParticipants()[0].ID, 1);
	EXPECT_EQ(file.GetParticipants()[0].ClubID, 1);
	EXPECT_NE(file.GetParticipants()[0].Club, nullptr);
	EXPECT_EQ(file.GetParticipants()[0].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[0].Firstname, "Robin");
	EXPECT_EQ(file.GetParticipants()[0].Lastname, "Kä");
	EXPECT_EQ(file.GetParticipants()[0].Weight, -1);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 2006);

	EXPECT_EQ(file.GetParticipants()[1].ID, 2);
	EXPECT_EQ(file.GetParticipants()[1].ClubID, 1);
	EXPECT_NE(file.GetParticipants()[1].Club, nullptr);
	EXPECT_EQ(file.GetParticipants()[1].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[1].Firstname, "Jan");
	EXPECT_EQ(file.GetParticipants()[1].Lastname, "Kr");
	EXPECT_EQ(file.GetParticipants()[1].Weight, 66);
	EXPECT_EQ(file.GetParticipants()[1].Birthyear, 2005);
}