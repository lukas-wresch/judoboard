#include "tests.h"



TEST(DM4, Read_Verein1_U13_m)
{
	initialize();

	DM4 file("../test-data/Verein1_U13 (m).dm4");

	ASSERT_TRUE(file);

	EXPECT_EQ(file.GetSenderClubName(), "Verein1");
	EXPECT_EQ(file.GetSenderName(),     "Vereinsleiter Nachname");

	EXPECT_EQ(file.GetTournamentName(), "Turniername");
	EXPECT_EQ(file.GetTournamentDate(), "01.02.2020");
	EXPECT_EQ(file.GetTournamentPlace(), "Ort des Turniers");
	EXPECT_EQ(file.GetAgeGroup(), "U13 (m)");
	EXPECT_EQ(file.GetGender(), Gender::Male);

	ASSERT_EQ(file.GetClubs().size(), 1);
	ASSERT_TRUE(file.GetClubs()[0]);
	EXPECT_EQ(file.GetClubs()[0]->ID,  1);
	EXPECT_EQ(file.GetClubs()[0]->Name, "Verein1");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Firstname, "Vereinsleiter");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Lastname,  "Nachname");

	ASSERT_EQ(file.GetParticipants().size(), 1);
	EXPECT_EQ(file.GetParticipants()[0].ID,  1);
	EXPECT_EQ(file.GetParticipants()[0].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[0].Club);
	EXPECT_EQ(file.GetParticipants()[0].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[0].Firstname, "VornameTN");
	EXPECT_EQ(file.GetParticipants()[0].Lastname,  "NachnameTN");
	EXPECT_EQ(file.GetParticipants()[0].Gender, Gender::Male);
	EXPECT_EQ(file.GetParticipants()[0].Weight, 50);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 2007);
}



TEST(DM4, Read_Verein1_U15_m)
{
	initialize();

	DM4 file("../test-data/Verein1_U15 (m).dm4");

	ASSERT_TRUE(file);

	EXPECT_EQ(file.GetSenderClubName(), "Verein1");
	EXPECT_EQ(file.GetSenderName(),     "Vereinsleiter Nachname");

	EXPECT_EQ(file.GetTournamentName(), "13. Judoturnier");
	EXPECT_EQ(file.GetTournamentDate(), "31.07.2019");
	EXPECT_EQ(file.GetTournamentPlace(), "Turnierort");
	EXPECT_EQ(file.GetAgeGroup(), "U15 (m)");
	EXPECT_EQ(file.GetGender(), Gender::Male);

	ASSERT_EQ(file.GetClubs().size(), 1);
	ASSERT_TRUE(file.GetClubs()[0]);
	EXPECT_EQ(file.GetClubs()[0]->ID, 1);
	EXPECT_EQ(file.GetClubs()[0]->Name, "Verein1");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Firstname, "Vereinsleiter");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Lastname,  "Nachname");

	ASSERT_EQ(file.GetParticipants().size(), 2);
	EXPECT_EQ(file.GetParticipants()[0].ID, 1);
	EXPECT_EQ(file.GetParticipants()[0].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[0].Club);
	EXPECT_EQ(file.GetParticipants()[0].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[0].Firstname,  "Robin");
	EXPECT_EQ(file.GetParticipants()[0].Lastname,   "Kд");
	EXPECT_EQ(file.GetParticipants()[0].Gender, Gender::Male);
	EXPECT_EQ(file.GetParticipants()[0].Weight, -1);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 2006);

	EXPECT_EQ(file.GetParticipants()[1].ID, 2);
	EXPECT_EQ(file.GetParticipants()[1].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[1].Club);
	EXPECT_EQ(file.GetParticipants()[1].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[1].Firstname, "Jan");
	EXPECT_EQ(file.GetParticipants()[1].Lastname, "Kr");
	EXPECT_EQ(file.GetParticipants()[1].Gender, Gender::Male);
	EXPECT_EQ(file.GetParticipants()[1].Weight, 66);
	EXPECT_EQ(file.GetParticipants()[1].Birthyear, 2005);
}



TEST(DM4, Read_Verein1_U15_w)
{
	initialize();

	DM4 file("../test-data/Verein1_U15 (w).dm4");

	ASSERT_TRUE(file);

	EXPECT_EQ(file.GetSenderClubName(), "Verein1");
	EXPECT_EQ(file.GetSenderName(), "Vereinsleiter Nachname");

	EXPECT_EQ(file.GetTournamentName(), "13. Judoturnier");
	EXPECT_EQ(file.GetTournamentDate(), "31.03.2021");
	EXPECT_EQ(file.GetTournamentPlace(), "Turnierort");
	EXPECT_EQ(file.GetAgeGroup(), "U15 (w)");
	EXPECT_EQ(file.GetGender(), Gender::Female);

	ASSERT_EQ(file.GetClubs().size(), 1);
	ASSERT_TRUE(file.GetClubs()[0]);
	EXPECT_EQ(file.GetClubs()[0]->ID, 1);
	EXPECT_EQ(file.GetClubs()[0]->Name, "Verein1");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Firstname, "Vereinsleiter");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Lastname,  "Nachname");

	ASSERT_EQ(file.GetParticipants().size(), 2);
	EXPECT_EQ(file.GetParticipants()[0].ID, 1);
	EXPECT_EQ(file.GetParticipants()[0].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[0].Club);
	EXPECT_EQ(file.GetParticipants()[0].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[0].Firstname, "Joyce");
	EXPECT_EQ(file.GetParticipants()[0].Lastname, "F");
	EXPECT_EQ(file.GetParticipants()[0].Gender, Gender::Female);
	EXPECT_EQ(file.GetParticipants()[0].Weight, -1);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 2005);

	EXPECT_EQ(file.GetParticipants()[1].ID, 2);
	EXPECT_EQ(file.GetParticipants()[1].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[1].Club);
	EXPECT_EQ(file.GetParticipants()[1].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[1].Firstname, "Clara");
	EXPECT_EQ(file.GetParticipants()[1].Lastname, "H");
	EXPECT_EQ(file.GetParticipants()[1].Gender, Gender::Female);
	EXPECT_EQ(file.GetParticipants()[1].Weight, 63);
	EXPECT_EQ(file.GetParticipants()[1].Birthyear, 2006);
}



TEST(DM4, Read_Verein1_U18_w)
{
	initialize();

	DM4 file("../test-data/Verein1_U18 (w).dm4");

	ASSERT_TRUE(file);

	EXPECT_EQ(file.GetSenderClubName(), "Verein1");
	EXPECT_EQ(file.GetSenderName(), "Vereinsleiter Nachname");

	EXPECT_EQ(file.GetTournamentName(), "13. Judoturnier");
	EXPECT_EQ(file.GetTournamentDate(), "15.07.2021");
	EXPECT_EQ(file.GetTournamentPlace(), "Turnierort");
	EXPECT_EQ(file.GetAgeGroup(), "FU18 (w)");
	EXPECT_EQ(file.GetGender(), Gender::Female);

	ASSERT_EQ(file.GetClubs().size(), 1);
	ASSERT_TRUE(file.GetClubs()[0]);
	EXPECT_EQ(file.GetClubs()[0]->ID, 1);
	EXPECT_EQ(file.GetClubs()[0]->Name, "Verein1");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Firstname, "Vereinsleiter");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Lastname, "Nachname");

	ASSERT_EQ(file.GetParticipants().size(), 4);
	EXPECT_EQ(file.GetParticipants()[0].ID, 1);
	EXPECT_EQ(file.GetParticipants()[0].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[0].Club);
	EXPECT_EQ(file.GetParticipants()[0].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[0].Firstname, "Lena");
	EXPECT_EQ(file.GetParticipants()[0].Lastname, "F");
	EXPECT_EQ(file.GetParticipants()[0].Gender, Gender::Female);
	EXPECT_EQ(file.GetParticipants()[0].Weight, -1);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 2002);

	EXPECT_EQ(file.GetParticipants()[1].ID, 2);
	EXPECT_EQ(file.GetParticipants()[1].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[1].Club);
	EXPECT_EQ(file.GetParticipants()[1].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[1].Firstname, "Hanna");
	EXPECT_EQ(file.GetParticipants()[1].Lastname, "ья");
	EXPECT_EQ(file.GetParticipants()[1].Gender, Gender::Female);
	EXPECT_EQ(file.GetParticipants()[1].Weight, -1);
	EXPECT_EQ(file.GetParticipants()[1].Birthyear, 2003);

	EXPECT_EQ(file.GetParticipants()[2].ID, 3);
	EXPECT_EQ(file.GetParticipants()[2].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[2].Club);
	EXPECT_EQ(file.GetParticipants()[2].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[2].Firstname, "Catherina");
	EXPECT_EQ(file.GetParticipants()[2].Lastname, "S");
	EXPECT_EQ(file.GetParticipants()[2].Gender, Gender::Female);
	EXPECT_EQ(file.GetParticipants()[2].Weight, -1);
	EXPECT_EQ(file.GetParticipants()[2].Birthyear, 2002);

	EXPECT_EQ(file.GetParticipants()[3].ID, 4);
	EXPECT_EQ(file.GetParticipants()[3].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[3].Club);
	EXPECT_EQ(file.GetParticipants()[3].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[3].Firstname, "Alissa");
	EXPECT_EQ(file.GetParticipants()[3].Lastname, "W");
	EXPECT_EQ(file.GetParticipants()[3].Gender, Gender::Female);
	EXPECT_EQ(file.GetParticipants()[3].Weight, -1);
	EXPECT_EQ(file.GetParticipants()[3].Birthyear, 2004);
}



TEST(DM4, Read_Verein1_U21_n)
{
	initialize();

	DM4 file("../test-data/Verein1_U21 (m).dm4");

	ASSERT_TRUE(file);

	EXPECT_EQ(file.GetSenderClubName(), "Verein1");
	EXPECT_EQ(file.GetSenderName(), "Vereinsleiter Nachname");

	EXPECT_EQ(file.GetTournamentName(), "13. Judoturnier");
	EXPECT_EQ(file.GetTournamentDate(), "15.07.2021");
	EXPECT_EQ(file.GetTournamentPlace(), "Turnierort");
	EXPECT_EQ(file.GetAgeGroup(), "MU21 (m)");
	EXPECT_EQ(file.GetGender(), Gender::Male);

	ASSERT_EQ(file.GetClubs().size(), 1);
	ASSERT_TRUE(file.GetClubs()[0]);
	EXPECT_EQ(file.GetClubs()[0]->ID, 1);
	EXPECT_EQ(file.GetClubs()[0]->Name, "Verein1");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Firstname, "Vereinsleiter");
	EXPECT_EQ(file.GetClubs()[0]->Representative_Lastname,  "Nachname");

	ASSERT_EQ(file.GetParticipants().size(), 1);
	EXPECT_EQ(file.GetParticipants()[0].ID, 1);
	EXPECT_EQ(file.GetParticipants()[0].ClubID, 1);
	ASSERT_TRUE(file.GetParticipants()[0].Club);
	EXPECT_EQ(file.GetParticipants()[0].Club->Name, "Verein1");
	EXPECT_EQ(file.GetParticipants()[0].Firstname, "Dustin");
	EXPECT_EQ(file.GetParticipants()[0].Lastname, "L");
	EXPECT_EQ(file.GetParticipants()[0].Weight, -1);
	EXPECT_EQ(file.GetParticipants()[0].Birthyear, 2001);
}