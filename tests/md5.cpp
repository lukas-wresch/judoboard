#include "tests.h"



TEST(MD5, ReadTestData)
{
	initialize();

#ifdef _WIN32
	MD5 file("../test-data/Test.md5");
#else
	MD5 file("test-data/Test.md5");
#endif

	ASSERT_TRUE(file);

	file.Dump();

	//EXPECT_EQ(file.GetAssociations().size(), file.GetNumAssociations());//This seems to be broken even in the test file
	EXPECT_EQ(file.GetClubs().size(),		 file.GetNumClubs());
	EXPECT_EQ(file.GetParticipants().size(), file.GetNumParticipants());

	EXPECT_EQ(file.GetClubs().size(), 21);
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

#ifdef _WIN32
	MD5 file("../test-data/Test.md5");
#else
	MD5 file("test-data/Test.md5");
#endif

	ASSERT_TRUE(file);

	Tournament tour(file);

	EXPECT_EQ(tour.GetDatabase().GetNumJudoka(), 142);
	EXPECT_EQ(tour.GetDatabase().GetNumClubs(),   21);
}



TEST(MD5, CreateTournamentFromTestData2)
{
	initialize();

#ifdef _WIN32
	MD5 file("../test-data/Test.md5");
#else
	MD5 file("test-data/Test.md5");
#endif

	ASSERT_TRUE(file);

	Database db;
	Tournament tour(file, &db);

	EXPECT_EQ(db.GetNumJudoka(), 142);
	EXPECT_EQ(db.GetNumClubs(),   21);
}



TEST(MD5, ImportIntoTournament)
{
	initialize();

#ifdef _WIN32
	MD5 file("../test-data/Test.md5");
#else
	MD5 file("test-data/Test.md5");
#endif

	ASSERT_TRUE(file);

	Database db;
	Tournament tour(file, &db);

	auto table = tour.FindMatchTableByName("Jugend u10 w -20&#44;7 kg");
	ASSERT_TRUE(table);
	auto results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Laura");


	table = tour.FindMatchTableByName("Jugend u10 w -26&#44;7 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mila");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Amalia");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Julia");

	table = tour.FindMatchTableByName("Jugend u10 w -30&#44;7 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Chiara");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lina");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Narja");

	table = tour.FindMatchTableByName("Jugend u10 m -23&#44;7 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jonas");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Alexander");

	table = tour.FindMatchTableByName("Jugend u10 m -25&#44;4 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Dominik");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Antonio");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Bjarne");

	table = tour.FindMatchTableByName("Jugend u10 m -26&#44;3 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Diego");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arthur");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lenny");

	table = tour.FindMatchTableByName("Jugend u10 m -28&#44;9 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Anton");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Janos");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kemal");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "David");

	table = tour.FindMatchTableByName("Jugend u10 m -31&#44;3 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lemaliam");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Georg");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Merdan");

	table = tour.FindMatchTableByName("Jugend u10 m -33&#44;7 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Philipp");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Timon");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Emil");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mark");

	table = tour.FindMatchTableByName("Jugend u10 m -39 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonard");

	table = tour.FindMatchTableByName("Jugend u15 w -10 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 0);

	table = tour.FindMatchTableByName("Jugend u15 w -33 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Paula");

	table = tour.FindMatchTableByName("Jugend u15 w -36 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lisa");

	table = tour.FindMatchTableByName("Jugend u15 w -44 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gloria");

	table = tour.FindMatchTableByName("Jugend u15 w -48 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mia");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Kira");

	table = tour.FindMatchTableByName("Jugend u15 w -52 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Hannah");

	table = tour.FindMatchTableByName("Jugend u15 w -57 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 0);
	
	table = tour.FindMatchTableByName("Jugend u15 w -63 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Alina");

	table = tour.FindMatchTableByName("Jugend u15 w +63 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Clara");

	table = tour.FindMatchTableByName("Jugend u15 m -34 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lukas");

	table = tour.FindMatchTableByName("Jugend u15 m -37 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Luca");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Vincent");

	table = tour.FindMatchTableByName("Jugend u15 m -40 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Cedric");

	table = tour.FindMatchTableByName("Jugend u15 m -43 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nico");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "David");

	table = tour.FindMatchTableByName("Jugend u15 m -46 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 0);
	
	table = tour.FindMatchTableByName("Jugend u15 m -50 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 0);

	table = tour.FindMatchTableByName("Jugend u15 m -55 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 5);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Sebastian");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Harry");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Marlon");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Maximilian");
	EXPECT_EQ(results[4].Judoka->GetFirstname(), "Jan");

	table = tour.FindMatchTableByName("Jugend u15 m -60 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mikail");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maurice");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Robin");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

	table = tour.FindMatchTableByName("Jugend u15 m -66 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nikita");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dimitri");

	table = tour.FindMatchTableByName("Jugend u15 m +66 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 0);

	table = tour.FindMatchTableByName("Frauen u18 -43&#44;1 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gina");

	table = tour.FindMatchTableByName("Frauen u18 -57&#44;7 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Maria-T");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Evelyne");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fia");

	table = tour.FindMatchTableByName(u8"M\u00e4nner u18 -54 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Simon");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Justin");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vakhtang");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Vincenzo");

	table = tour.FindMatchTableByName(u8"M\u00e4nner u18 -65&#44;5 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Jason");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Ben-N");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilian");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Quentin");

	table = tour.FindMatchTableByName(u8"M\u00e4nner u18 -72&#44;6 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Tobias");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Nikita");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Ewan F");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Jan");

	table = tour.FindMatchTableByName(u8"Jugend u13w -28&#44;1 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Klara");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marie");

	table = tour.FindMatchTableByName(u8"Jugend u13w -30&#44;3 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonie");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Viktoria");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Diana");

	table = tour.FindMatchTableByName(u8"Jugend u13w -33&#44;9 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Valentina");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Luisa");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Mariebelle");

	table = tour.FindMatchTableByName(u8"Jugend u13w -36&#44;1 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Vanessa");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Hadisha");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lentje");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leane");

	table = tour.FindMatchTableByName(u8"Jugend u13w -39&#44;2 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Pia");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Rieke");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vienna");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mia");

	table = tour.FindMatchTableByName(u8"Jugend u13w -42&#44;9 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nika");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Pia");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Sirpa");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Lia");

	table = tour.FindMatchTableByName(u8"Jugend u13w -52&#44;9 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 2);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Josephine");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marijana");

	table = tour.FindMatchTableByName(u8"Jugend u13m -23&#44;3 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 1);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Selim");

	table = tour.FindMatchTableByName(u8"Jugend u13m -28&#44;1 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Milo");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arne");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fabian");

	table = tour.FindMatchTableByName(u8"Jugend u13m -28&#44;7 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Arthur");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Damir");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilain");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Moritz");

	table = tour.FindMatchTableByName(u8"Jugend u13m -30&#44;4 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maxim");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kerwin");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Matthies");

	table = tour.FindMatchTableByName(u8"Jugend u13m -34&#44;4 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gregor");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dominik");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Florian");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Philip");

	table = tour.FindMatchTableByName(u8"Jugend u13m -35&#44;1 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Silas");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Laurenz");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Amon");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leonard");

	table = tour.FindMatchTableByName(u8"Jugend u13m -37&#44;9 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Ewald");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arvid");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kai");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Tim");

	table = tour.FindMatchTableByName(u8"Jugend u13m -39&#44;9 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Enrico");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Felix");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

	table = tour.FindMatchTableByName(u8"Jugend u13m -43&#44;7 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Matti");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jan");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Jonas");

	table = tour.FindMatchTableByName(u8"Jugend u13m -47&#44;7 kg");
	ASSERT_TRUE(table);
	results = table->CalculateResults();

	ASSERT_EQ(results.size(), 4);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Norwin");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Darian");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Luca");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Titus");
}



TEST(MD5, ImportIntoTournament_LoadAfterSave)
{
	initialize();

#ifdef _WIN32
	MD5 file("../test-data/Test.md5");
#else
	MD5 file("test-data/Test.md5");
#endif

	ASSERT_TRUE(file);

	{
		Database db;
		{
			Tournament tour(file, &db);
			tour.SetName("deleteMe");
			tour.Save();
		}

		Tournament tour2("deleteMe");

		auto table = tour2.FindMatchTableByName("Jugend u10 w -20&#44;7 kg");
		ASSERT_TRUE(table);
		auto results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Laura");


		table = tour2.FindMatchTableByName("Jugend u10 w -26&#44;7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mila");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Amalia");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Julia");

		table = tour2.FindMatchTableByName("Jugend u10 w -30&#44;7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Chiara");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lina");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Narja");

		table = tour2.FindMatchTableByName("Jugend u10 m -23&#44;7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jonas");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Alexander");

		table = tour2.FindMatchTableByName("Jugend u10 m -25&#44;4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Dominik");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Antonio");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Bjarne");

		table = tour2.FindMatchTableByName("Jugend u10 m -26&#44;3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Diego");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arthur");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lenny");

		table = tour2.FindMatchTableByName("Jugend u10 m -28&#44;9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Anton");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Janos");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kemal");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "David");

		table = tour2.FindMatchTableByName("Jugend u10 m -31&#44;3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Fabian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Lemaliam");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Georg");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Merdan");

		table = tour2.FindMatchTableByName("Jugend u10 m -33&#44;7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Philipp");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Timon");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Emil");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mark");

		table = tour2.FindMatchTableByName("Jugend u10 m -39 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonard");

		table = tour2.FindMatchTableByName("Jugend u15 w -10 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 0);

		table = tour2.FindMatchTableByName("Jugend u15 w -33 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Paula");

		table = tour2.FindMatchTableByName("Jugend u15 w -36 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lisa");

		table = tour2.FindMatchTableByName("Jugend u15 w -44 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gloria");

		table = tour2.FindMatchTableByName("Jugend u15 w -48 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mia");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Kira");

		table = tour2.FindMatchTableByName("Jugend u15 w -52 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Hannah");

		table = tour2.FindMatchTableByName("Jugend u15 w -57 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 0);

		table = tour2.FindMatchTableByName("Jugend u15 w -63 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Alina");

		table = tour2.FindMatchTableByName("Jugend u15 w +63 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Clara");

		table = tour2.FindMatchTableByName("Jugend u15 m -34 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Lukas");

		table = tour2.FindMatchTableByName("Jugend u15 m -37 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Luca");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Vincent");

		table = tour2.FindMatchTableByName("Jugend u15 m -40 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Cedric");

		table = tour2.FindMatchTableByName("Jugend u15 m -43 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nico");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "David");

		table = tour2.FindMatchTableByName("Jugend u15 m -46 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 0);

		table = tour2.FindMatchTableByName("Jugend u15 m -50 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 0);

		table = tour2.FindMatchTableByName("Jugend u15 m -55 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 5);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Sebastian");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Harry");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Marlon");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Maximilian");
		EXPECT_EQ(results[4].Judoka->GetFirstname(), "Jan");

		table = tour2.FindMatchTableByName("Jugend u15 m -60 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Mikail");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maurice");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Robin");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

		table = tour2.FindMatchTableByName("Jugend u15 m -66 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nikita");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dimitri");

		table = tour2.FindMatchTableByName("Jugend u15 m +66 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 0);

		table = tour2.FindMatchTableByName("Frauen u18 -43&#44;1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gina");

		table = tour2.FindMatchTableByName("Frauen u18 -57&#44;7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Maria-T");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Evelyne");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fia");

		table = tour2.FindMatchTableByName(u8"M\u00e4nner u18 -54 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Simon");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Justin");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vakhtang");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Vincenzo");

		table = tour2.FindMatchTableByName(u8"M\u00e4nner u18 -65&#44;5 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Jason");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Ben-N");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilian");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Quentin");

		table = tour2.FindMatchTableByName(u8"M\u00e4nner u18 -72&#44;6 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Tobias");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Nikita");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Ewan F");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Jan");

		table = tour2.FindMatchTableByName(u8"Jugend u13w -28&#44;1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Klara");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marie");

		table = tour2.FindMatchTableByName(u8"Jugend u13w -30&#44;3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Leonie");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Viktoria");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Diana");

		table = tour2.FindMatchTableByName(u8"Jugend u13w -33&#44;9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Valentina");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Luisa");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Mariebelle");

		table = tour2.FindMatchTableByName(u8"Jugend u13w -36&#44;1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Vanessa");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Hadisha");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Lentje");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leane");

		table = tour2.FindMatchTableByName(u8"Jugend u13w -39&#44;2 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Pia");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Rieke");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Vienna");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Mia");

		table = tour2.FindMatchTableByName(u8"Jugend u13w -42&#44;9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Nika");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Pia");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Sirpa");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Lia");

		table = tour2.FindMatchTableByName(u8"Jugend u13w -52&#44;9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 2);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Josephine");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Marijana");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -23&#44;3 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 1);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Selim");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -28&#44;1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Milo");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arne");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Fabian");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -28&#44;7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Arthur");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Damir");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Maximilain");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Moritz");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -30&#44;4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Maxim");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kerwin");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Matthies");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -34&#44;4 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Gregor");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Dominik");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Florian");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Philip");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -35&#44;1 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Silas");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Laurenz");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Amon");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Leonard");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -37&#44;9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Ewald");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Arvid");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Kai");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Tim");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -39&#44;9 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Benjamin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Enrico");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Felix");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Paul");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -43&#44;7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 3);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Matti");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Jan");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Jonas");

		table = tour2.FindMatchTableByName(u8"Jugend u13m -47&#44;7 kg");
		ASSERT_TRUE(table);
		results = table->CalculateResults();

		ASSERT_EQ(results.size(), 4);
		EXPECT_EQ(results[0].Judoka->GetFirstname(), "Norwin");
		EXPECT_EQ(results[1].Judoka->GetFirstname(), "Darian");
		EXPECT_EQ(results[2].Judoka->GetFirstname(), "Luca");
		EXPECT_EQ(results[3].Judoka->GetFirstname(), "Titus");
	}


	ZED::Core::RemoveFile("deleteMe");
}



TEST(MD5, Export)
{
	initialize();

#ifdef _WIN32
	MD5 file("../test-data/Test-cleaned.md5");
	const auto hash_expected = ZED::SHA512(ZED::File("../test-data/Test-cleaned.md5"));
#else
	MD5 file("test-data/Test-cleaned.md5");
	const auto hash_expected = ZED::SHA512(ZED::File("test-data/Test-cleaned.md5"));
#endif

	ASSERT_TRUE(file);

	file.Dump();

	EXPECT_TRUE(file.Save("deleteMe"));

	const auto hash_has = ZED::SHA512(ZED::File("deleteMe"));

	EXPECT_EQ(hash_has, hash_expected);

	ZED::Core::RemoveFile("deleteMe");
}