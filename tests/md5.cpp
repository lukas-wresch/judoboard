#include "tests.h"



TEST(MD5, AgeGroups2020)
{
	initialize();

	{
		MD5 file1("test-data/Test-cleaned.md5");//Read MD5 file

		ASSERT_TRUE(file1);

		Database db;
		Tournament tour_temp(file1, &db);//Convert to native

		EXPECT_EQ(tour_temp.GetDatabase().GetYear(), 2020);
		ASSERT_EQ(tour_temp.GetAgeGroups().size(), 8);
		EXPECT_EQ(tour_temp.GetAgeGroups()[2]->GetName(), "Jugend u13m");
		EXPECT_EQ(tour_temp.GetAgeGroups()[2]->GetMinAge(), 10);
		EXPECT_EQ(tour_temp.GetAgeGroups()[2]->GetMaxAge(), 12);
		EXPECT_EQ(tour_temp.GetAgeGroups()[3]->GetName(), "Jugend u13w");
		EXPECT_EQ(tour_temp.GetAgeGroups()[3]->GetMinAge(), 10);
		EXPECT_EQ(tour_temp.GetAgeGroups()[3]->GetMaxAge(), 12);
		EXPECT_EQ(tour_temp.GetAgeGroups()[4]->GetName(), "Jugend u15 m");
		EXPECT_EQ(tour_temp.GetAgeGroups()[4]->GetMinAge(), 12);
		EXPECT_EQ(tour_temp.GetAgeGroups()[4]->GetMaxAge(), 14);
		EXPECT_EQ(tour_temp.GetAgeGroups()[5]->GetName(), "Jugend u15 w");
		EXPECT_EQ(tour_temp.GetAgeGroups()[5]->GetMinAge(), 12);
		EXPECT_EQ(tour_temp.GetAgeGroups()[5]->GetMaxAge(), 14);
	}
}



TEST(MD5, AgeGroups2023)
{
	initialize();

	{
		MD5 file1("test-data/BEM_U13_2023.md7");//Read MD7 file

		ASSERT_TRUE(file1);

		Database db;
		Tournament tour_temp(file1, &db);//Convert to native

		EXPECT_EQ(tour_temp.GetDatabase().GetYear(), 2023);
		ASSERT_EQ(tour_temp.GetAgeGroups().size(), 2);
		EXPECT_EQ(tour_temp.GetAgeGroups()[0]->GetMinAge(), 10);
		EXPECT_EQ(tour_temp.GetAgeGroups()[0]->GetMaxAge(), 12);
		EXPECT_EQ(tour_temp.GetAgeGroups()[1]->GetMinAge(), 10);
		EXPECT_EQ(tour_temp.GetAgeGroups()[1]->GetMaxAge(), 12);

		EXPECT_GE(tour_temp.GetSchedule().size(), 1);
	}
}



TEST(MD5, AssignAgeGroups)
{
	initialize();

	{
		MD5 file1("test-data/KEM_KT_07052023.md5");//Read MD5 file

		ASSERT_TRUE(file1);

		Database db;
		Tournament tour_temp(file1, &db);//Convert to native

		for (auto judoka : tour_temp.GetDatabase().GetAllJudokas())
		{
			auto age_group = tour_temp.GetAgeGroupOfJudoka(judoka);
			EXPECT_TRUE(age_group);
		}
	}
}



TEST(MD5, ReadTestData)
{
	initialize();

	MD5 file("test-data/Test-cleaned.md5");

	ASSERT_TRUE(file);

	file.Dump();

	//EXPECT_EQ(file.GetAssociations().size(), file.GetNumAssociations());//This seems to be broken even in the test file
	EXPECT_EQ(file.GetClubs().size(),		 file.GetNumClubs());
	EXPECT_EQ(file.GetParticipants().size(), file.GetNumParticipants());

	EXPECT_EQ(file.GetClubs().size(), 20);
	EXPECT_EQ(file.GetParticipants().size(), 142);

	ASSERT_TRUE( file.FindResult("Jugend u10 w", "-20,7 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 w", "-20,7 kg", 1)->Participant->Firstname, "Laura");
	ASSERT_FALSE(file.FindResult("Jugend u10 w", "-20,7 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u10 w", "-26,7 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 w", "-26,7 kg", 1)->Participant->Firstname, "Mila");
	EXPECT_EQ(   file.FindResult("Jugend u10 w", "-26,7 kg", 2)->Participant->Firstname, "Amalia");
	EXPECT_EQ(   file.FindResult("Jugend u10 w", "-26,7 kg", 3)->Participant->Firstname, "Julia");
	ASSERT_FALSE(file.FindResult("Jugend u10 w", "-26,7 kg", 4));

	ASSERT_TRUE( file.FindResult("Jugend u10 w", "-30,7 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 w", "-30,7 kg", 1)->Participant->Firstname, "Chiara");
	EXPECT_EQ(   file.FindResult("Jugend u10 w", "-30,7 kg", 2)->Participant->Firstname, "Lina");
	EXPECT_EQ(   file.FindResult("Jugend u10 w", "-30,7 kg", 3)->Participant->Firstname, "Narja");
	ASSERT_FALSE(file.FindResult("Jugend u10 w", "-30,7 kg", 4));

	ASSERT_TRUE( file.FindResult("Jugend u10 m", "-23,7 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-23,7 kg", 1)->Participant->Firstname, "Fabian");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-23,7 kg", 2)->Participant->Firstname, "Jonas");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-23,7 kg", 3)->Participant->Firstname, "Alexander");
	ASSERT_FALSE(file.FindResult("Jugend u10 m", "-23,7 kg", 4));

	ASSERT_TRUE( file.FindResult("Jugend u10 m", "-25,4 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-25,4 kg", 1)->Participant->Firstname, "Dominik");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-25,4 kg", 2)->Participant->Firstname, "Antonio");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-25,4 kg", 3)->Participant->Firstname, "Bjarne");
	ASSERT_FALSE(file.FindResult("Jugend u10 m", "-25,4 kg", 4));

	ASSERT_TRUE( file.FindResult("Jugend u10 m", "-26,3 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-26,3 kg", 1)->Participant->Firstname, "Diego");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-26,3 kg", 2)->Participant->Firstname, "Arthur");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-26,3 kg", 3)->Participant->Firstname, "Lenny");
	ASSERT_FALSE(file.FindResult("Jugend u10 m", "-26,3 kg", 4));

	ASSERT_TRUE( file.FindResult("Jugend u10 m", "-28,9 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-28,9 kg", 1)->Participant->Firstname, "Anton");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-28,9 kg", 2)->Participant->Firstname, "Janos");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-28,9 kg", 3)->Participant->Firstname, "Kemal");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-28,9 kg", 4)->Participant->Firstname, "David");
	ASSERT_FALSE(file.FindResult("Jugend u10 m", "-28,9 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u10 m", "-31,3 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-31,3 kg", 1)->Participant->Firstname, "Fabian");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-31,3 kg", 2)->Participant->Firstname, "Lemaliam");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-31,3 kg", 3)->Participant->Firstname, "Georg");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-31,3 kg", 4)->Participant->Firstname, "Merdan");
	ASSERT_FALSE(file.FindResult("Jugend u10 m", "-31,3 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u10 m", "-33,7 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-33,7 kg", 1)->Participant->Firstname, "Philipp");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-33,7 kg", 2)->Participant->Firstname, "Timon");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-33,7 kg", 3)->Participant->Firstname, "Emil");
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-33,7 kg", 4)->Participant->Firstname, "Mark");
	ASSERT_FALSE(file.FindResult("Jugend u10 m", "-33,7 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u10 m", "-39 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u10 m", "-39 kg", 1)->Participant->Firstname, "Leonard");
	ASSERT_FALSE(file.FindResult("Jugend u10 m", "-39 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u15 w", "-33 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 w", "-33 kg", 1)->Participant->Firstname, "Paula");
	ASSERT_FALSE(file.FindResult("Jugend u15 w", "-33 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u15 w", "-36 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 w", "-36 kg", 1)->Participant->Firstname, "Lisa");
	ASSERT_FALSE(file.FindResult("Jugend u15 w", "-36 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u15 w", "-44 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 w", "-44 kg", 1)->Participant->Firstname, "Gloria");
	ASSERT_FALSE(file.FindResult("Jugend u15 w", "-44 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u15 w", "-48 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 w", "-48 kg", 1)->Participant->Firstname, "Mia");
	EXPECT_EQ(   file.FindResult("Jugend u15 w", "-48 kg", 2)->Participant->Firstname, "Kira");
	ASSERT_FALSE(file.FindResult("Jugend u15 w", "-48 kg", 3));

	ASSERT_TRUE( file.FindResult("Jugend u15 w", "-52 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 w", "-52 kg", 1)->Participant->Firstname, "Hannah");
	ASSERT_FALSE(file.FindResult("Jugend u15 w", "-52 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u15 w", "-63 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 w", "-63 kg", 1)->Participant->Firstname, "Alina");
	ASSERT_FALSE(file.FindResult("Jugend u15 w", "-63 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u15 w", "+63 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 w", "+63 kg", 1)->Participant->Firstname, "Clara");
	ASSERT_FALSE(file.FindResult("Jugend u15 w", "+63 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u15 m", "-34 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-34 kg", 1)->Participant->Firstname, "Lukas");
	ASSERT_FALSE(file.FindResult("Jugend u15 m", "-34 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u15 m", "-37 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-37 kg", 1)->Participant->Firstname, "Luca");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-37 kg", 2)->Participant->Firstname, "Vincent");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-37 kg", 2)->Participant->Lastname,  u8"B\u00f6");
	ASSERT_FALSE(file.FindResult("Jugend u15 m", "-37 kg", 3));

	ASSERT_TRUE( file.FindResult("Jugend u15 m", "-40 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-40 kg", 1)->Participant->Firstname, "Cedric");
	ASSERT_FALSE(file.FindResult("Jugend u15 m", "-40 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u15 m", "-43 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-43 kg", 1)->Participant->Firstname, "Nico");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-43 kg", 2)->Participant->Firstname, "David");
	ASSERT_FALSE(file.FindResult("Jugend u15 m", "-43 kg", 3));

	ASSERT_TRUE( file.FindResult("Jugend u15 m", "-55 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-55 kg", 1)->Participant->Firstname, "Sebastian");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-55 kg", 2)->Participant->Firstname, "Harry");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-55 kg", 3)->Participant->Firstname, "Marlon");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-55 kg", 4)->Participant->Firstname, "Maximilian");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-55 kg", 5)->Participant->Firstname, "Jan");
	ASSERT_FALSE(file.FindResult("Jugend u15 m", "-55 kg", 6));

	ASSERT_TRUE( file.FindResult("Jugend u15 m", "-60 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-60 kg", 1)->Participant->Firstname, "Mikail");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-60 kg", 2)->Participant->Firstname, "Maurice");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-60 kg", 3)->Participant->Firstname, "Robin");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-60 kg", 4)->Participant->Firstname, "Paul");
	ASSERT_FALSE(file.FindResult("Jugend u15 m", "-60 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u15 m", "-66 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-66 kg", 1)->Participant->Firstname, "Nikita");
	EXPECT_EQ(   file.FindResult("Jugend u15 m", "-66 kg", 2)->Participant->Firstname, "Dimitri");
	ASSERT_FALSE(file.FindResult("Jugend u15 m", "-66 kg", 3));

	ASSERT_TRUE( file.FindResult("Frauen u18", "-43,1 kg", 1));
	EXPECT_EQ(   file.FindResult("Frauen u18", "-43,1 kg", 1)->Participant->Firstname, "Gina");
	ASSERT_FALSE(file.FindResult("Frauen u18", "-43,1 kg", 2));

	ASSERT_TRUE( file.FindResult("Frauen u18", "-57,7 kg", 1));
	EXPECT_EQ(   file.FindResult("Frauen u18", "-57,7 kg", 1)->Participant->Firstname, "Maria-T");
	EXPECT_EQ(   file.FindResult("Frauen u18", "-57,7 kg", 2)->Participant->Firstname, "Evelyne");
	EXPECT_EQ(   file.FindResult("Frauen u18", "-57,7 kg", 3)->Participant->Firstname, "Fia");
	ASSERT_FALSE(file.FindResult("Frauen u18", "-57,7 kg", 4));

	ASSERT_TRUE( file.FindResult(u8"M\u00e4nner u18", "-54 kg", 1));
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-54 kg", 1)->Participant->Firstname, "Simon");
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-54 kg", 2)->Participant->Firstname, "Justin");
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-54 kg", 3)->Participant->Firstname, "Vakhtang");
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-54 kg", 4)->Participant->Firstname, "Vincenzo");
	ASSERT_FALSE(file.FindResult(u8"\u00e4änner u18", "-54 kg", 5));

	ASSERT_TRUE( file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 1));
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 1)->Participant->Firstname, "Jason");
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 2)->Participant->Firstname, "Ben-N");
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 3)->Participant->Firstname, "Maximilian");
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 4)->Participant->Firstname, "Quentin");
	ASSERT_FALSE(file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 5));

	ASSERT_TRUE( file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 1));
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 1)->Participant->Firstname, "Tobias");
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 2)->Participant->Firstname, "Nikita");
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 3)->Participant->Firstname, "Ewan F");
	EXPECT_EQ(   file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 4)->Participant->Firstname, "Jan");
	ASSERT_FALSE(file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13w", "-28,1 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-28,1 kg", 1)->Participant->Firstname, "Klara");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-28,1 kg", 2)->Participant->Firstname, "Marie");
	ASSERT_FALSE(file.FindResult("Jugend u13w", "-28,1 kg", 3));

	ASSERT_TRUE( file.FindResult("Jugend u13w", "-30,3 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-30,3 kg", 1)->Participant->Firstname, "Leonie");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-30,3 kg", 2)->Participant->Firstname, "Viktoria");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-30,3 kg", 3)->Participant->Firstname, "Diana");
	ASSERT_FALSE(file.FindResult("Jugend u13w", "-30,3 kg", 4));

	ASSERT_TRUE( file.FindResult("Jugend u13w", "-33,9 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-33,9 kg", 1)->Participant->Firstname, "Valentina");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-33,9 kg", 2)->Participant->Firstname, "Luisa");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-33,9 kg", 3)->Participant->Firstname, "Mariebelle");
	ASSERT_FALSE(file.FindResult("Jugend u13w", "-33,9 kg", 4));

	ASSERT_TRUE( file.FindResult("Jugend u13w", "-36,1 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-36,1 kg", 1)->Participant->Firstname, "Vanessa");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-36,1 kg", 2)->Participant->Firstname, "Hadisha");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-36,1 kg", 3)->Participant->Firstname, "Lentje");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-36,1 kg", 4)->Participant->Firstname, "Leane");
	ASSERT_FALSE(file.FindResult("Jugend u13w", "-36,1 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13w", "-39,2 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-39,2 kg", 1)->Participant->Firstname, "Pia");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-39,2 kg", 2)->Participant->Firstname, "Rieke");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-39,2 kg", 3)->Participant->Firstname, "Vienna");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-39,2 kg", 4)->Participant->Firstname, "Mia");
	ASSERT_FALSE(file.FindResult("Jugend u13w", "-39,2 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13w", "-42,9 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-42,9 kg", 1)->Participant->Firstname, "Nika");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-42,9 kg", 2)->Participant->Firstname, "Pia");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-42,9 kg", 3)->Participant->Firstname, "Sirpa");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-42,9 kg", 4)->Participant->Firstname, "Lia");
	ASSERT_FALSE(file.FindResult("Jugend u13w", "-42,9 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13w", "-52,9 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-52,9 kg", 1)->Participant->Firstname, "Josephine");
	EXPECT_EQ(   file.FindResult("Jugend u13w", "-52,9 kg", 2)->Participant->Firstname, "Marijana");
	ASSERT_FALSE(file.FindResult("Jugend u13w", "-52,9 kg", 3));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-23,3 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-23,3 kg", 1)->Participant->Firstname, "Selim");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-23,3 kg", 2));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-28,1 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-28,1 kg", 1)->Participant->Firstname, "Milo");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-28,1 kg", 2)->Participant->Firstname, "Arne");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-28,1 kg", 3)->Participant->Firstname, "Fabian");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-28,1 kg", 4));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-28,7 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-28,7 kg", 1)->Participant->Firstname, "Arthur");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-28,7 kg", 2)->Participant->Firstname, "Damir");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-28,7 kg", 3)->Participant->Firstname, "Maximilain");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-28,7 kg", 4)->Participant->Firstname, "Moritz");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-28,7 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-30,4 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-30,4 kg", 1)->Participant->Firstname, "Benjamin");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-30,4 kg", 2)->Participant->Firstname, "Maxim");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-30,4 kg", 3)->Participant->Firstname, "Kerwin");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-30,4 kg", 4)->Participant->Firstname, "Matthies");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-30,4 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-34,4 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-34,4 kg", 1)->Participant->Firstname, "Gregor");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-34,4 kg", 2)->Participant->Firstname, "Dominik");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-34,4 kg", 3)->Participant->Firstname, "Florian");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-34,4 kg", 4)->Participant->Firstname, "Philip");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-34,4 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-35,1 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-35,1 kg", 1)->Participant->Firstname, "Silas");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-35,1 kg", 2)->Participant->Firstname, "Laurenz");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-35,1 kg", 3)->Participant->Firstname, "Amon");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-35,1 kg", 4)->Participant->Firstname, "Leonard");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-35,1 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-37,9 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-37,9 kg", 1)->Participant->Firstname, "Ewald");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-37,9 kg", 2)->Participant->Firstname, "Arvid");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-37,9 kg", 3)->Participant->Firstname, "Kai");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-37,9 kg", 4)->Participant->Firstname, "Tim");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-37,9 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-39,9 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-39,9 kg", 1)->Participant->Firstname, "Benjamin");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-39,9 kg", 2)->Participant->Firstname, "Enrico");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-39,9 kg", 3)->Participant->Firstname, "Felix");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-39,9 kg", 4)->Participant->Firstname, "Paul");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-39,9 kg", 5));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-43,7 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-43,7 kg", 1)->Participant->Firstname, "Matti");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-43,7 kg", 2)->Participant->Firstname, "Jan");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-43,7 kg", 3)->Participant->Firstname, "Jonas");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-43,7 kg", 4));

	ASSERT_TRUE( file.FindResult("Jugend u13m", "-47,7 kg", 1));
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-47,7 kg", 1)->Participant->Firstname, "Norwin");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-47,7 kg", 2)->Participant->Firstname, "Darian");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-47,7 kg", 3)->Participant->Firstname, "Luca");
	EXPECT_EQ(   file.FindResult("Jugend u13m", "-47,7 kg", 4)->Participant->Firstname, "Titus");
	ASSERT_FALSE(file.FindResult("Jugend u13m", "-47,7 kg", 5));
}



TEST(MD5, CreateTournamentFromTestData)
{
	initialize();

	MD5 file("test-data/Test-cleaned.md5");

	ASSERT_TRUE(file);

	Tournament tour(file);

	EXPECT_EQ(tour.GetDatabase().GetNumJudoka(), 142);
	EXPECT_EQ(tour.GetDatabase().GetNumClubs(),   file.GetClubs().size());

	//Check alphabetical order
	for (int i = 0; i < tour.GetDatabase().GetNumJudoka(); i++)
	{
		for (int j = i + 1; j < tour.GetDatabase().GetNumJudoka(); j++)
		{
			auto j1 = tour.GetDatabase().GetAllJudokas()[i];
			auto j2 = tour.GetDatabase().GetAllJudokas()[j];

			if (j1->GetLastname() != j2->GetLastname())
				EXPECT_LT(j1->GetLastname(), j2->GetLastname());
			else if (j1->GetFirstname() != j2->GetFirstname())
				EXPECT_LT(j1->GetFirstname(), j2->GetFirstname());
		}
	}
}



TEST(MD5, CreateTournamentFromTestData2)
{
	initialize();

	{
		MD5 file("test-data/Test-cleaned.md5");

		ASSERT_TRUE(file);

		Database db;
		Tournament tour(file, &db);

		EXPECT_EQ(file.GetNumParticipants(),     142);
		EXPECT_EQ(file.GetParticipants().size(), 142);
		//EXPECT_EQ(db.GetNumJudoka(),             142);
		//EXPECT_EQ(db.GetAllJudokas().size(),     142);
		//EXPECT_EQ(db.GetNumClubs(), 20);
		EXPECT_EQ(db.GetNumJudoka(),         0);
		EXPECT_EQ(db.GetAllJudokas().size(), 0);
		EXPECT_EQ(db.GetNumClubs(),          0);

		//Check if every color is used
		int color_count[(int)Color::Name::Max];
		for (auto& c : color_count)
			c = 0;

		for (auto table : tour.GetMatchTables())
		{
			EXPECT_GE(table->GetScheduleIndex(), 0);
			color_count[(int)table->GetColor()]++;
		}

		for (auto& c : color_count)
			EXPECT_GE(c, 1);//Every color should be used at least once

		for (auto match : tour.GetSchedule())
			EXPECT_EQ(match->GetMatID(), 1);
	}

	ZED::Core::RemoveFile("tournaments/KEM U15 KT U10 - U18.yml");
}



TEST(MD5, ImportIntoTournament)
{
	initialize();

	{
		Localizer::SetLanguage(Language::German);

		MD5 file("test-data/Test-cleaned.md5");

		ASSERT_TRUE(file);

		file.Dump();

		ASSERT_TRUE(file.GetOrganizer());
		EXPECT_EQ(file.GetOrganizer()->Description, u8"Bielefeld/G\u00fctersloh");

		Database db;
		Tournament tour(file, &db);

		ASSERT_TRUE(tour.GetOrganizer());
		EXPECT_EQ(tour.GetOrganizer()->GetName(), u8"Bielefeld/G\u00fctersloh");

		auto table = tour.FindMatchTableByDescription("Jugend u10 w -20,7 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("20,7"));
		auto results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Laura");


		table = tour.FindMatchTableByDescription("Jugend u10 w -26,7 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("26,7"));
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mila");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Amalia");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Julia");

		table = tour.FindMatchTableByDescription("Jugend u10 w -30,7 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("30,7"));
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Chiara");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lina");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Narja");

		table = tour.FindMatchTableByDescription("Jugend u10 m -23,7 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("23,7"));
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jonas");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Alexander");

		table = tour.FindMatchTableByDescription("Jugend u10 m -25,4 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("25,4"));
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Dominik");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Antonio");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Bjarne");

		table = tour.FindMatchTableByDescription("Jugend u10 m -26,3 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("26,3"));
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Diego");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arthur");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lenny");

		table = tour.FindMatchTableByDescription("Jugend u10 m -28,9 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("28,9"));
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Anton");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Janos");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kemal");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "David");

		table = tour.FindMatchTableByDescription("Jugend u10 m -31,3 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("31,3"));
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lemaliam");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Georg");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Merdan");

		table = tour.FindMatchTableByDescription("Jugend u10 m -33,7 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("33,7"));
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Philipp");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Timon");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Emil");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mark");

		table = tour.FindMatchTableByDescription("Jugend u10 m -39 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("39"));
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonard");

		table = tour.FindMatchTableByDescription("Jugend u15 w -10 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Jugend u15 w -33 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Paula");

		table = tour.FindMatchTableByDescription("Jugend u15 w -36 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lisa");

		table = tour.FindMatchTableByDescription("Jugend u15 w -44 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gloria");

		table = tour.FindMatchTableByDescription("Jugend u15 w -48 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mia");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Kira");

		table = tour.FindMatchTableByDescription("Jugend u15 w -52 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Hannah");

		table = tour.FindMatchTableByDescription("Jugend u15 w -57 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Jugend u15 w -63 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Alina");

		table = tour.FindMatchTableByDescription("Jugend u15 w +63 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Clara");

		table = tour.FindMatchTableByDescription("Jugend u15 m -34 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lukas");

		table = tour.FindMatchTableByDescription("Jugend u15 m -37 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Luca");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Vincent");

		table = tour.FindMatchTableByDescription("Jugend u15 m -40 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Cedric");

		table = tour.FindMatchTableByDescription("Jugend u15 m -43 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nico");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "David");

		table = tour.FindMatchTableByDescription("Jugend u15 m -46 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Jugend u15 m -50 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Jugend u15 m -55 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		EXPECT_EQ(table->GetType(), MatchTable::Type::DoubleElimination);
		ASSERT_GE(results.GetSize(), 5);//double elimination
		ASSERT_TRUE(results[0].Judoka);
		ASSERT_TRUE(results[1].Judoka);
		ASSERT_TRUE(results[2].Judoka);
		ASSERT_TRUE(results[3].Judoka);
		ASSERT_TRUE(results[4].Judoka);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Sebastian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Harry");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Marlon");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Maximilian");
		EXPECT_EQ(results[4].Judoka->GetFirstname(), "Jan");

		table = tour.FindMatchTableByDescription("Jugend u15 m -60 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mikail");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maurice");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Robin");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

		table = tour.FindMatchTableByDescription("Jugend u15 m -66 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nikita");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dimitri");

		table = tour.FindMatchTableByDescription("Jugend u15 m +66 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Frauen u18 -43,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gina");

		table = tour.FindMatchTableByDescription("Frauen u18 -57,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Maria-T");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Evelyne");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fia");

		table = tour.FindMatchTableByDescription(u8"M\u00e4nner u18 -54 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Simon");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Justin");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vakhtang");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Vincenzo");

		table = tour.FindMatchTableByDescription(u8"M\u00e4nner u18 -65,5 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Jason");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Ben-N");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilian");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Quentin");

		table = tour.FindMatchTableByDescription(u8"M\u00e4nner u18 -72,6 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Tobias");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Nikita");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Ewan F");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Jan");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -28,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Klara");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marie");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -30,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonie");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Viktoria");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Diana");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -33,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Valentina");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Luisa");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Mariebelle");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -36,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Vanessa");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Hadisha");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lentje");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leane");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -39,2 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Pia");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Rieke");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vienna");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mia");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -42,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nika");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Pia");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Sirpa");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Lia");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -52,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Josephine");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marijana");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -23,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Selim");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -28,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Milo");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arne");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fabian");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -28,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Arthur");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Damir");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilain");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Moritz");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -30,4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maxim");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kerwin");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Matthies");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -34,4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gregor");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dominik");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Florian");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Philip");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -35,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Silas");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Laurenz");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Amon");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leonard");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -37,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Ewald");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arvid");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kai");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Tim");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -39,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Enrico");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Felix");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -43,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Matti");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jan");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Jonas");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -47,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Norwin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Darian");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Luca");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Titus");

		EXPECT_EQ(db.GetNumJudoka(), 0);
		EXPECT_EQ(db.GetNumClubs(),  0);
	}

	ZED::Core::RemoveFile("tournaments/KEM U15 KT U10 - U18.yml");
}



TEST(MD5, ImportTestTurnierIntoTournament)
{
	initialize();

	{
		Localizer::SetLanguage(Language::German);

		MD5 file("test-data/Testturnier.md5");

		ASSERT_TRUE(file);

		file.Dump();

		Database db;
		Tournament tour(file, &db);

		auto table = tour.FindMatchTableByDescription("Jugend u15 m -37 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMinWeight(), Weight("10"));
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("37"));


		table = tour.FindMatchTableByDescription("Jugend u15 m -40 kg");
		ASSERT_TRUE(table);
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMinWeight(), Weight("10"));
		EXPECT_EQ(((Weightclass*)table->GetFilter())->GetMaxWeight(), Weight("40"));
	}

	ZED::Core::RemoveFile("tournaments/Testturnier.yml");
}



TEST(MD5, ImportIntoTournament_LoadAfterSave)
{
	initialize();

	MD5 file("test-data/Test-cleaned.md5");

	ASSERT_TRUE(file);

	{
		Database db;
		{
			Tournament tour(file, &db);
			tour.SetName("deleteMe");
			tour.Save();
		}

		Tournament tour2("deleteMe");

		auto table = tour2.FindMatchTableByDescription("Jugend u10 w -20,7 kg");
		ASSERT_TRUE(table);
		auto results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Laura");


		table = tour2.FindMatchTableByDescription("Jugend u10 w -26,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mila");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Amalia");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Julia");

		table = tour2.FindMatchTableByDescription("Jugend u10 w -30,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Chiara");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lina");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Narja");

		table = tour2.FindMatchTableByDescription("Jugend u10 m -23,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jonas");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Alexander");

		table = tour2.FindMatchTableByDescription("Jugend u10 m -25,4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Dominik");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Antonio");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Bjarne");

		table = tour2.FindMatchTableByDescription("Jugend u10 m -26,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Diego");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arthur");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lenny");

		table = tour2.FindMatchTableByDescription("Jugend u10 m -28,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Anton");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Janos");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kemal");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "David");

		table = tour2.FindMatchTableByDescription("Jugend u10 m -31,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lemaliam");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Georg");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Merdan");

		table = tour2.FindMatchTableByDescription("Jugend u10 m -33,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Philipp");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Timon");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Emil");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mark");

		table = tour2.FindMatchTableByDescription("Jugend u10 m -39 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonard");

		table = tour2.FindMatchTableByDescription("Jugend u15 w -10 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour2.FindMatchTableByDescription("Jugend u15 w -33 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Paula");

		table = tour2.FindMatchTableByDescription("Jugend u15 w -36 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lisa");

		table = tour2.FindMatchTableByDescription("Jugend u15 w -44 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gloria");

		table = tour2.FindMatchTableByDescription("Jugend u15 w -48 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mia");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Kira");

		table = tour2.FindMatchTableByDescription("Jugend u15 w -52 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Hannah");

		table = tour2.FindMatchTableByDescription("Jugend u15 w -57 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour2.FindMatchTableByDescription("Jugend u15 w -63 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Alina");

		table = tour2.FindMatchTableByDescription("Jugend u15 w +63 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Clara");

		table = tour2.FindMatchTableByDescription("Jugend u15 m -34 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lukas");

		table = tour2.FindMatchTableByDescription("Jugend u15 m -37 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Luca");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Vincent");

		table = tour2.FindMatchTableByDescription("Jugend u15 m -40 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Cedric");

		table = tour2.FindMatchTableByDescription("Jugend u15 m -43 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nico");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "David");

		table = tour2.FindMatchTableByDescription("Jugend u15 m -46 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour2.FindMatchTableByDescription("Jugend u15 m -50 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour2.FindMatchTableByDescription("Jugend u15 m -55 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_GE(results.GetSize(), 5);//double elimination
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Sebastian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Harry");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Marlon");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Maximilian");
		EXPECT_EQ(results[4].Judoka->GetFirstname(), "Jan");

		table = tour2.FindMatchTableByDescription("Jugend u15 m -60 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mikail");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maurice");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Robin");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

		table = tour2.FindMatchTableByDescription("Jugend u15 m -66 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nikita");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dimitri");

		table = tour2.FindMatchTableByDescription("Jugend u15 m +66 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour2.FindMatchTableByDescription("Frauen u18 -43,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gina");

		table = tour2.FindMatchTableByDescription("Frauen u18 -57,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Maria-T");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Evelyne");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fia");

		table = tour2.FindMatchTableByDescription(u8"M\u00e4nner u18 -54 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Simon");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Justin");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vakhtang");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Vincenzo");

		table = tour2.FindMatchTableByDescription(u8"M\u00e4nner u18 -65,5 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Jason");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Ben-N");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilian");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Quentin");

		table = tour2.FindMatchTableByDescription(u8"M\u00e4nner u18 -72,6 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Tobias");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Nikita");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Ewan F");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Jan");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13w -28,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Klara");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marie");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13w -30,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonie");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Viktoria");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Diana");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13w -33,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Valentina");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Luisa");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Mariebelle");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13w -36,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Vanessa");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Hadisha");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lentje");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leane");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13w -39,2 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Pia");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Rieke");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vienna");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mia");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13w -42,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nika");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Pia");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Sirpa");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Lia");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13w -52,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Josephine");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marijana");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -23,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Selim");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -28,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Milo");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arne");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fabian");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -28,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Arthur");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Damir");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilain");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Moritz");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -30,4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maxim");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kerwin");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Matthies");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -34,4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gregor");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dominik");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Florian");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Philip");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -35,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Silas");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Laurenz");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Amon");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leonard");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -37,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Ewald");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arvid");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kai");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Tim");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -39,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Enrico");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Felix");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -43,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Matti");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jan");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Jonas");

		table = tour2.FindMatchTableByDescription(u8"Jugend u13m -47,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Norwin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Darian");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Luca");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Titus");
	}


	ZED::Core::RemoveFile("tournaments/KEM U15 KT U10 - U18.yml");
	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	ZED::Core::RemoveFile("deleteMe");
}



TEST(MD5, Export)
{
	initialize();

	{
		MD5 file("test-data/Test-cleaned.md5");
		const auto hash_expected = ZED::SHA512(ZED::Blob(ZED::File("test-data/Test-cleaned.md5")));

		ASSERT_TRUE(file);

		file.Dump();

		EXPECT_TRUE(file.Save("deleteMe"));

		const auto hash_has = ZED::SHA512(ZED::Blob(ZED::File("deleteMe")));

		EXPECT_EQ(hash_has, hash_expected);
	}

	ZED::Core::RemoveFile("deleteMe");
	ZED::Core::RemoveFile("tournaments/KEM U15 KT U10 - U18.yml");
}



TEST(MD5, ConvertToMD5)
{
	initialize();

	{
		MD5 file1("test-data/Test-cleaned.md5");//Read MD5 file

		ASSERT_TRUE(file1);

		Database db;
		Tournament tour_temp(file1, &db);//Convert to native

		MD5 file(tour_temp);//Convert back to MD5

		//ASSERT_EQ(file.GetLottery().size(), file1.GetLottery().size());
		for (int i = 0; i < file.GetLottery().size(); ++i)
		{
			auto assoc = file.FindAssociation(file.GetLottery()[i].AssociationID);
			ASSERT_TRUE(assoc);

			auto assoc1 = file1.FindAssociationByName(assoc->Description);
			ASSERT_TRUE(assoc1);
			auto file1_lot = file1.FindLotOfAssociation(assoc1->ID);

			EXPECT_EQ(file.GetLottery()[i].StartNo, file1_lot);
		}

		ASSERT_TRUE(file.GetOrganizer());
		ASSERT_TRUE(file1.GetOrganizer());
		EXPECT_EQ(file.GetOrganizer()->Description, file1.GetOrganizer()->Description);

		EXPECT_EQ(file.GetClubs().size(), file1.GetClubs().size());
		EXPECT_EQ(file.GetParticipants().size(), 142);

		ASSERT_TRUE(file1.FindResult("Jugend u10 w", "-20,7 kg", 1));
		EXPECT_EQ(file1.FindResult("Jugend u10 w", "-20,7 kg", 1)->Participant->Firstname, "Laura");

		ASSERT_TRUE(file.FindResult("Jugend u10 w", "-20,7 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 w", "-20,7 kg", 1)->Participant->Firstname, "Laura");
		ASSERT_FALSE(file.FindResult("Jugend u10 w", "-20,7 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u10 w", "-26,7 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 w", "-26,7 kg", 1)->Participant->Firstname, "Mila");
		EXPECT_EQ(file.FindResult("Jugend u10 w", "-26,7 kg", 2)->Participant->Firstname, "Amalia");
		EXPECT_EQ(file.FindResult("Jugend u10 w", "-26,7 kg", 3)->Participant->Firstname, "Julia");
		ASSERT_FALSE(file.FindResult("Jugend u10 w", "-26,7 kg", 4));

		ASSERT_TRUE(file.FindResult("Jugend u10 w", "-30,7 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 w", "-30,7 kg", 1)->Participant->Firstname, "Chiara");
		EXPECT_EQ(file.FindResult("Jugend u10 w", "-30,7 kg", 2)->Participant->Firstname, "Lina");
		EXPECT_EQ(file.FindResult("Jugend u10 w", "-30,7 kg", 3)->Participant->Firstname, "Narja");
		ASSERT_FALSE(file.FindResult("Jugend u10 w", "-30,7 kg", 4));

		ASSERT_TRUE(file.FindResult("Jugend u10 m", "-23,7 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-23,7 kg", 1)->Participant->Firstname, "Fabian");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-23,7 kg", 2)->Participant->Firstname, "Jonas");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-23,7 kg", 3)->Participant->Firstname, "Alexander");
		ASSERT_FALSE(file.FindResult("Jugend u10 m", "-23,7 kg", 4));

		ASSERT_TRUE(file.FindResult("Jugend u10 m", "-25,4 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-25,4 kg", 1)->Participant->Firstname, "Dominik");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-25,4 kg", 2)->Participant->Firstname, "Antonio");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-25,4 kg", 3)->Participant->Firstname, "Bjarne");
		ASSERT_FALSE(file.FindResult("Jugend u10 m", "-25,4 kg", 4));

		ASSERT_TRUE(file.FindResult("Jugend u10 m", "-26,3 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-26,3 kg", 1)->Participant->Firstname, "Diego");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-26,3 kg", 2)->Participant->Firstname, "Arthur");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-26,3 kg", 3)->Participant->Firstname, "Lenny");
		ASSERT_FALSE(file.FindResult("Jugend u10 m", "-26,3 kg", 4));

		ASSERT_TRUE(file.FindResult("Jugend u10 m", "-28,9 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-28,9 kg", 1)->Participant->Firstname, "Anton");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-28,9 kg", 2)->Participant->Firstname, "Janos");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-28,9 kg", 3)->Participant->Firstname, "Kemal");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-28,9 kg", 4)->Participant->Firstname, "David");
		ASSERT_FALSE(file.FindResult("Jugend u10 m", "-28,9 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u10 m", "-31,3 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-31,3 kg", 1)->Participant->Firstname, "Fabian");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-31,3 kg", 2)->Participant->Firstname, "Lemaliam");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-31,3 kg", 3)->Participant->Firstname, "Georg");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-31,3 kg", 4)->Participant->Firstname, "Merdan");
		ASSERT_FALSE(file.FindResult("Jugend u10 m", "-31,3 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u10 m", "-33,7 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-33,7 kg", 1)->Participant->Firstname, "Philipp");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-33,7 kg", 2)->Participant->Firstname, "Timon");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-33,7 kg", 3)->Participant->Firstname, "Emil");
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-33,7 kg", 4)->Participant->Firstname, "Mark");
		ASSERT_FALSE(file.FindResult("Jugend u10 m", "-33,7 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u10 m", "-39 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u10 m", "-39 kg", 1)->Participant->Firstname, "Leonard");
		ASSERT_FALSE(file.FindResult("Jugend u10 m", "-39 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u15 w", "-33 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 w", "-33 kg", 1)->Participant->Firstname, "Paula");
		ASSERT_FALSE(file.FindResult("Jugend u15 w", "-33 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u15 w", "-36 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 w", "-36 kg", 1)->Participant->Firstname, "Lisa");
		ASSERT_FALSE(file.FindResult("Jugend u15 w", "-36 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u15 w", "-44 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 w", "-44 kg", 1)->Participant->Firstname, "Gloria");
		ASSERT_FALSE(file.FindResult("Jugend u15 w", "-44 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u15 w", "-48 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 w", "-48 kg", 1)->Participant->Firstname, "Mia");
		EXPECT_EQ(file.FindResult("Jugend u15 w", "-48 kg", 2)->Participant->Firstname, "Kira");
		ASSERT_FALSE(file.FindResult("Jugend u15 w", "-48 kg", 3));

		ASSERT_TRUE(file.FindResult("Jugend u15 w", "-52 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 w", "-52 kg", 1)->Participant->Firstname, "Hannah");
		ASSERT_FALSE(file.FindResult("Jugend u15 w", "-52 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u15 w", "-63 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 w", "-63 kg", 1)->Participant->Firstname, "Alina");
		ASSERT_FALSE(file.FindResult("Jugend u15 w", "-63 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u15 w", "+63 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 w", "+63 kg", 1)->Participant->Firstname, "Clara");
		ASSERT_FALSE(file.FindResult("Jugend u15 w", "+63 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u15 m", "-34 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-34 kg", 1)->Participant->Firstname, "Lukas");
		ASSERT_FALSE(file.FindResult("Jugend u15 m", "-34 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u15 m", "-37 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-37 kg", 1)->Participant->Firstname, "Luca");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-37 kg", 2)->Participant->Firstname, "Vincent");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-37 kg", 2)->Participant->Lastname, u8"B\u00f6");
		ASSERT_FALSE(file.FindResult("Jugend u15 m", "-37 kg", 3));

		ASSERT_TRUE(file.FindResult("Jugend u15 m", "-40 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-40 kg", 1)->Participant->Firstname, "Cedric");
		ASSERT_FALSE(file.FindResult("Jugend u15 m", "-40 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u15 m", "-43 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-43 kg", 1)->Participant->Firstname, "Nico");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-43 kg", 2)->Participant->Firstname, "David");
		ASSERT_FALSE(file.FindResult("Jugend u15 m", "-43 kg", 3));

		ASSERT_TRUE(file.FindResult("Jugend u15 m", "-55 kg", 1));//double elimination
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-55 kg", 1)->Participant->Firstname, "Sebastian");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-55 kg", 2)->Participant->Firstname, "Harry");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-55 kg", 3)->Participant->Firstname, "Marlon");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-55 kg", 4)->Participant->Firstname, "Maximilian");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-55 kg", 5)->Participant->Firstname, "Jan");
		ASSERT_FALSE(file.FindResult("Jugend u15 m", "-55 kg", 6));

		ASSERT_TRUE(file.FindResult("Jugend u15 m", "-60 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-60 kg", 1)->Participant->Firstname, "Mikail");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-60 kg", 2)->Participant->Firstname, "Maurice");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-60 kg", 3)->Participant->Firstname, "Robin");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-60 kg", 4)->Participant->Firstname, "Paul");
		ASSERT_FALSE(file.FindResult("Jugend u15 m", "-60 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u15 m", "-66 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-66 kg", 1)->Participant->Firstname, "Nikita");
		EXPECT_EQ(file.FindResult("Jugend u15 m", "-66 kg", 2)->Participant->Firstname, "Dimitri");
		ASSERT_FALSE(file.FindResult("Jugend u15 m", "-66 kg", 3));

		ASSERT_TRUE(file.FindResult("Frauen u18", "-43,1 kg", 1));
		EXPECT_EQ(file.FindResult("Frauen u18", "-43,1 kg", 1)->Participant->Firstname, "Gina");
		ASSERT_FALSE(file.FindResult("Frauen u18", "-43,1 kg", 2));

		ASSERT_TRUE(file.FindResult("Frauen u18", "-57,7 kg", 1));
		EXPECT_EQ(file.FindResult("Frauen u18", "-57,7 kg", 1)->Participant->Firstname, "Maria-T");
		EXPECT_EQ(file.FindResult("Frauen u18", "-57,7 kg", 2)->Participant->Firstname, "Evelyne");
		EXPECT_EQ(file.FindResult("Frauen u18", "-57,7 kg", 3)->Participant->Firstname, "Fia");
		ASSERT_FALSE(file.FindResult("Frauen u18", "-57,7 kg", 4));

		ASSERT_TRUE(file.FindResult(u8"M\u00e4nner u18", "-54 kg", 1));
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-54 kg", 1)->Participant->Firstname, "Simon");
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-54 kg", 2)->Participant->Firstname, "Justin");
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-54 kg", 3)->Participant->Firstname, "Vakhtang");
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-54 kg", 4)->Participant->Firstname, "Vincenzo");
		ASSERT_FALSE(file.FindResult(u8"\u00e4änner u18", "-54 kg", 5));

		ASSERT_TRUE(file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 1));
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 1)->Participant->Firstname, "Jason");
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 2)->Participant->Firstname, "Ben-N");
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 3)->Participant->Firstname, "Maximilian");
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 4)->Participant->Firstname, "Quentin");
		ASSERT_FALSE(file.FindResult(u8"M\u00e4nner u18", "-65,5 kg", 5));

		ASSERT_TRUE(file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 1));
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 1)->Participant->Firstname, "Tobias");
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 2)->Participant->Firstname, "Nikita");
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 3)->Participant->Firstname, "Ewan F");
		EXPECT_EQ(file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 4)->Participant->Firstname, "Jan");
		ASSERT_FALSE(file.FindResult(u8"M\u00e4nner u18", "-72,6 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13w", "-28,1 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13w", "-28,1 kg", 1)->Participant->Firstname, "Klara");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-28,1 kg", 2)->Participant->Firstname, "Marie");
		ASSERT_FALSE(file.FindResult("Jugend u13w", "-28,1 kg", 3));

		ASSERT_TRUE(file.FindResult("Jugend u13w", "-30,3 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13w", "-30,3 kg", 1)->Participant->Firstname, "Leonie");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-30,3 kg", 2)->Participant->Firstname, "Viktoria");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-30,3 kg", 3)->Participant->Firstname, "Diana");
		ASSERT_FALSE(file.FindResult("Jugend u13w", "-30,3 kg", 4));

		ASSERT_TRUE(file.FindResult("Jugend u13w", "-33,9 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13w", "-33,9 kg", 1)->Participant->Firstname, "Valentina");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-33,9 kg", 2)->Participant->Firstname, "Luisa");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-33,9 kg", 3)->Participant->Firstname, "Mariebelle");
		ASSERT_FALSE(file.FindResult("Jugend u13w", "-33,9 kg", 4));

		ASSERT_TRUE(file.FindResult("Jugend u13w", "-36,1 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13w", "-36,1 kg", 1)->Participant->Firstname, "Vanessa");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-36,1 kg", 2)->Participant->Firstname, "Hadisha");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-36,1 kg", 3)->Participant->Firstname, "Lentje");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-36,1 kg", 4)->Participant->Firstname, "Leane");
		ASSERT_FALSE(file.FindResult("Jugend u13w", "-36,1 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13w", "-39,2 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13w", "-39,2 kg", 1)->Participant->Firstname, "Pia");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-39,2 kg", 2)->Participant->Firstname, "Rieke");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-39,2 kg", 3)->Participant->Firstname, "Vienna");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-39,2 kg", 4)->Participant->Firstname, "Mia");
		ASSERT_FALSE(file.FindResult("Jugend u13w", "-39,2 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13w", "-42,9 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13w", "-42,9 kg", 1)->Participant->Firstname, "Nika");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-42,9 kg", 2)->Participant->Firstname, "Pia");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-42,9 kg", 3)->Participant->Firstname, "Sirpa");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-42,9 kg", 4)->Participant->Firstname, "Lia");
		ASSERT_FALSE(file.FindResult("Jugend u13w", "-42,9 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13w", "-52,9 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13w", "-52,9 kg", 1)->Participant->Firstname, "Josephine");
		EXPECT_EQ(file.FindResult("Jugend u13w", "-52,9 kg", 2)->Participant->Firstname, "Marijana");
		ASSERT_FALSE(file.FindResult("Jugend u13w", "-52,9 kg", 3));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-23,3 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-23,3 kg", 1)->Participant->Firstname, "Selim");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-23,3 kg", 2));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-28,1 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-28,1 kg", 1)->Participant->Firstname, "Milo");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-28,1 kg", 2)->Participant->Firstname, "Arne");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-28,1 kg", 3)->Participant->Firstname, "Fabian");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-28,1 kg", 4));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-28,7 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-28,7 kg", 1)->Participant->Firstname, "Arthur");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-28,7 kg", 2)->Participant->Firstname, "Damir");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-28,7 kg", 3)->Participant->Firstname, "Maximilain");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-28,7 kg", 4)->Participant->Firstname, "Moritz");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-28,7 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-30,4 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-30,4 kg", 1)->Participant->Firstname, "Benjamin");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-30,4 kg", 2)->Participant->Firstname, "Maxim");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-30,4 kg", 3)->Participant->Firstname, "Kerwin");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-30,4 kg", 4)->Participant->Firstname, "Matthies");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-30,4 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-34,4 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-34,4 kg", 1)->Participant->Firstname, "Gregor");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-34,4 kg", 2)->Participant->Firstname, "Dominik");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-34,4 kg", 3)->Participant->Firstname, "Florian");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-34,4 kg", 4)->Participant->Firstname, "Philip");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-34,4 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-35,1 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-35,1 kg", 1)->Participant->Firstname, "Silas");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-35,1 kg", 2)->Participant->Firstname, "Laurenz");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-35,1 kg", 3)->Participant->Firstname, "Amon");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-35,1 kg", 4)->Participant->Firstname, "Leonard");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-35,1 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-37,9 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-37,9 kg", 1)->Participant->Firstname, "Ewald");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-37,9 kg", 2)->Participant->Firstname, "Arvid");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-37,9 kg", 3)->Participant->Firstname, "Kai");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-37,9 kg", 4)->Participant->Firstname, "Tim");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-37,9 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-39,9 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-39,9 kg", 1)->Participant->Firstname, "Benjamin");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-39,9 kg", 2)->Participant->Firstname, "Enrico");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-39,9 kg", 3)->Participant->Firstname, "Felix");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-39,9 kg", 4)->Participant->Firstname, "Paul");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-39,9 kg", 5));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-43,7 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-43,7 kg", 1)->Participant->Firstname, "Matti");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-43,7 kg", 2)->Participant->Firstname, "Jan");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-43,7 kg", 3)->Participant->Firstname, "Jonas");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-43,7 kg", 4));

		ASSERT_TRUE(file.FindResult("Jugend u13m", "-47,7 kg", 1));
		EXPECT_EQ(file.FindResult("Jugend u13m", "-47,7 kg", 1)->Participant->Firstname, "Norwin");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-47,7 kg", 2)->Participant->Firstname, "Darian");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-47,7 kg", 3)->Participant->Firstname, "Luca");
		EXPECT_EQ(file.FindResult("Jugend u13m", "-47,7 kg", 4)->Participant->Firstname, "Titus");
		ASSERT_FALSE(file.FindResult("Jugend u13m", "-47,7 kg", 5));
	}

	ZED::Core::RemoveFile("tournaments/KEM U15 KT U10 - U18.yml");
}



TEST(MD5, ConvertToMD5AndBack)
{
	initialize();

	{
		MD5 file("test-data/Test-cleaned.md5");//Read MD5 file

		ASSERT_TRUE(file);

		Database db;
		Tournament tour_temp(file, &db);//Convert to native

		MD5 md5_tournament(tour_temp);//Convert back to MD5

		Tournament tour(md5_tournament, &db);//Convert back to native

		for (auto [assoc_id, lot] : tour.GetLots())
		{
			auto assoc_native = tour.GetDatabase().FindAssociation(assoc_id);

			if (assoc_native)
			{
				auto assoc = file.FindAssociationByName(assoc_native->GetName());
				ASSERT_TRUE(assoc);

				EXPECT_EQ(lot, file.FindLotOfAssociation(assoc->ID));
			}

			else
			{
				auto club_native = tour.GetDatabase().FindClub(assoc_id);
				ASSERT_TRUE(club_native);

				auto club = file.FindClubByName(club_native->GetName());
				ASSERT_TRUE(club);

				EXPECT_EQ(lot, file.FindLotOfAssociation(club->ID));
			}
		}

		auto table = tour.FindMatchTableByDescription("Jugend u10 w -20,7 kg");
		ASSERT_TRUE(table);
		auto results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Laura");


		table = tour.FindMatchTableByDescription("Jugend u10 w -26,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mila");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Amalia");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Julia");

		table = tour.FindMatchTableByDescription("Jugend u10 w -30,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Chiara");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lina");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Narja");

		table = tour.FindMatchTableByDescription("Jugend u10 m -23,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jonas");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Alexander");

		table = tour.FindMatchTableByDescription("Jugend u10 m -25,4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Dominik");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Antonio");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Bjarne");

		table = tour.FindMatchTableByDescription("Jugend u10 m -26,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Diego");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arthur");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lenny");

		table = tour.FindMatchTableByDescription("Jugend u10 m -28,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Anton");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Janos");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kemal");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "David");

		table = tour.FindMatchTableByDescription("Jugend u10 m -31,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lemaliam");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Georg");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Merdan");

		table = tour.FindMatchTableByDescription("Jugend u10 m -33,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Philipp");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Timon");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Emil");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mark");

		table = tour.FindMatchTableByDescription("Jugend u10 m -39 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonard");

		table = tour.FindMatchTableByDescription("Jugend u15 w -10 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Jugend u15 w -33 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Paula");

		table = tour.FindMatchTableByDescription("Jugend u15 w -36 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lisa");

		table = tour.FindMatchTableByDescription("Jugend u15 w -44 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gloria");

		table = tour.FindMatchTableByDescription("Jugend u15 w -48 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mia");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Kira");

		table = tour.FindMatchTableByDescription("Jugend u15 w -52 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Hannah");

		table = tour.FindMatchTableByDescription("Jugend u15 w -57 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Jugend u15 w -63 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Alina");

		table = tour.FindMatchTableByDescription("Jugend u15 w +63 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Clara");

		table = tour.FindMatchTableByDescription("Jugend u15 m -34 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lukas");

		table = tour.FindMatchTableByDescription("Jugend u15 m -37 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Luca");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Vincent");

		table = tour.FindMatchTableByDescription("Jugend u15 m -40 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Cedric");

		table = tour.FindMatchTableByDescription("Jugend u15 m -43 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nico");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "David");

		table = tour.FindMatchTableByDescription("Jugend u15 m -46 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Jugend u15 m -50 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Jugend u15 m -55 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_GE(results.GetSize(), 5);//double elimination
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Sebastian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Harry");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Marlon");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Maximilian");
		EXPECT_EQ(results[4].Judoka->GetFirstname(), "Jan");

		table = tour.FindMatchTableByDescription("Jugend u15 m -60 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mikail");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maurice");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Robin");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

		table = tour.FindMatchTableByDescription("Jugend u15 m -66 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nikita");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dimitri");

		table = tour.FindMatchTableByDescription("Jugend u15 m +66 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 0);

		table = tour.FindMatchTableByDescription("Frauen u18 -43,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gina");

		table = tour.FindMatchTableByDescription("Frauen u18 -57,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Maria-T");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Evelyne");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fia");

		table = tour.FindMatchTableByDescription(u8"M\u00e4nner u18 -54 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Simon");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Justin");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vakhtang");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Vincenzo");

		table = tour.FindMatchTableByDescription(u8"M\u00e4nner u18 -65,5 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Jason");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Ben-N");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilian");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Quentin");

		table = tour.FindMatchTableByDescription(u8"M\u00e4nner u18 -72,6 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Tobias");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Nikita");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Ewan F");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Jan");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -28,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Klara");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marie");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -30,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonie");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Viktoria");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Diana");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -33,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Valentina");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Luisa");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Mariebelle");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -36,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Vanessa");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Hadisha");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lentje");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leane");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -39,2 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Pia");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Rieke");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vienna");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mia");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -42,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nika");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Pia");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Sirpa");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Lia");

		table = tour.FindMatchTableByDescription(u8"Jugend u13w -52,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Josephine");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marijana");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -23,3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Selim");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -28,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Milo");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arne");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fabian");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -28,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Arthur");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Damir");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilain");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Moritz");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -30,4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maxim");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kerwin");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Matthies");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -34,4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gregor");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dominik");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Florian");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Philip");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -35,1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Silas");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Laurenz");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Amon");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leonard");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -37,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Ewald");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arvid");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kai");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Tim");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -39,9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Enrico");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Felix");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -43,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Matti");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jan");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Jonas");

		table = tour.FindMatchTableByDescription(u8"Jugend u13m -47,7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.GetSize(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Norwin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Darian");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Luca");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Titus");
	}

	ZED::Core::RemoveFile("tournaments/KEM U15 KT U10 - U18.yml");
}



TEST(MD5, CalculateStartPosBasedOnLots)
{
	initialize();

	MD5 file("test-data/lot-to-startpos.md5");

	ASSERT_TRUE(file);

	Tournament tour(file);

	file.Dump();

	auto table = tour.GetMatchTables()[0];

	auto j1 = table->GetJudokaByStartPosition(0);
	auto j2 = table->GetJudokaByStartPosition(1);
	auto j3 = table->GetJudokaByStartPosition(2);
	auto j4 = table->GetJudokaByStartPosition(3);
	auto j5 = table->GetJudokaByStartPosition(4);
	auto j6 = table->GetJudokaByStartPosition(5);
	auto j7 = table->GetJudokaByStartPosition(6);
	auto j8 = table->GetJudokaByStartPosition(7);

	ASSERT_TRUE(j1);
	ASSERT_TRUE(j2);
	ASSERT_TRUE(j3);
	ASSERT_TRUE(j4);
	ASSERT_TRUE(j5);
	ASSERT_TRUE(j6);
	ASSERT_TRUE(j7);
	ASSERT_TRUE(j8);

	EXPECT_EQ(j1->GetFirstname(), "v5");
	EXPECT_EQ(j2->GetFirstname(), "v6");
	EXPECT_EQ(j3->GetFirstname(), "v7");
	EXPECT_EQ(j4->GetFirstname(), "v8");
	EXPECT_EQ(j5->GetFirstname(), "v1");
	EXPECT_EQ(j6->GetFirstname(), "v2");
	EXPECT_EQ(j7->GetFirstname(), "v3");
	EXPECT_EQ(j8->GetFirstname(), "v4");

	auto c1 = tour.FindClubByName("club1");
	auto c2 = tour.FindClubByName("club2");
	ASSERT_TRUE(c1);
	ASSERT_TRUE(c2);


	table->SetStartPosition(j4, 4);
	table->SetStartPosition(j3, 5);
	table->SetStartPosition(j2, 6);
	table->SetStartPosition(j1, 7);

	tour.PerformLottery();

	while (tour.GetLotOfAssociation(*c1) != 1)
		tour.PerformLottery();

	tour.GenerateSchedule();

	table = tour.GetMatchTables()[0];

	j1 = table->GetJudokaByStartPosition(0);
	j2 = table->GetJudokaByStartPosition(1);
	j3 = table->GetJudokaByStartPosition(2);
	j4 = table->GetJudokaByStartPosition(3);
	j5 = table->GetJudokaByStartPosition(4);
	j6 = table->GetJudokaByStartPosition(5);
	j7 = table->GetJudokaByStartPosition(6);
	j8 = table->GetJudokaByStartPosition(7);

	EXPECT_EQ(j1->GetClub()->GetName(), "club2");
	EXPECT_EQ(j2->GetClub()->GetName(), "club2");
	EXPECT_EQ(j3->GetClub()->GetName(), "club2");
	EXPECT_EQ(j4->GetClub()->GetName(), "club2");
	EXPECT_EQ(j5->GetClub()->GetName(), "club1");
	EXPECT_EQ(j6->GetClub()->GetName(), "club1");
	EXPECT_EQ(j7->GetClub()->GetName(), "club1");
	EXPECT_EQ(j8->GetClub()->GetName(), "club1");



	tour.PerformLottery();

	while (tour.GetLotOfAssociation(*c1) != 0)
		tour.PerformLottery();

	tour.GenerateSchedule();

	table = tour.GetMatchTables()[0];

	j1 = table->GetJudokaByStartPosition(0);
	j2 = table->GetJudokaByStartPosition(1);
	j3 = table->GetJudokaByStartPosition(2);
	j4 = table->GetJudokaByStartPosition(3);
	j5 = table->GetJudokaByStartPosition(4);
	j6 = table->GetJudokaByStartPosition(5);
	j7 = table->GetJudokaByStartPosition(6);
	j8 = table->GetJudokaByStartPosition(7);

	EXPECT_EQ(j1->GetClub()->GetName(), "club1");
	EXPECT_EQ(j2->GetClub()->GetName(), "club1");
	EXPECT_EQ(j3->GetClub()->GetName(), "club1");
	EXPECT_EQ(j4->GetClub()->GetName(), "club1");
	EXPECT_EQ(j5->GetClub()->GetName(), "club2");
	EXPECT_EQ(j6->GetClub()->GetName(), "club2");
	EXPECT_EQ(j7->GetClub()->GetName(), "club2");
	EXPECT_EQ(j8->GetClub()->GetName(), "club2");
}



TEST(MD5, ReadStructureData1)
{
	initialize();

	MD5 file("test-data/structure-1.md5");

	ASSERT_TRUE(file);

	file.Dump();

	ASSERT_TRUE(file.GetOrganizer());
	auto organizer = file.GetOrganizer();
	EXPECT_EQ(organizer->Description, "International");
	EXPECT_EQ(organizer->Tier, 2);
}



TEST(MD5, ReadStructureData2)
{
	initialize();

	MD5 file("test-data/structure-2.md5");

	ASSERT_TRUE(file);

	file.Dump();

	ASSERT_TRUE(file.GetOrganizer());
	auto organizer = file.GetOrganizer();
	EXPECT_EQ(organizer->Description, "Bayern");
	EXPECT_EQ(organizer->Tier, 6);
}



TEST(MD5, ReadStructureData3)
{
	initialize();

	MD5 file("test-data/structure-3.md5");

	ASSERT_TRUE(file);

	file.Dump();

	ASSERT_TRUE(file.GetOrganizer());
	auto organizer = file.GetOrganizer();
	EXPECT_EQ(organizer->Description, "Berlin");
	EXPECT_EQ(organizer->Tier, 6);
}



TEST(MD5, ExportStructureData)
{
	initialize();

	auto j = new Judoka("first", "last");
	auto in = new Association("International", nullptr);
	auto club = new Club("club", in);

	j->SetClub(club);

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament tour("deleteMe");
	tour.AddParticipant(j);

	MD5 file(tour);

	ASSERT_TRUE(file);

	file.Dump();

	ASSERT_TRUE(file.GetOrganizer());
	auto organizer = file.GetOrganizer();
	EXPECT_EQ(organizer->Description, "International");
	EXPECT_EQ(organizer->Tier, 2);
}



TEST(MD5, ExportCompletedTournament)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/demo-file.yml");
	Tournament tour("demo-file");

	auto inter = new Judoboard::Association("International", nullptr);

	auto de = new Judoboard::Association("Deutschland", inter);

	auto dn = new Judoboard::Association("Deutschland-Nord", de);
	auto ds = new Judoboard::Association(u8"Deutschland-S\u00fcd", de);

	auto nord  = new Judoboard::Association("Nord", dn);
	auto west  = new Judoboard::Association("West", dn);
	auto nost  = new Judoboard::Association("Nordost", dn);
	auto sued  = new Judoboard::Association(u8"S\u00fcd", ds);
	auto swest = new Judoboard::Association(u8"S\u00fcdwest", ds);

	auto nieder  = new Judoboard::Association("Niedersachsen", nord);
	auto hamburg = new Judoboard::Association("Hamburg", nord);
	auto berlin  = new Judoboard::Association("Berlin", nost);
	auto nrw     = new Judoboard::Association("Nordrhein-Westfalen", west);

	auto detmold = new Judoboard::Association("Detmold", nrw);

	auto biegue = new Judoboard::Association(u8"Bielefeld/G\u00fctersloh", detmold);

	tour.SetOrganizer(biegue);

	std::vector<Club*> clubs;
	clubs.push_back(new Judoboard::Club("Altenhagen", biegue));
	clubs.push_back(new Judoboard::Club("Brackwede", biegue));
	clubs.push_back(new Judoboard::Club("Senne", biegue));

	auto age_group1 = new AgeGroup("U11",  8,  10, nullptr);
	auto age_group2 = new AgeGroup("U15", 12,  14, nullptr);
	tour.AddAgeGroup(age_group1);
	tour.AddAgeGroup(age_group2);

	for (int i = 0; i < 200; ++i)
	{
		const std::string firstname_male[] =
		{ "Ben", "Friedrich", "Phillipp", "Tim", "Lukas", "Marco", "Peter", "Martin", "Detlef", "Andreas", "Dominik", "Mathias", "Stephan", u8"S\u00f6ren", "Eric", "Finn", "Felix", "Julian", "Maximilian", "Jannik" };
		const std::string firstname_female[] =
		{ "Emma", "Stephanie", "Julia", "Jana", "Uta", "Petra", "Sophie", "Kerstin", "Lena", "Jennifer", "Kathrin", "Katherina", "Anna", "Carla", "Paulina", "Clara", "Hanna" };
		const std::string lastname[] =
		{ "Ehrlichmann", "Dresdner", "Biermann", "Fisher", "Vogler", "Pfaff", "Eberhart", "Frankfurter", u8"K\u00f6nig", "Pabst", "Ziegler", "Hartmann", "Pabst", "Kortig", "Schweitzer", "Luft", "Wexler", "Kaufmann", u8"Fr\u00fchauf", "Bieber", "Schumacher", u8"M\u00fcncher", "Schmidt", "Meier", "Fischer", "Weber", "Meyer", "Wagner", "Becker", "Schulz", "Hoffmann" };

		Judoboard::Judoka* j = nullptr;

		if (rand() & 1)
		{
			auto fname = firstname_male[rand() % (sizeof(firstname_male) / sizeof(firstname_male[0]) - 1)];
			auto lname = lastname[rand() % (sizeof(lastname) / sizeof(std::string) - 1)];
			j = new Judoka(fname, lname, 25 + rand() % 60, Judoboard::Gender::Male);
		}
		else
		{
			auto fname = firstname_female[rand() % (sizeof(firstname_female) / sizeof(firstname_female[0]) - 1)];
			auto lname = lastname[rand() % (sizeof(lastname) / sizeof(std::string) - 1)];
			j = new Judoka(fname, lname, 25 + rand() % 60, Judoboard::Gender::Female);
		}

		j->SetBirthyear(2005 + rand() % 15);
		if (!age_group1->IsElgiable(*j, tour.GetDatabase()) && !age_group2->IsElgiable(*j, tour.GetDatabase()))
		{
			delete j;
			continue;
		}

		int club_index = rand() % clubs.size();
		j->SetClub(clubs[club_index]);

		tour.AddParticipant(j);
	}

	auto age_groups  = tour.GetAgeGroups();
	auto descriptors = tour.GenerateWeightclasses(3, 5, 20, age_groups, true);

	EXPECT_TRUE(tour.ApplyWeightclasses(descriptors));

	tour.PerformLottery();

	Mat mat(1);

	while (auto match = tour.GetNextMatch(-1))
	{
		EXPECT_TRUE(mat.StartMatch(match));

		if (rand() & 1)
			mat.AddIppon(Fighter::White);
		else
			mat.AddIppon(Fighter::Blue);

		EXPECT_TRUE(mat.EndMatch());
	}


	MD5 file(tour);

	ASSERT_TRUE(file);

	ASSERT_EQ(file.GetLottery().size(), 3);

	for (int i = 0; i < file.GetLottery().size(); ++i)
	{
		auto assoc = file.FindAssociation(file.GetLottery()[i].AssociationID);

		if (assoc)
		{
			auto assoc_native = tour.GetDatabase().FindAssociationByName(assoc->Description);
			ASSERT_TRUE(assoc_native);

			auto native_lot = tour.GetLotOfAssociation(*assoc_native);
			EXPECT_EQ(native_lot, file.GetLottery()[i].StartNo);
		}

		else
		{
			auto club = file.FindClub(file.GetLottery()[i].AssociationID);

			ASSERT_TRUE(club);
			auto club_native = tour.GetDatabase().FindClubByName(club->Name);
			ASSERT_TRUE(club_native);

			auto native_lot = tour.GetLotOfAssociation(*club_native);
			EXPECT_EQ(native_lot, file.GetLottery()[i].StartNo);
		}
	}

	file.Dump();

	tour.Save();
	file.Save("demo-file.md5");

	//MD5 file_cmp("demo-file-fixed.md5");

	ASSERT_TRUE(file.GetOrganizer());
	auto organizer = file.GetOrganizer();
	EXPECT_EQ(organizer->Description, u8"Bielefeld/G\u00fctersloh");
	//EXPECT_EQ(organizer->Tier, 2);

	for (auto judoka : file.GetParticipants())
	{
		EXPECT_TRUE(judoka->HasBeenWeighted);
	}

	for (auto table : file.GetWeightclasses())
	{
		EXPECT_EQ(table->Status, 4);
	}

	for (const auto& match : file.GetMatches())
	{
		EXPECT_LE(match.MatchNo, 30);
	}

	for (const auto& result : file.GetResults())
	{
		EXPECT_EQ(result.RankType, 1);
	}
}



TEST(MD5, ReadRoundRobin)
{
	initialize();

	MD5 file("test-data/roundrobin.md5");
	//MD5 file2("test-data/roundrobin-1match.md5");
	//MD5 file3("test-data/roundrobin-no-lottery.md5");

	ASSERT_TRUE(file);

	file.Dump();

	Tournament tour(file);

	ASSERT_EQ(tour.GetMatchTables().size(), 12);
	EXPECT_EQ(tour.GetMatchTables()[0]->GetSchedule().size(), 6);

	MD5 exprt(tour);

	//exprt.Save("export.md5");

	ASSERT_TRUE(file.GetOrganizer());

	for (int i = 0; i < exprt.GetWeightclasses().size(); ++i)
	{
		EXPECT_EQ(exprt.GetWeightclasses()[i]->Status,
			       file.GetWeightclasses()[i]->Status);
	}


	for (int i = 0; i < exprt.GetMatches().size(); ++i)
	{
		EXPECT_EQ(exprt.GetMatches()[i].Result,
				   file.GetMatches()[i].Result);
		EXPECT_EQ(exprt.GetMatches()[i].Pool,
				   file.GetMatches()[i].Pool);
	}
}



TEST(MD5, ReadPool)
{
	initialize();

	MD5 file("test-data/pool.md5");

	ASSERT_TRUE(file);

	file.Dump();

	ASSERT_TRUE(file.GetOrganizer());

	Tournament tourney(file);

	EXPECT_EQ(tourney.GetSchedule().size(), file.GetMatches().size());
}



TEST(MD5, ReadPool2)
{
	initialize();

	MD5 file("test-data/BEM_U13_2023_with_matches.md7");

	ASSERT_TRUE(file);

	file.Dump();

	Tournament tourney(file);

	//EXPECT_EQ(tourney.GetSchedule().size(), file.GetMatches().size());

	auto table = tourney.FindMatchTableByDescription(u8"weibliche Jugend U13 -44 kg");
	auto md5_table = file.FindMatchesOfWeightclass(u8"weibliche Jugend U13", "-44 kg");

	EXPECT_EQ(table->GetSchedule().size(), md5_table.size() - 4);//4 dummy matches


	table = tourney.FindMatchTableByDescription(u8"m\u00e4nnliche Jugend U13 -34 kg");
	md5_table = file.FindMatchesOfWeightclass(u8"m\u00e4nnliche Jugend U13", "-34 kg");

	EXPECT_EQ(table->GetSchedule().size(), md5_table.size());


	table = tourney.FindMatchTableByDescription(u8"m\u00e4nnliche Jugend U13 -37 kg");
	md5_table = file.FindMatchesOfWeightclass(u8"m\u00e4nnliche Jugend U13", "-37 kg");

	EXPECT_EQ(table->GetSchedule().size(), md5_table.size());
}



TEST(MD5, ReadPool3)
{
	initialize();

	MD5 file("test-data/BEM_U13_2023_complete.md7");

	ASSERT_TRUE(file);

	file.Dump();

	ASSERT_TRUE(file.GetOrganizer());

	Tournament tourney(file);

	//EXPECT_EQ(tourney.GetSchedule().size(), file.GetMatches().size());

	//Compare results of pools

	auto table = tourney.FindMatchTableByDescription(u8"weibliche Jugend U13 -44 kg");
	auto results = table->CalculateResults();
	ASSERT_GE(results.GetSize(), 5);
	for (int i = 0; i < 5; i++)
		EXPECT_EQ(file.FindResult(u8"weibliche Jugend U13", "-44 kg", i+1)->Participant->Firstname,
				  results[i].Judoka->GetFirstname());

	table = tourney.FindMatchTableByDescription(u8"m\u00e4nnliche Jugend U13 -34 kg");
	results = table->CalculateResults();
	ASSERT_GE(results.GetSize(), 5);
	for (int i = 0; i < 5; i++)
		EXPECT_EQ(file.FindResult(u8"m\u00e4nnliche Jugend U13", "-34 kg", i+1)->Participant->Firstname,
			results[i].Judoka->GetFirstname());

	table = tourney.FindMatchTableByDescription(u8"m\u00e4nnliche Jugend U13 -37 kg");
	results = table->CalculateResults();
	ASSERT_GE(results.GetSize(), 5);
	for (int i = 0; i < 5; i++)
		EXPECT_EQ(file.FindResult(u8"m\u00e4nnliche Jugend U13", "-37 kg", i+1)->Participant->Firstname,
			results[i].Judoka->GetFirstname());
}



TEST(MD5, ReadDoubleElimination)
{
	initialize();

	MD5 file("test-data/doubleelimination.md5");

	ASSERT_TRUE(file);

	file.Dump();

	ASSERT_TRUE(file.GetOrganizer());
}



TEST(MD5, ReadSingleElimination)
{
	initialize();

	MD5 file("test-data/single-elimination(single-consulation-bracket).md5");

	ASSERT_TRUE(file);

	file.Dump();

	auto table = file.FindWeightclass("Jugend u10 m", "-10 kg");
	ASSERT_TRUE(table);
	EXPECT_EQ(table->FightSystemID, 19);
	EXPECT_EQ(table->MatchForThirdPlace, true);
	EXPECT_EQ(table->MatchForFifthPlace, true);

	auto matches = file.GetMatches();
	ASSERT_EQ(matches.size(), 21);

	EXPECT_EQ(matches[0].Status, 2);
	EXPECT_EQ(matches[0].Red->Firstname, "v1");
}



TEST(MD5, ImportSingleElimination)
{
	initialize();

	MD5 file("test-data/single-elimination(single-consulation-bracket).md5");

	ASSERT_TRUE(file);

	file.Dump();

	Database db;
	Tournament tour(file, &db);

	auto j1 = tour.GetDatabase().FindJudokaByName("v1 n1");
	auto j2 = tour.GetDatabase().FindJudokaByName("v2 n2");
	auto j3 = tour.GetDatabase().FindJudokaByName("v3 n3");
	auto j4 = tour.GetDatabase().FindJudokaByName("v4 n4");
	auto j5 = tour.GetDatabase().FindJudokaByName("v5 n5");
	auto j6 = tour.GetDatabase().FindJudokaByName("v6 n6");

	ASSERT_TRUE(j1);
	ASSERT_TRUE(j2);
	ASSERT_TRUE(j3);
	ASSERT_TRUE(j4);
	ASSERT_TRUE(j5);
	ASSERT_TRUE(j6);

	auto table = tour.FindMatchTableByDescription("Jugend u10 m -10 kg");
	ASSERT_TRUE(table);
	ASSERT_EQ(table->GetParticipants().size(), 6);

	EXPECT_EQ(table->GetType(), MatchTable::Type::SingleElimination);

	EXPECT_EQ(table->GetStartPosition(j1)+1, 1);
	EXPECT_EQ(table->GetStartPosition(j2)+1, 2);
	EXPECT_EQ(table->GetStartPosition(j3)+1, 3);
	EXPECT_EQ(table->GetStartPosition(j4)+1, 4);
	EXPECT_EQ(table->GetStartPosition(j5)+1, 5);
	EXPECT_EQ(table->GetStartPosition(j6)+1, 6);

	ASSERT_TRUE(table->GetMatch(11));

	EXPECT_TRUE(table->GetMatch(0)->Contains(*j1));
	EXPECT_TRUE(table->GetMatch(1)->Contains(*j5));
	EXPECT_TRUE(table->GetMatch(2)->Contains(*j3));

	EXPECT_TRUE(table->GetMatch(4)->Contains(*j2));
	EXPECT_TRUE(table->GetMatch(5)->Contains(*j6));
	EXPECT_TRUE(table->GetMatch(6)->Contains(*j4));

	EXPECT_TRUE(table->GetMatch(8)->Contains(*j1));
	EXPECT_TRUE(table->GetMatch(8)->Contains(*j5));
	EXPECT_TRUE(table->GetMatch(9)->Contains(*j3));
	EXPECT_TRUE(table->GetMatch(10)->Contains(*j2));
	EXPECT_TRUE(table->GetMatch(10)->Contains(*j6));
	EXPECT_TRUE(table->GetMatch(11)->Contains(*j4));
}



TEST(MD5, ExportSingleElimination16)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament* t = new Tournament("deleteMe");
	t->EnableAutoSave(false);

	auto inter = new Judoboard::Association("International", nullptr);

	auto de = new Judoboard::Association("Deutschland", inter);

	auto dn = new Judoboard::Association("Deutschland-Nord", de);
	auto ds = new Judoboard::Association(u8"Deutschland-S\u00fcd", de);

	auto nord  = new Judoboard::Association("Nord", dn);
	auto west  = new Judoboard::Association("West", dn);
	auto nost  = new Judoboard::Association("Nordost", dn);
	auto sued  = new Judoboard::Association(u8"S\u00fcd", ds);
	auto swest = new Judoboard::Association(u8"S\u00fcdwest", ds);

	auto nieder  = new Judoboard::Association("Niedersachsen", nord);
	auto hamburg = new Judoboard::Association("Hamburg", nord);
	auto berlin  = new Judoboard::Association("Berlin", nost);
	auto nrw     = new Judoboard::Association("Nordrhein-Westfalen", west);

	auto detmold = new Judoboard::Association("Detmold", nrw);

	auto biegue = new Judoboard::Association(u8"Bielefeld/G\u00fctersloh", detmold);

	t->SetOrganizer(biegue);

	std::vector<Club*> clubs;
	clubs.push_back(new Judoboard::Club("Altenhagen", biegue));
	clubs.push_back(new Judoboard::Club("Brackwede", biegue));
	clubs.push_back(new Judoboard::Club("Senne", biegue));


	AgeGroup* age = new AgeGroup("Youth", 1, 99, nullptr);
	t->AddAgeGroup(age);

	Judoka* j[17];

	for (int i = 1; i <= 16; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i, Judoboard::Gender::Male);
		j[i]->SetBirthyear(2000);
		j[i]->SetClub(clubs[rand()%3]);
		t->AddParticipant(j[i]);
	}

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	group->SetAgeGroup(age);
	t->AddMatchTable(group);

	for (int i = 1; i <= 16; ++i)
		group->SetStartPosition(j[i], i-1);

	t->GenerateSchedule();

	ASSERT_EQ(t->GetSchedule().size(), 15);
	ASSERT_EQ(group->GetParticipants().size(), 16);

	Mat m(1);

	for (auto match : group->GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[16]->GetUUID());


	MD5 file(*t);

	ASSERT_EQ(file.GetWeightclasses().size(), 1);
	auto& table = file.GetWeightclasses()[0];

	const auto& results2 = file.FindResults(table->AgeGroupID, table->ID);
	ASSERT_EQ(results2.size(), 2);

	EXPECT_EQ(results2[0]->RankNo, 1);
	EXPECT_EQ(results2[0]->Participant->Firstname, j[16]->GetFirstname());

	ASSERT_EQ(file.GetMatches().size(), 15);

	EXPECT_EQ(file.GetMatches()[0].MatchNo, 1);
	EXPECT_EQ(file.GetMatches()[1].MatchNo, 2);
	EXPECT_EQ(file.GetMatches()[12].MatchNo, 13);
	EXPECT_EQ(file.GetMatches()[13].MatchNo, 14);
	EXPECT_EQ(file.GetMatches()[14].MatchNo, 19);

	//MD5 file2("test-data/single-elimination(single-consulation-bracket).md5");
	file.Save("output.md5");

	delete t;
}



TEST(MD5, ExportSingleElimination16_3rd_5th)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament* t = new Tournament("deleteMe");
	t->EnableAutoSave(false);

	auto inter = new Judoboard::Association("International", nullptr);

	auto de = new Judoboard::Association("Deutschland", inter);

	auto dn = new Judoboard::Association("Deutschland-Nord", de);
	auto ds = new Judoboard::Association(u8"Deutschland-S\u00fcd", de);

	auto nord  = new Judoboard::Association("Nord", dn);
	auto west  = new Judoboard::Association("West", dn);
	auto nost  = new Judoboard::Association("Nordost", dn);
	auto sued  = new Judoboard::Association(u8"S\u00fcd", ds);
	auto swest = new Judoboard::Association(u8"S\u00fcdwest", ds);

	auto nieder  = new Judoboard::Association("Niedersachsen", nord);
	auto hamburg = new Judoboard::Association("Hamburg", nord);
	auto berlin  = new Judoboard::Association("Berlin", nost);
	auto nrw     = new Judoboard::Association("Nordrhein-Westfalen", west);

	auto detmold = new Judoboard::Association("Detmold", nrw);

	auto biegue = new Judoboard::Association(u8"Bielefeld/G\u00fctersloh", detmold);

	t->SetOrganizer(biegue);

	std::vector<Club*> clubs;
	clubs.push_back(new Judoboard::Club("Altenhagen", biegue));
	clubs.push_back(new Judoboard::Club("Brackwede", biegue));
	clubs.push_back(new Judoboard::Club("Senne", biegue));


	AgeGroup* age = new AgeGroup("Youth", 1, 99, nullptr);
	t->AddAgeGroup(age);

	Judoka* j[17];

	for (int i = 1; i <= 16; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		j[i]->SetBirthyear(2000);
		j[i]->SetClub(clubs[rand()%3]);
		t->AddParticipant(j[i]);
	}

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	group->SetAgeGroup(age);
	group->IsThirdPlaceMatch(true);
	group->IsFifthPlaceMatch(true);
	t->AddMatchTable(group);

	for (int i = 1; i <= 16; ++i)
		group->SetStartPosition(j[i], i-1);

	t->GenerateSchedule();

	ASSERT_EQ(group->GetParticipants().size(), 16);

	Mat m(1);

	for (auto match : group->GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 6);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[16]->GetUUID());


	MD5 file(*t);

	ASSERT_EQ(file.GetWeightclasses().size(), 1);
	auto& table = file.GetWeightclasses()[0];

	const auto& results2 = file.FindResults(table->AgeGroupID, table->ID);
	ASSERT_EQ(results2.size(), 6);

	EXPECT_EQ(results2[0]->RankNo, 1);
	EXPECT_EQ(results2[0]->Participant->Firstname, j[16]->GetFirstname());

	ASSERT_EQ(file.GetMatches().size(), 15 + 4);

	EXPECT_EQ(file.GetMatches()[0].MatchNo, 1);
	EXPECT_EQ(file.GetMatches()[1].MatchNo, 2);
	EXPECT_EQ(file.GetMatches()[14 + 2].MatchNo, 21);
	EXPECT_EQ(file.GetMatches()[14 + 3].MatchNo, 20);
	EXPECT_EQ(file.GetMatches()[14 + 4].MatchNo, 19);

	//MD5 file2("test-data/single-elimination(single-consulation-bracket).md5");
	file.Save("output.md5");

	delete t;
}



TEST(MD5, ExportSingleElimination32)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament* t = new Tournament("deleteMe");
	t->EnableAutoSave(false);

	auto inter = new Judoboard::Association("International", nullptr);

	auto de = new Judoboard::Association("Deutschland", inter);

	auto dn = new Judoboard::Association("Deutschland-Nord", de);
	auto ds = new Judoboard::Association(u8"Deutschland-S\u00fcd", de);

	auto nord  = new Judoboard::Association("Nord", dn);
	auto west  = new Judoboard::Association("West", dn);
	auto nost  = new Judoboard::Association("Nordost", dn);
	auto sued  = new Judoboard::Association(u8"S\u00fcd", ds);
	auto swest = new Judoboard::Association(u8"S\u00fcdwest", ds);

	auto nieder  = new Judoboard::Association("Niedersachsen", nord);
	auto hamburg = new Judoboard::Association("Hamburg", nord);
	auto berlin  = new Judoboard::Association("Berlin", nost);
	auto nrw     = new Judoboard::Association("Nordrhein-Westfalen", west);

	auto detmold = new Judoboard::Association("Detmold", nrw);

	auto biegue = new Judoboard::Association(u8"Bielefeld/G\u00fctersloh", detmold);

	t->SetOrganizer(biegue);

	std::vector<Club*> clubs;
	clubs.push_back(new Judoboard::Club("Altenhagen", biegue));
	clubs.push_back(new Judoboard::Club("Brackwede", biegue));
	clubs.push_back(new Judoboard::Club("Senne", biegue));


	AgeGroup* age = new AgeGroup("Youth", 1, 99, nullptr);
	t->AddAgeGroup(age);

	Judoka* j[33];

	for (int i = 1; i <= 32; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		j[i]->SetBirthyear(2000);
		j[i]->SetClub(clubs[rand()%3]);
		t->AddParticipant(j[i]);
	}

	SingleElimination* group = new SingleElimination(0, 200);
	group->SetMatID(1);
	group->SetAgeGroup(age);
	t->AddMatchTable(group);

	for (int i = 1; i <= 32; ++i)
		group->SetStartPosition(j[i], i-1);

	t->GenerateSchedule();

	ASSERT_EQ(group->GetParticipants().size(), 32);

	Mat m(1);

	for (auto match : group->GetSchedule())
	{
		if (!match->HasValidFighters())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[32]->GetUUID());


	MD5 file(*t);

	ASSERT_EQ(file.GetWeightclasses().size(), 1);
	auto& table = file.GetWeightclasses()[0];

	const auto& results2 = file.FindResults(table->AgeGroupID, table->ID);
	ASSERT_EQ(results2.size(), 2);

	EXPECT_EQ(results2[0]->RankNo, 1);
	EXPECT_EQ(results2[0]->Participant->Firstname, j[32]->GetFirstname());

	ASSERT_EQ(file.GetMatches().size(), 31);

	EXPECT_EQ(file.GetMatches()[0].MatchNo, 1);
	EXPECT_EQ(file.GetMatches()[1].MatchNo, 2);
	EXPECT_EQ(file.GetMatches()[12].MatchNo, 13);
	EXPECT_EQ(file.GetMatches()[13].MatchNo, 14);
	EXPECT_EQ(file.GetMatches()[30].MatchNo, 37);

	//MD5 file2("test-data/single-elimination(single-consulation-bracket).md5");
	file.Save("output.md5");

	delete t;
}



TEST(MD5, ReadMD7TestData)
{
	initialize();

	MD5 file("test-data/Schlosspokal2023_U11m.md7");

	ASSERT_TRUE(file);

	file.Dump();

	Tournament t(file);
}