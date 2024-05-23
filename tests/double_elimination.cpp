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



TEST(DoubleElimination, Count6)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 6;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 1; i <= count; ++i)
		group->SetStartPosition(j[i], i);

	EXPECT_EQ(group->GetMaxStartPositions(), 8);

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	//EXPECT_TRUE(loser_schedule[0]->Contains(*j[1]));

	ASSERT_EQ(winner_schedule.size(),  4 + 2 + 1);
	ASSERT_EQ(loser_schedule.size(),   2 + 2);

	Mat m(1);
	int match_count = 0;

	for (auto match : group->GetSchedule())
	{
		if (match->IsEmptyMatch())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
		match_count++;
	}

	EXPECT_EQ(match_count, 7);

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 2);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[6]->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j[5]->GetUUID());

	delete t;
}



TEST(DoubleElimination, Count6_2)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 6;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 1; i <= count; ++i)
		group->SetStartPosition(j[i], i);

	EXPECT_EQ(group->GetMaxStartPositions(), 8);

	group->IsThirdPlaceMatch(true);
	group->IsFifthPlaceMatch(true);

	t->GenerateSchedule();

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	ASSERT_EQ(winner_schedule.size(),  4 + 2 + 1);
	ASSERT_EQ(loser_schedule.size(),   2 + 2 + 1 + 1);

	Mat m(1);
	int match_count = 0;

	for (auto match : t->GetSchedule())
	{
		EXPECT_FALSE(match->IsEmptyMatch());

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
		match_count++;
	}

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 6);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[6]->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j[5]->GetUUID());

	delete t;
}



TEST(DoubleElimination, Count6_Example)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament* t = new Tournament("deleteMe");

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 6;

	j[1] = new Judoka("Alexa", "", 50);
	j[2] = new Judoka("Hans", "", 50);
	j[3] = new Judoka("Johann", "", 50);
	j[4] = new Judoka("Peter", "", 50);
	j[5] = new Judoka("Wulf", "", 50);
	j[6] = new Judoka("Tim", "", 50);

	for (int i = 1; i <= count; ++i)
	{
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->IsThirdPlaceMatch(true);
	group->IsFifthPlaceMatch(true);
	group->SetMatID(1);
	t->AddMatchTable(group);

	group->SetStartPosition(j[1], 1-1);
	group->SetStartPosition(j[2], 2-1);
	group->SetStartPosition(j[3], 3-1);
	group->SetStartPosition(j[4], 4-1);
	group->SetStartPosition(j[5], 5-1);
	group->SetStartPosition(j[6], 6-1);

	EXPECT_EQ(group->GetMaxStartPositions(), 8);
	group->GenerateSchedule();

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	ASSERT_EQ(winner_schedule.size(),  4 + 2 + 1);
	EXPECT_EQ(loser_schedule.size(),   2 + 2 + 1 + 1);

	Mat m(1);

	t->GenerateSchedule();
	auto& schedule = group->GetSchedule();
	auto tournament_schedule = t->GetSchedule();
	ASSERT_EQ(tournament_schedule.size(), winner_schedule.size() + loser_schedule.size() - 4);

	EXPECT_FALSE(schedule[1 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[2 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[3 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[4 - 1]->IsEmptyMatch());

	EXPECT_FALSE(schedule[5 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[6 - 1]->IsEmptyMatch());

	EXPECT_TRUE(schedule[7 - 1]->IsEmptyMatch());
	EXPECT_TRUE(schedule[8 - 1]->IsEmptyMatch());

	EXPECT_FALSE(schedule[9  - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[10 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[1 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Alexa");
	EXPECT_EQ(schedule[1 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Wulf");
	EXPECT_TRUE(m.StartMatch(schedule[1 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(group->CalculateResults().GetSize(), 0);

	EXPECT_TRUE(schedule[2 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[3 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Hans");
	EXPECT_EQ(schedule[3 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Tim");
	EXPECT_TRUE(m.StartMatch(schedule[3 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(  schedule[4 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Peter");
	EXPECT_TRUE(schedule[4 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[5 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Alexa");
	EXPECT_EQ(schedule[5 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Johann");
	EXPECT_TRUE(m.StartMatch(schedule[5 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[6 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Tim");
	EXPECT_EQ(schedule[6 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Peter");
	EXPECT_TRUE(m.StartMatch(schedule[6 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(  schedule[7 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Hans");
	EXPECT_EQ(  schedule[7 - 1]->GetFighter(Fighter::Blue ), nullptr);
	EXPECT_TRUE(schedule[7 - 1]->IsEmptyMatch());

	EXPECT_EQ(  schedule[8 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Wulf");
	EXPECT_EQ(  schedule[8 - 1]->GetFighter(Fighter::Blue ), nullptr);
	EXPECT_TRUE(schedule[8 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[9 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[7 - 1]->GetUUID());
	EXPECT_EQ(schedule[9 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[5 - 1]->GetUUID());
	ASSERT_FALSE(schedule[9 - 1]->IsEmptyMatch());
	EXPECT_EQ(schedule[9 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Hans");
	EXPECT_EQ(schedule[9 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Alexa");
	EXPECT_TRUE(m.StartMatch(schedule[9 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[10 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[8 - 1]->GetUUID());
	EXPECT_EQ(schedule[10 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[6 - 1]->GetUUID());
	ASSERT_FALSE(schedule[10 - 1]->IsEmptyMatch());
	EXPECT_EQ(schedule[10 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Wulf");
	EXPECT_EQ(schedule[10 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Tim");
	EXPECT_TRUE(m.StartMatch(schedule[10 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[11 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[5 - 1]->GetUUID());
	EXPECT_EQ(schedule[11 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[6 - 1]->GetUUID());
	EXPECT_EQ(schedule[11 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Johann");
	EXPECT_EQ(schedule[11 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Peter");
	EXPECT_TRUE(m.StartMatch(schedule[11 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	//3rd place
	EXPECT_EQ(schedule[12 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[9  - 1]->GetUUID());
	EXPECT_EQ(schedule[12 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[10 - 1]->GetUUID());
	EXPECT_EQ(schedule[12 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Hans");
	EXPECT_EQ(schedule[12 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Tim");
	EXPECT_TRUE(m.StartMatch(schedule[12 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	//5th place
	EXPECT_EQ(schedule[13 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[9 - 1]->GetUUID());
	EXPECT_EQ(schedule[13 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[10 - 1]->GetUUID());
	EXPECT_EQ(schedule[13 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Alexa");
	EXPECT_EQ(schedule[13 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Wulf");
	EXPECT_TRUE(m.StartMatch(schedule[13 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());	


	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 6);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Johann");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Peter");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Hans");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Tim");
	EXPECT_EQ(results[4].Judoka->GetFirstname(), "Alexa");
	EXPECT_EQ(results[5].Judoka->GetFirstname(), "Wulf");

	t->DeleteAllMatchResults();

	for (auto m : tournament_schedule)
		EXPECT_TRUE(m->IsScheduled());
	for (auto m : schedule)
		EXPECT_TRUE(m->IsScheduled());
	for (auto m : winner_schedule)
		EXPECT_TRUE(m->IsScheduled());
	for (auto m : loser_schedule)
		EXPECT_TRUE(m->IsScheduled());

	delete t;
}



TEST(DoubleElimination, Count7)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 7;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 1; i <= count; ++i)
		group->SetStartPosition(j[i], i);

	EXPECT_EQ(group->GetMaxStartPositions(), 8);

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	//EXPECT_TRUE(loser_schedule[0]->Contains(*j[1]));

	ASSERT_EQ(winner_schedule.size(),  4 + 2 + 1);
	ASSERT_EQ(loser_schedule.size(),   2 + 2);

	Mat m(1);

	for (auto match : group->GetSchedule())
	{
		if (match->IsEmptyMatch())
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
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[7]->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j[6]->GetUUID());
	//EXPECT_EQ(results[2].Judoka->GetUUID(), j[5]->GetUUID());
	//EXPECT_EQ(results[3].Judoka->GetUUID(), j[4]->GetUUID());

	delete t;
}



TEST(DoubleElimination, Count7_With3rd)
{
	initialize();

	Tournament* t = new Tournament("Tournament Name");
	t->EnableAutoSave(false);

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 7;

	for (int i = 1; i <= count; ++i)
	{
		j[i] = new Judoka(GetFakeFirstname(), GetFakeLastname(), 50 + i);
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->IsThirdPlaceMatch(true);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 1; i <= count; ++i)
		group->SetStartPosition(j[i], i);

	EXPECT_EQ(group->GetMaxStartPositions(), 8);

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	//EXPECT_TRUE(loser_schedule[0]->Contains(*j[1]));

	ASSERT_EQ(winner_schedule.size(),  4 + 2 + 1);
	ASSERT_EQ(loser_schedule.size(),   2 + 2 + 1);

	Mat m(1);

	for (auto match : group->GetSchedule())
	{
		if (match->IsEmptyMatch())
			continue;

		EXPECT_TRUE(m.StartMatch(match));
		if (m.GetFighter(Fighter::White).GetWeight() > m.GetFighter(Fighter::Blue).GetWeight())
			m.AddIppon(Fighter::White);
		else
			m.AddIppon(Fighter::Blue);
		EXPECT_TRUE(m.EndMatch());
	}

	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 4);
	EXPECT_EQ(results[0].Judoka->GetUUID(), j[7]->GetUUID());
	EXPECT_EQ(results[1].Judoka->GetUUID(), j[6]->GetUUID());
	EXPECT_EQ(results[2].Judoka->GetUUID(), j[5]->GetUUID());
	EXPECT_EQ(results[3].Judoka->GetUUID(), j[4]->GetUUID());

	delete t;
}



TEST(DoubleElimination, Count7_Example)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament* t = new Tournament("deleteMe");

	Judoka* j[33];
	bool has_match[33];
	const size_t count = 7;

	j[1] = new Judoka("He", "", 50);
	j[2] = new Judoka("Sc", "", 50);
	j[3] = new Judoka("Li", "", 50);
	j[4] = new Judoka("Hi", "", 50);
	j[5] = new Judoka("Ku", "", 50);
	j[6] = new Judoka("Er", "", 50);
	j[7] = new Judoka("Gr", "", 50);

	for (int i = 1; i <= count; ++i)
	{
		has_match[i] = false;
		t->AddParticipant(j[i]);
	}

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->IsThirdPlaceMatch(true);
	group->IsFifthPlaceMatch(true);
	group->SetMatID(1);
	t->AddMatchTable(group);

	group->SetStartPosition(j[1], 1-1);
	group->SetStartPosition(j[2], 5-1);
	group->SetStartPosition(j[3], 3-1);
	group->SetStartPosition(j[4], 7-1);
	group->SetStartPosition(j[5], 2-1);
	group->SetStartPosition(j[6], 6-1);
	group->SetStartPosition(j[7], 4-1);

	EXPECT_EQ(group->GetMaxStartPositions(), 8);
	group->GenerateSchedule();

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	ASSERT_EQ(winner_schedule.size(),  4 + 2 + 1);
	EXPECT_EQ(loser_schedule.size(),   2 + 2 + 1 + 1);

	Mat m(1);

	auto& schedule = group->GetSchedule();
	EXPECT_EQ(schedule.size(), winner_schedule.size() + loser_schedule.size());

	EXPECT_FALSE(schedule[1 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[2 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[3 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[4 - 1]->IsEmptyMatch());

	EXPECT_FALSE(schedule[5 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[6 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[7 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[8 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[1 - 1]->GetFighter(Fighter::White)->GetFirstname(), "He");
	EXPECT_EQ(schedule[1 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Sc");
	EXPECT_TRUE(m.StartMatch(schedule[1 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[2 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Li");
	EXPECT_EQ(schedule[2 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Hi");
	EXPECT_TRUE(m.StartMatch(schedule[2 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[3 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Ku");
	EXPECT_EQ(schedule[3 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Er");
	EXPECT_TRUE(m.StartMatch(schedule[3 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[5 - 1]->GetFighter(Fighter::White)->GetFirstname(), "He");
	EXPECT_EQ(schedule[5 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Li");
	EXPECT_TRUE(m.StartMatch(schedule[5 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[6 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Er");
	EXPECT_EQ(schedule[6 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Gr");
	EXPECT_TRUE(m.StartMatch(schedule[6 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[7 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[3 - 1]->GetUUID());
	EXPECT_EQ(schedule[7 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[4 - 1]->GetUUID());
	EXPECT_EQ(schedule[7 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Ku");
	EXPECT_EQ(schedule[7 - 1]->GetFighter(Fighter::Blue ), nullptr);
	EXPECT_TRUE(schedule[7 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[8 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[1 - 1]->GetUUID());
	EXPECT_EQ(schedule[8 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[2 - 1]->GetUUID());
	EXPECT_EQ(schedule[8 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Sc");
	EXPECT_EQ(schedule[8 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Hi");
	EXPECT_TRUE(m.StartMatch(schedule[8 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[9 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[7 - 1]->GetUUID());
	EXPECT_EQ(schedule[9 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[5 - 1]->GetUUID());
	ASSERT_FALSE(schedule[9 - 1]->IsEmptyMatch());
	EXPECT_EQ(schedule[9 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Ku");
	EXPECT_EQ(schedule[9 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "He");
	EXPECT_TRUE(m.StartMatch(schedule[9 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[10 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[8 - 1]->GetUUID());
	EXPECT_EQ(schedule[10 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[6 - 1]->GetUUID());
	ASSERT_FALSE(schedule[10 - 1]->IsEmptyMatch());
	EXPECT_EQ(schedule[10 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Hi");
	EXPECT_EQ(schedule[10 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Er");
	EXPECT_TRUE(m.StartMatch(schedule[10 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[11 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[5 - 1]->GetUUID());
	EXPECT_EQ(schedule[11 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[6 - 1]->GetUUID());
	EXPECT_EQ(schedule[11 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Li");
	EXPECT_EQ(schedule[11 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Gr");
	EXPECT_TRUE(m.StartMatch(schedule[11 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	//3rd place
	EXPECT_EQ(schedule[12 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[9 - 1]->GetUUID());
	EXPECT_EQ(schedule[12 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[10 - 1]->GetUUID());
	EXPECT_EQ(schedule[12 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Ku");
	EXPECT_EQ(schedule[12 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Er");
	EXPECT_TRUE(m.StartMatch(schedule[12 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	//5th place
	EXPECT_EQ(schedule[13 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[9 - 1]->GetUUID());
	EXPECT_EQ(schedule[13 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[10 - 1]->GetUUID());
	EXPECT_EQ(schedule[13 - 1]->GetFighter(Fighter::White)->GetFirstname(), "He");
	EXPECT_EQ(schedule[13 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Hi");
	EXPECT_TRUE(m.StartMatch(schedule[13 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());	


	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 6);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Li");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Gr");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Er");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Ku");
	EXPECT_EQ(results[4].Judoka->GetFirstname(), "Hi");
	EXPECT_EQ(results[5].Judoka->GetFirstname(), "He");

	delete t;
}



TEST(DoubleElimination, Count11_Example)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament* t = new Tournament("deleteMe");

	Judoka* j[33] = {};
	const size_t count = 11;

	j[1]  = new Judoka("Alexa", "", 50);
	j[2]  = new Judoka("Hans", "", 50);
	j[3]  = new Judoka("Johann", "", 50);
	j[4]  = new Judoka("Peter", "", 50);
	j[5]  = new Judoka("Wulf", "", 50);
	j[6]  = new Judoka("Tim", "", 50);
	j[7]  = new Judoka("Matthias", "", 50);
	j[8]  = new Judoka("Petra", "", 50);
	j[9]  = new Judoka("Sophie", "", 50);
	j[10] = new Judoka("Luft", "", 50);
	j[11] = new Judoka("Kathrin", "", 50);

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->IsThirdPlaceMatch(true);
	group->IsFifthPlaceMatch(true);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 1; i <= 32; ++i)
	{
		if (j[i])
			t->AddParticipant(j[i]);
	}

	for (int i = 1; i <= 32; ++i)
	{
		if (j[i])
			group->SetStartPosition(j[i], i-1);
	}

	for (int i = 1; i <= 32; ++i)
	{
		if (j[i])
			EXPECT_EQ(*group->GetJudokaByStartPosition(i-1), *j[i]);
	}

	EXPECT_EQ(group->GetMaxStartPositions(), 16);
	group->GenerateSchedule();

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	ASSERT_EQ(winner_schedule.size(),  8 + 4 + 2 + 1);
	EXPECT_EQ(loser_schedule.size(),   4 + 4 + 2 + 2 + 1 + 1);

	Mat m(1);

	auto& schedule = group->GetSchedule();
	EXPECT_EQ(schedule.size(), winner_schedule.size() + loser_schedule.size());


	EXPECT_FALSE(schedule[1 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[2 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[3 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[4 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[5 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[6 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[7 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[8 - 1]->IsEmptyMatch());

	EXPECT_FALSE(schedule[9  - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[10 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[11 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[12 - 1]->IsEmptyMatch());

	EXPECT_TRUE( schedule[13 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[14 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[15 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[16 - 1]->IsEmptyMatch());

	EXPECT_FALSE(schedule[17 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[18 - 1]->IsEmptyMatch());
	EXPECT_FALSE(schedule[19 - 1]->IsEmptyMatch());
	EXPECT_TRUE( schedule[20 - 1]->IsEmptyMatch());


	EXPECT_EQ(schedule[1 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Alexa");
	EXPECT_EQ(schedule[1 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Sophie");
	EXPECT_TRUE(m.StartMatch(schedule[1 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[2 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Wulf");
	EXPECT_EQ(schedule[2 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[3 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Johann");
	EXPECT_EQ(schedule[3 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Kathrin");
	EXPECT_TRUE(m.StartMatch(schedule[3 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[4 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Matthias");
	EXPECT_EQ(schedule[4 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[5 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Hans");
	EXPECT_EQ(schedule[5 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Luft");
	EXPECT_TRUE(m.StartMatch(schedule[5 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[6 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Tim");
	EXPECT_EQ(schedule[6 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[7 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Peter");
	EXPECT_EQ(schedule[7 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[8 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Petra");
	EXPECT_EQ(schedule[8 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[9 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[1 - 1]->GetUUID());
	EXPECT_EQ(schedule[9 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[2 - 1]->GetUUID());
	EXPECT_EQ(schedule[9 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Alexa");
	EXPECT_EQ(schedule[9 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Wulf");
	EXPECT_TRUE(m.StartMatch(schedule[9 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[10 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[3 - 1]->GetUUID());
	EXPECT_EQ(schedule[10 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[4 - 1]->GetUUID());
	EXPECT_EQ(schedule[10 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Kathrin");
	EXPECT_EQ(schedule[10 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Matthias");
	EXPECT_TRUE(m.StartMatch(schedule[10 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[11 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[5 - 1]->GetUUID());
	EXPECT_EQ(schedule[11 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[6 - 1]->GetUUID());
	EXPECT_EQ(schedule[11 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Hans");
	EXPECT_EQ(schedule[11 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Tim");
	EXPECT_TRUE(m.StartMatch(schedule[11 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[12 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[7 - 1]->GetUUID());
	EXPECT_EQ(schedule[12 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[8 - 1]->GetUUID());
	EXPECT_EQ(schedule[12 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Peter");
	EXPECT_EQ(schedule[12 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Petra");
	EXPECT_TRUE(m.StartMatch(schedule[12 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[13 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[1 - 1]->GetUUID());
	EXPECT_EQ(schedule[13 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[2 - 1]->GetUUID());
	EXPECT_EQ(schedule[13 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Sophie");
	EXPECT_EQ(schedule[13 - 1]->GetFighter(Fighter::Blue ), nullptr);
	EXPECT_TRUE(schedule[13 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[14 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[3 - 1]->GetUUID());
	EXPECT_EQ(schedule[14 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[4 - 1]->GetUUID());
	EXPECT_EQ(schedule[14 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Johann");
	EXPECT_EQ(schedule[14 - 1]->GetFighter(Fighter::Blue ), nullptr);
	EXPECT_TRUE(schedule[14 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[15 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[5 - 1]->GetUUID());
	EXPECT_EQ(schedule[15 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[6 - 1]->GetUUID());
	EXPECT_EQ(schedule[15 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Luft");
	EXPECT_EQ(schedule[15 - 1]->GetFighter(Fighter::Blue ), nullptr);
	EXPECT_TRUE(schedule[15 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[16 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[7 - 1]->GetUUID());
	EXPECT_EQ(schedule[16 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[8 - 1]->GetUUID());
	EXPECT_EQ(schedule[16 - 1]->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(schedule[16 - 1]->GetFighter(Fighter::Blue ), nullptr);
	EXPECT_TRUE(schedule[16 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[17 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[13 - 1]->GetUUID());
	EXPECT_EQ(schedule[17 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[11 - 1]->GetUUID());
	EXPECT_EQ(schedule[17 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Sophie");
	EXPECT_EQ(schedule[17 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Tim");
	EXPECT_TRUE(m.StartMatch(schedule[17 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[18 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[14 - 1]->GetUUID());
	EXPECT_EQ(schedule[18 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[12 - 1]->GetUUID());
	EXPECT_EQ(schedule[18 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Johann");
	EXPECT_EQ(schedule[18 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Peter");
	EXPECT_TRUE(m.StartMatch(schedule[18 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[19 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[15 - 1]->GetUUID());
	EXPECT_EQ(schedule[19 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[9 - 1]->GetUUID());
	EXPECT_EQ(schedule[19 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Luft");
	EXPECT_EQ(schedule[19 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Alexa");
	EXPECT_TRUE(m.StartMatch(schedule[19 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[20 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[16 - 1]->GetUUID());
	EXPECT_EQ(schedule[20 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[10 - 1]->GetUUID());
	EXPECT_EQ(schedule[20 - 1]->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(schedule[20 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Kathrin");
	EXPECT_TRUE(schedule[20 - 1]->IsEmptyMatch());

	EXPECT_EQ(schedule[21 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[9 - 1]->GetUUID());
	EXPECT_EQ(schedule[21 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[10 - 1]->GetUUID());
	EXPECT_EQ(schedule[21 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Wulf");
	EXPECT_EQ(schedule[21 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Matthias");
	EXPECT_TRUE(m.StartMatch(schedule[21 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[22 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[11 - 1]->GetUUID());
	EXPECT_EQ(schedule[22 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[12 - 1]->GetUUID());
	EXPECT_EQ(schedule[22 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Hans");
	EXPECT_EQ(schedule[22 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Petra");
	EXPECT_TRUE(m.StartMatch(schedule[22 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[23 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[17 - 1]->GetUUID());
	EXPECT_EQ(schedule[23 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[18 - 1]->GetUUID());
	EXPECT_EQ(schedule[23 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Tim");
	EXPECT_EQ(schedule[23 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Peter");
	EXPECT_TRUE(m.StartMatch(schedule[23 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[24 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[19 - 1]->GetUUID());
	EXPECT_EQ(schedule[24 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[20 - 1]->GetUUID());
	EXPECT_EQ(schedule[24 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Alexa");
	EXPECT_EQ(schedule[24 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Kathrin");
	EXPECT_TRUE(m.StartMatch(schedule[24 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[25 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[23 - 1]->GetUUID());
	EXPECT_EQ(schedule[25 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[21 - 1]->GetUUID());
	EXPECT_EQ(schedule[25 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Peter");
	EXPECT_EQ(schedule[25 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Matthias");
	EXPECT_TRUE(m.StartMatch(schedule[25 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[26 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[24 - 1]->GetUUID());
	EXPECT_EQ(schedule[26 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[22 - 1]->GetUUID());
	EXPECT_EQ(schedule[26 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Alexa");
	EXPECT_EQ(schedule[26 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Hans");
	EXPECT_TRUE(m.StartMatch(schedule[26 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[27 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[21 - 1]->GetUUID());
	EXPECT_EQ(schedule[27 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[22 - 1]->GetUUID());
	EXPECT_EQ(schedule[27 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Wulf");
	EXPECT_EQ(schedule[27 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Petra");
	EXPECT_TRUE(m.StartMatch(schedule[27 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[28 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[25 - 1]->GetUUID());
	EXPECT_EQ(schedule[28 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[26 - 1]->GetUUID());
	EXPECT_EQ(schedule[28 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Matthias");
	EXPECT_EQ(schedule[28 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Alexa");
	EXPECT_TRUE(m.StartMatch(schedule[28 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[29 - 1]->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[25 - 1]->GetUUID());
	EXPECT_EQ(schedule[29 - 1]->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[26 - 1]->GetUUID());
	EXPECT_EQ(schedule[29 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Peter");
	EXPECT_EQ(schedule[29 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Hans");
	EXPECT_TRUE(m.StartMatch(schedule[29 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());


	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 6);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Wulf");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Petra");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Matthias");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Alexa");
	EXPECT_EQ(results[4].Judoka->GetFirstname(), "Peter");
	EXPECT_EQ(results[5].Judoka->GetFirstname(), "Hans");

	delete t;
}



TEST(DoubleElimination, Count20_Example)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");
	Tournament* t = new Tournament("deleteMe");

	Judoka* j[33] = {};
	const size_t count = 11;

	j[1]  = new Judoka("Carla", "", 50);
	j[2]  = new Judoka("Andreas", "", 50);
	j[3]  = new Judoka("Julian", "", 50);
	j[4]  = new Judoka("Stephanie", "", 50);
	j[5]  = new Judoka("Phillip", "", 50);
	j[6]  = new Judoka("Dresdner", "", 50);
	j[7]  = new Judoka("Stephan", "", 50);
	j[8]  = new Judoka("Emma", "", 50);
	j[9]  = new Judoka("Maximilian", "", 50);
	j[10] = new Judoka("Carla", "", 50);
	j[11] = new Judoka("Paulina", "", 50);
	j[12] = new Judoka("Tim", "", 50);
	j[13] = new Judoka("Anna", "", 50);
	j[14] = new Judoka("Soeren", "", 50);
	j[15] = new Judoka("Kerstin", "", 50);
	j[16] = new Judoka("Emily", "", 50);
	j[17] = new Judoka("Hans", "", 50);
	j[18] = new Judoka("Jade", "", 50);
	j[19] = new Judoka("Sophia", "", 50);
	j[20] = new Judoka("Wurst", "", 50);

	DoubleElimination* group = new DoubleElimination(0, 200);
	group->IsThirdPlaceMatch(true);
	group->IsFifthPlaceMatch(true);
	group->SetMatID(1);
	t->AddMatchTable(group);

	for (int i = 1; i <= 32; ++i)
	{
		if (j[i])
			t->AddParticipant(j[i]);
	}

	for (int i = 1; i <= 32; ++i)
	{
		if (j[i])
			group->SetStartPosition(j[i], i-1);
	}

	for (int i = 1; i <= 32; ++i)
	{
		if (j[i])
			EXPECT_EQ(*group->GetJudokaByStartPosition(i-1), *j[i]);
	}

	EXPECT_EQ(group->GetMaxStartPositions(), 32);
	group->GenerateSchedule();

	auto& winner_schedule = group->GetWinnerBracket().GetSchedule();
	auto& loser_schedule  = group->GetLoserBracket() .GetSchedule();

	ASSERT_EQ(winner_schedule.size(),  16 + 8 + 4 + 2 + 1);
	EXPECT_EQ(loser_schedule.size(),    8 + 8 + 4 + 4 + 2 + 2 + 1 + 1);

	Mat m(1);

	auto& schedule = group->GetSchedule();
	EXPECT_EQ(schedule.size(), winner_schedule.size() + loser_schedule.size());

	t->Save();
	

	EXPECT_EQ(schedule[1 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Carla");
	EXPECT_EQ(schedule[1 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Hans");
	EXPECT_TRUE(m.StartMatch(schedule[1 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[2 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Maximilian");
	EXPECT_EQ(schedule[2 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[3 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Phillip");
	EXPECT_EQ(schedule[3 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[4 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Anna");
	EXPECT_EQ(schedule[4 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[5 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Julian");
	EXPECT_EQ(schedule[5 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Sophia");
	EXPECT_TRUE(m.StartMatch(schedule[5 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[6 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Paulina");
	EXPECT_EQ(schedule[6 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[7 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Stephan");
	EXPECT_EQ(schedule[7 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[8 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Kerstin");
	EXPECT_EQ(schedule[8 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[9 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Andreas");
	EXPECT_EQ(schedule[9 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Jade");
	EXPECT_TRUE(m.StartMatch(schedule[9 - 1]));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[10 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Carla");
	EXPECT_EQ(schedule[10 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[11 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Dresdner");
	EXPECT_EQ(schedule[11 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[12 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Soeren");
	EXPECT_EQ(schedule[12 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[13 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Stephanie");
	EXPECT_EQ(schedule[13 - 1]->GetFighter(Fighter::Blue )->GetFirstname(), "Wurst");
	EXPECT_TRUE(m.StartMatch(schedule[13 - 1]));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	EXPECT_EQ(schedule[14 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Tim");
	EXPECT_EQ(schedule[14 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[15 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Emma");
	EXPECT_EQ(schedule[15 - 1]->GetFighter(Fighter::Blue ), nullptr);

	EXPECT_EQ(schedule[16 - 1]->GetFighter(Fighter::White)->GetFirstname(), "Emily");
	EXPECT_EQ(schedule[16 - 1]->GetFighter(Fighter::Blue ), nullptr);

	//

	auto match = schedule[17 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[1 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[2 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Carla");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Maximilian");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[18 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[3 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[4 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Phillip");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Anna");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[19 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[5 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[6 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Sophia");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Paulina");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[20 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[7 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[8 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Stephan");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Kerstin");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[21 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[9 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[10 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Andreas");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Carla");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[22 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[11 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[12 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Dresdner");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Soeren");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[23 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[13 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[14 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Wurst");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Tim");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[24 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[15 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[16 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Emma");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Emily");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	//

	match = schedule[25 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[13 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[14 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Stephanie");
	EXPECT_EQ(match->GetFighter(Fighter::Blue ), nullptr);

	match = schedule[26 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[15 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[16 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(match->GetFighter(Fighter::Blue ), nullptr);

	match = schedule[27 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[9 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[10 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Jade");
	EXPECT_EQ(match->GetFighter(Fighter::Blue ), nullptr);
	
	match = schedule[28 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[11 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[12 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(match->GetFighter(Fighter::Blue ), nullptr);

	match = schedule[29 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[5 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[6 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Julian");
	EXPECT_EQ(match->GetFighter(Fighter::Blue ), nullptr);

	match = schedule[30 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[7 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[8 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(match->GetFighter(Fighter::Blue ), nullptr);

	match = schedule[31 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[1 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[2 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Hans");
	EXPECT_EQ(match->GetFighter(Fighter::Blue ), nullptr);

	match = schedule[32 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[3 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[4 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(match->GetFighter(Fighter::Blue ), nullptr);

	//

	match = schedule[33 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[17 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[18 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Carla");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Anna");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[34 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[19 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[20 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Sophia");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Kerstin");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[35 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[21 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[22 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Andreas");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Soeren");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[36 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[23 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[24 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Wurst");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Emily");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	//

	match = schedule[37 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[25 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[21 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Stephanie");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Carla");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[38 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[26 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[22 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Dresdner");

	match = schedule[39 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[27 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[23 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Jade");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Tim");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[40 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[28 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[24 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Emma");

	match = schedule[41 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[29 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[17 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Julian");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Maximilian");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[42 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[30 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[18 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Phillip");

	match = schedule[43 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[31 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[19 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Hans");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Paulina");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[44 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[32 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[20 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White), nullptr);
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Stephan");

	//

	match = schedule[45 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[33 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[34 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Carla");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Kerstin");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[46 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[35 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[36 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Andreas");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Emily");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	//

	match = schedule[47 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[37 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[38 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Stephanie");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Dresdner");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[48 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[39 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[40 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Tim");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Emma");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[49 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[41 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[42 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Julian");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Phillip");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[50 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[43 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[44 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Paulina");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Stephan");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	//

	match = schedule[51 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[47 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[33 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Stephanie");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Anna");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[52 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[48 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[34 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Emma");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Sophia");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[53 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[49 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[35 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Julian");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Soeren");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[54 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[50 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[36 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Stephan");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Wurst");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	//

	match = schedule[55 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[51 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[52 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Stephanie");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Sophia");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[56 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[53 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[54 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Julian");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Wurst");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	//

	match = schedule[57 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[55 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[46 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Stephanie");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Emily");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[58 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[56 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[45 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Wurst");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Carla");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());

	//

	match = schedule[59 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[45 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[46 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Kerstin");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Andreas");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[60 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[57 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[58 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Stephanie");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Carla");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::White);
	EXPECT_TRUE(m.EndMatch());

	match = schedule[61 - 1];
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::White)->GetUUID(), schedule[57 - 1]->GetUUID());
	EXPECT_EQ(match->GetDependentMatchOf(Fighter::Blue )->GetUUID(), schedule[58 - 1]->GetUUID());
	EXPECT_EQ(match->GetFighter(Fighter::White)->GetFirstname(), "Emily");
	EXPECT_EQ(match->GetFighter(Fighter::Blue )->GetFirstname(), "Wurst");
	EXPECT_TRUE(m.StartMatch(match));
	m.AddIppon(Fighter::Blue);
	EXPECT_TRUE(m.EndMatch());



	auto results = group->CalculateResults();

	ASSERT_EQ(results.GetSize(), 6);
	EXPECT_EQ(results[0].Judoka->GetFirstname(), "Kerstin");
	EXPECT_EQ(results[1].Judoka->GetFirstname(), "Andreas");
	EXPECT_EQ(results[2].Judoka->GetFirstname(), "Stephanie");
	EXPECT_EQ(results[3].Judoka->GetFirstname(), "Carla");
	EXPECT_EQ(results[4].Judoka->GetFirstname(), "Wurst");
	EXPECT_EQ(results[5].Judoka->GetFirstname(), "Emily");

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
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[18]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[19]));

	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[20]));
	EXPECT_TRUE(group->GetWinnerBracket().FindMatch(*group->GetSchedule()[21]));

	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[22]));
	EXPECT_TRUE(group->GetLoserBracket().FindMatch(*group->GetSchedule()[23]));

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

	for (size_t count = 33; count <= 64; count++)
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

		EXPECT_EQ(group->GetMaxStartPositions(), 64);

		EXPECT_EQ(group->GetSchedule().size(), 32+16+8+4+2+1 + 16+16+8+8+4+4+2+2);
		auto& loser_schedule = group->GetLoserBracket().GetSchedule();

		t.AddMatchTable(group);

		YAML::Emitter yaml;
		*group >> yaml;

		DoubleElimination group2(YAML::Load(yaml.c_str()), &t);

		EXPECT_EQ(group->GetSchedule().size(), group2.GetSchedule().size());
		EXPECT_EQ(group->ToHTML(), group2.ToHTML());

		//Pointers should point to the same object
		EXPECT_EQ((void*)group->GetSchedule()[0], (void*)group->GetWinnerBracket().GetSchedule()[0]);
		EXPECT_EQ((void*)group2.GetSchedule()[0], (void*)group2.GetWinnerBracket().GetSchedule()[0]);

		for (int i = 1; i <= count; ++i)
			delete j[i];
	}
}