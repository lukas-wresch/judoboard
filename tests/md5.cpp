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

	//TODO
}