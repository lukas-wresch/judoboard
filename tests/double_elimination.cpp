#include "tests.h"



TEST(LoserBracket, Count16)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 14;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	LoserBracket* group = new LoserBracket(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(), count);

	ASSERT_EQ(group->GetSchedule().size(), 4+4+2+2);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[4]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[8]));

	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[10]));

	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[14]));

	ASSERT_EQ(group->GetParticipants().size(), 14);

	delete t;
}



TEST(LoserBracket, Count16_Final)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 14;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	LoserBracket* group = new LoserBracket(0, 200);
	group->SetMatID(1);
	group->IsFinalMatch(true);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(), count);

	ASSERT_EQ(group->GetSchedule().size(), 4+4+2+2+1);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[4]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[8]));

	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[10]));

	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[14]));

	ASSERT_EQ(group->GetParticipants().size(), 14);

	delete t;
}



TEST(LoserBracket, Count16_Final_3rd)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 14;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	LoserBracket* group = new LoserBracket(0, 200);
	group->SetMatID(1);
	group->IsFinalMatch(true);
	group->IsThirdPlaceMatch(true);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(), count);
	ASSERT_EQ(group->GetParticipants().size(), count);
	ASSERT_EQ(group->GetSchedule().size(), 4+4+2+2+1+1);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[4]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[8]));

	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[10]));

	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[14]));

	delete t;
}



TEST(LoserBracket, Count32)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[65];
	bool has_match[65];
	const size_t count = 30;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	LoserBracket* group = new LoserBracket(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(),   count);
	ASSERT_EQ(group->GetParticipants().size(), count);
	ASSERT_EQ(group->GetSchedule().size(), 8 + 8 + 4 + 4 + 2 + 2);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[14]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[15]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[16]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[10]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[8]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[4]));

	EXPECT_TRUE(group->GetMatch(8)->Contains(*j[21]));
	EXPECT_TRUE(group->GetMatch(9)->Contains(*j[22]));
	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[23]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[24]));
	EXPECT_TRUE(group->GetMatch(12)->Contains(*j[17]));
	EXPECT_TRUE(group->GetMatch(13)->Contains(*j[18]));
	EXPECT_TRUE(group->GetMatch(14)->Contains(*j[19]));
	EXPECT_TRUE(group->GetMatch(15)->Contains(*j[20]));

	EXPECT_TRUE(group->GetMatch(20)->Contains(*j[25]));
	EXPECT_TRUE(group->GetMatch(21)->Contains(*j[26]));
	EXPECT_TRUE(group->GetMatch(22)->Contains(*j[27]));
	EXPECT_TRUE(group->GetMatch(23)->Contains(*j[28]));
	EXPECT_TRUE(group->GetMatch(26)->Contains(*j[30]));
	EXPECT_TRUE(group->GetMatch(27)->Contains(*j[29]));

	delete t;
}



TEST(LoserBracket, Count32_Final)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[65];
	bool has_match[65];
	const size_t count = 30;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	LoserBracket* group = new LoserBracket(0, 200);
	group->SetMatID(1);
	group->IsFinalMatch(true);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(),   count);
	ASSERT_EQ(group->GetParticipants().size(), count);
	ASSERT_EQ(group->GetSchedule().size(), 8 + 8 + 4 + 4 + 2 + 2 + 1);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[14]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[15]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[16]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[10]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[8]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[4]));

	EXPECT_TRUE(group->GetMatch(8)->Contains(*j[21]));
	EXPECT_TRUE(group->GetMatch(9)->Contains(*j[22]));
	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[23]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[24]));
	EXPECT_TRUE(group->GetMatch(12)->Contains(*j[17]));
	EXPECT_TRUE(group->GetMatch(13)->Contains(*j[18]));
	EXPECT_TRUE(group->GetMatch(14)->Contains(*j[19]));
	EXPECT_TRUE(group->GetMatch(15)->Contains(*j[20]));

	EXPECT_TRUE(group->GetMatch(20)->Contains(*j[25]));
	EXPECT_TRUE(group->GetMatch(21)->Contains(*j[26]));
	EXPECT_TRUE(group->GetMatch(22)->Contains(*j[27]));
	EXPECT_TRUE(group->GetMatch(23)->Contains(*j[28]));
	EXPECT_TRUE(group->GetMatch(26)->Contains(*j[30]));
	EXPECT_TRUE(group->GetMatch(27)->Contains(*j[29]));

	delete t;
}



TEST(LoserBracket, Count32_Final_3rd)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[65];
	bool has_match[65];
	const size_t count = 30;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	LoserBracket* group = new LoserBracket(0, 200);
	group->SetMatID(1);
	group->IsFinalMatch(true);
	group->IsThirdPlaceMatch(true);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(),   count);
	ASSERT_EQ(group->GetParticipants().size(), count);
	ASSERT_EQ(group->GetSchedule().size(), 8 + 8 + 4 + 4 + 2 + 2 + 1 + 1);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[14]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[15]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[16]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[10]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[8]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[4]));

	EXPECT_TRUE(group->GetMatch(8)->Contains(*j[21]));
	EXPECT_TRUE(group->GetMatch(9)->Contains(*j[22]));
	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[23]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[24]));
	EXPECT_TRUE(group->GetMatch(12)->Contains(*j[17]));
	EXPECT_TRUE(group->GetMatch(13)->Contains(*j[18]));
	EXPECT_TRUE(group->GetMatch(14)->Contains(*j[19]));
	EXPECT_TRUE(group->GetMatch(15)->Contains(*j[20]));

	EXPECT_TRUE(group->GetMatch(20)->Contains(*j[25]));
	EXPECT_TRUE(group->GetMatch(21)->Contains(*j[26]));
	EXPECT_TRUE(group->GetMatch(22)->Contains(*j[27]));
	EXPECT_TRUE(group->GetMatch(23)->Contains(*j[28]));
	EXPECT_TRUE(group->GetMatch(26)->Contains(*j[30]));
	EXPECT_TRUE(group->GetMatch(27)->Contains(*j[29]));

	delete t;
}



TEST(LoserBracket, Count64)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[65];
	bool has_match[65];
	const size_t count = 64-2;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	LoserBracket* group = new LoserBracket(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(),   count);
	ASSERT_EQ(group->GetParticipants().size(), count);
	ASSERT_EQ(group->GetSchedule().size(), 16 + 16 + 8 + 8 + 4 + 4 + 2 + 2);

	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[29]));
	EXPECT_TRUE(group->GetMatch(0)->Contains(*j[30]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[31]));
	EXPECT_TRUE(group->GetMatch(1)->Contains(*j[32]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[25]));
	EXPECT_TRUE(group->GetMatch(2)->Contains(*j[26]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[27]));
	EXPECT_TRUE(group->GetMatch(3)->Contains(*j[28]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[21]));
	EXPECT_TRUE(group->GetMatch(4)->Contains(*j[22]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[23]));
	EXPECT_TRUE(group->GetMatch(5)->Contains(*j[24]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[17]));
	EXPECT_TRUE(group->GetMatch(6)->Contains(*j[18]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[19]));
	EXPECT_TRUE(group->GetMatch(7)->Contains(*j[20]));
	EXPECT_TRUE(group->GetMatch(8)->Contains(*j[13]));
	EXPECT_TRUE(group->GetMatch(8)->Contains(*j[14]));
	EXPECT_TRUE(group->GetMatch(9)->Contains(*j[15]));
	EXPECT_TRUE(group->GetMatch(9)->Contains(*j[16]));
	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[9]));
	EXPECT_TRUE(group->GetMatch(10)->Contains(*j[10]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[11]));
	EXPECT_TRUE(group->GetMatch(11)->Contains(*j[12]));
	EXPECT_TRUE(group->GetMatch(12)->Contains(*j[5]));
	EXPECT_TRUE(group->GetMatch(12)->Contains(*j[6]));
	EXPECT_TRUE(group->GetMatch(13)->Contains(*j[7]));
	EXPECT_TRUE(group->GetMatch(13)->Contains(*j[8]));
	EXPECT_TRUE(group->GetMatch(14)->Contains(*j[1]));
	EXPECT_TRUE(group->GetMatch(14)->Contains(*j[2]));
	EXPECT_TRUE(group->GetMatch(15)->Contains(*j[3]));
	EXPECT_TRUE(group->GetMatch(15)->Contains(*j[4]));

	EXPECT_TRUE(group->GetMatch(16)->Contains(*j[37]));
	EXPECT_TRUE(group->GetMatch(17)->Contains(*j[38]));
	EXPECT_TRUE(group->GetMatch(18)->Contains(*j[39]));
	EXPECT_TRUE(group->GetMatch(19)->Contains(*j[40]));
	EXPECT_TRUE(group->GetMatch(20)->Contains(*j[33]));
	EXPECT_TRUE(group->GetMatch(21)->Contains(*j[34]));
	EXPECT_TRUE(group->GetMatch(22)->Contains(*j[35]));
	EXPECT_TRUE(group->GetMatch(23)->Contains(*j[36]));
	EXPECT_TRUE(group->GetMatch(24)->Contains(*j[45]));
	EXPECT_TRUE(group->GetMatch(25)->Contains(*j[46]));
	EXPECT_TRUE(group->GetMatch(26)->Contains(*j[47]));
	EXPECT_TRUE(group->GetMatch(27)->Contains(*j[48]));
	EXPECT_TRUE(group->GetMatch(28)->Contains(*j[41]));
	EXPECT_TRUE(group->GetMatch(29)->Contains(*j[42]));
	EXPECT_TRUE(group->GetMatch(30)->Contains(*j[43]));
	EXPECT_TRUE(group->GetMatch(31)->Contains(*j[44]));

	EXPECT_TRUE(group->GetMatch(40)->Contains(*j[69-16]));
	EXPECT_TRUE(group->GetMatch(41)->Contains(*j[70-16]));
	EXPECT_TRUE(group->GetMatch(42)->Contains(*j[71-16]));
	EXPECT_TRUE(group->GetMatch(43)->Contains(*j[72-16]));
	EXPECT_TRUE(group->GetMatch(44)->Contains(*j[65-16]));
	EXPECT_TRUE(group->GetMatch(45)->Contains(*j[66-16]));
	EXPECT_TRUE(group->GetMatch(46)->Contains(*j[67-16]));
	EXPECT_TRUE(group->GetMatch(47)->Contains(*j[68-16]));

	EXPECT_TRUE(group->GetMatch(52)->Contains(*j[89-16-16]));
	EXPECT_TRUE(group->GetMatch(53)->Contains(*j[90-16-16]));
	EXPECT_TRUE(group->GetMatch(54)->Contains(*j[91-16-16]));
	EXPECT_TRUE(group->GetMatch(55)->Contains(*j[92-16-16]));

	EXPECT_TRUE(group->GetMatch(58)->Contains(*j[110-16-16-16]));
	EXPECT_TRUE(group->GetMatch(59)->Contains(*j[109-16-16-16]));

	delete t;
}



TEST(DoubleElimination, Count8)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 8;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(), count);

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	//EXPECT_TRUE(loser_schedule[0]->Contains(*j[1]));

	ASSERT_EQ(winner_schedule.size(),  4 + 2 + 1);
	ASSERT_EQ(loser_schedule.size(),   2 + 2);

	delete t;
}



TEST(DoubleElimination, Count16)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 16;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(), count);

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	//EXPECT_TRUE(loser_schedule[0]->Contains(*j[1]));

	ASSERT_EQ(winner_schedule.size(), 8 + 4 + 2 + 1);
	ASSERT_EQ(loser_schedule.size(),  4 + 4 + 2 + 2);

	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[0]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[1]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[2]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[3]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[4]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[5]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[6]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[7]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[8]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[9]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[10]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[11]));

	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[12]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[13]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[14]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[15]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[16]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[17]));

	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[18]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[19]));

	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[20]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[21]));

	delete t;
}



TEST(DoubleElimination, Count32)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 32;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 0; i < count; ++i)
		group->SetStartPosition(j[i+1], i);

	EXPECT_EQ(group->GetMaxStartPositions(), count);

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	//EXPECT_TRUE(loser_schedule[0]->Contains(*j[1]));

	ASSERT_EQ(winner_schedule.size(), 16 + 8 + 4 + 2 + 1);
	ASSERT_EQ(loser_schedule.size(),  8 + 8 + 4 + 4 + 2 + 2);

	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[0]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[1]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[2]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[3]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[4]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[5]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[6]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[7]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[8]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[9]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[10]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[11]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[12]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[13]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[14]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[15]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[16]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[17]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[18]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[19]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[20]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[21]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[22]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[23]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[24]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[25]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[26]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[27]));

	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[28]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[29]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[30]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[31]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[32]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[33]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[34]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[35]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[36]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[37]));

	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[38]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[39]));

	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[40]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[41]));

	delete t;
}



TEST(DoubleElimination, ExportImport)
{
	initialize();

	for (size_t count = 1; count <= 64; count *= 2)
	{
		Judoka* j[65];
		bool has_match[65];

		Tournament t;
		DoubleElimination* group = new DoubleElimination(0, 200);

		for (int i = 1; i <= count; ++i)
		{
			j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
			has_match[i] = false;
			group->AddParticipant(j[i]);
		}

		group->SetMatID(1);

		for (int i = 1; i <= count; ++i)
			group->SetStartPosition(j[i], i-1);

		EXPECT_EQ(group->GetMaxStartPositions(), count);

		auto& loser_schedule = group->GetLoserBracket().GetSchedule();

		t.AddMatchTable(group);

		YAML::Emitter yaml;
		*group >> yaml;

		DoubleElimination group2(YAML::Load(yaml.c_str()), &t);

		EXPECT_EQ(group->ToHTML(), group2.ToHTML());

		for (int i = 1; i <= count; ++i)
			delete j[i];
	}
}