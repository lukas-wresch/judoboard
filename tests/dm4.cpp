#include "tests.h"



TEST(DM4, Read_Verein1_U13)
{
	initialize();

	DM4 file("test-data/Verein1_U13 (m).dm4");

	EXPECT_EQ(file.GetTournamentName(), "Turniername");
	EXPECT_EQ(file.GetTournamentDate(), "01.02.2020");
	EXPECT_EQ(file.GetTournamentPlace(), "Ort des Turniers");
	EXPECT_EQ(file.GetAgeGroup(), "U13 (m)");

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