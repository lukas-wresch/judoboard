#include "tests.h"



TEST(WeightclassGenerator, Median)
{
	initialize();
	
	std::vector<std::pair<Weight, int>> v = { {10, 0}, {20, 0}, {30, 0} };
	EXPECT_EQ(Generator::calculateMedian(v, 0, 1), Weight(10));
	EXPECT_EQ(Generator::calculateMedian(v, 0, 2), Weight(15));
	EXPECT_EQ(Generator::calculateMedian(v, 0, 3), Weight(20));

	EXPECT_EQ(Generator::calculateMedian(v, 1, 1), Weight(0));
	EXPECT_EQ(Generator::calculateMedian(v, 1, 2), Weight(20));
	EXPECT_EQ(Generator::calculateMedian(v, 1, 3), Weight(25));

	EXPECT_EQ(Generator::calculateMedian(v, 2, 1), Weight(0));
	EXPECT_EQ(Generator::calculateMedian(v, 2, 2), Weight(0));
	EXPECT_EQ(Generator::calculateMedian(v, 2, 3), Weight(30));

	std::vector<std::pair<Weight, int>> v2 = { {10, 0}, {20, 0}, {30, 0}, {40, 0} };
	EXPECT_EQ(Generator::calculateMedian(v2, 0, 1), Weight(10));
	EXPECT_EQ(Generator::calculateMedian(v2, 0, 2), Weight(15));
	EXPECT_EQ(Generator::calculateMedian(v2, 0, 3), Weight(20));
	EXPECT_EQ(Generator::calculateMedian(v2, 0, 4), Weight(25));
}



TEST(WeightclassGenerator, SmallSplit)
{
	initialize();

	std::vector<Weight> weightsSlots = {28, 36, 53, 58, 63, 76, 79};

	Generator gen;
	gen.m_Min  = 1;
	gen.m_Max  = 10;
	gen.m_Diff = 22;
	auto result = gen.split(weightsSlots);

	//Suggested split: 28 36 | 53 58 63 | 76 79
	ASSERT_EQ(result.m_Collection.size(), 3);
	EXPECT_EQ(result.m_Collection[0].m_NumParticipants, 2);
	EXPECT_EQ(result.m_Collection[1].m_NumParticipants, 3);
	EXPECT_EQ(result.m_Collection[2].m_NumParticipants, 2);
}



TEST(WeightclassGenerator, NoGroupWith1Participant_1)
{
	initialize();

	std::vector<Weight> weightsSlots = { 42, 46, 46, 48 };

	Generator gen;
	gen.m_Min  = 2;
	gen.m_Max  = 5;
	gen.m_Diff = 1;
	auto result = gen.split(weightsSlots);

	for (auto res : result.m_Collection)
		EXPECT_NE(res.m_NumParticipants, 1);
}



TEST(WeightclassGenerator, NoGroupWith1Participant_2)
{
	initialize();

	std::vector<Weight> weightsSlots = {25, 35, 35, 42, 46, 46, 48, 49, 51, 54, 54, 55, 60, 61, 66, 66, 67, 67, 68, 68, 69, 69, 72, 72, 74, 75, 76, 77, 79, 81, 81 };

	Generator gen;
	gen.m_Min  = 2;
	gen.m_Max  = 5;
	gen.m_Diff = 1;
	auto result = gen.split(weightsSlots);

	for (auto res : result.m_Collection)
		EXPECT_NE(res.m_NumParticipants, 1);
}



TEST(WeightclassGenerator, CorrectOutput)
{
	initialize();

	std::vector<Weight> weights = { 25, 35, 42, 54, 63, 65, 66, 67, 67, 68, 72, 74, 76, 76, 81 };

	Generator gen;
	gen.m_Min  = 3;
	gen.m_Max  = 5;
	gen.m_Diff = 30;
	
	std::vector<std::pair<Weight, int>> weightsSlots;
	for (auto weight : weights)
		weightsSlots.emplace_back(weight, 0);

	gen.split(weightsSlots, 0, (int)weightsSlots.size());

	WeightclassDescCollection result(weightsSlots);



	std::string line;
	for (auto w : weights)
		line += w.ToString() + " ";
	ZED::Log::Debug(line);

	std::string line2;
	for (auto [weight, slot] : weightsSlots)
		line2 += weight.ToString() + "(" + std::to_string(slot)  + ") ";
	ZED::Log::Debug(line2);

	for (auto r : result.m_Collection)
		ZED::Log::Info(r.m_Min.ToString() + " - " + r.m_Max.ToString() + "   #" + std::to_string(r.m_NumParticipants));

	ASSERT_EQ(result.m_Collection.size(), 4);
	EXPECT_EQ(result.m_Collection[0].m_NumParticipants, 3);
	EXPECT_EQ(result.m_Collection[1].m_NumParticipants, 3);
	EXPECT_EQ(result.m_Collection[2].m_NumParticipants, 5);
	EXPECT_EQ(result.m_Collection[3].m_NumParticipants, 4);

	EXPECT_GE(result.m_Collection[0].m_Max, Weight(42));
	EXPECT_LT(result.m_Collection[0].m_Max, Weight(54));

	EXPECT_GE(result.m_Collection[1].m_Min, Weight(54));
	EXPECT_GE(result.m_Collection[1].m_Max, Weight(65));
	EXPECT_LT(result.m_Collection[1].m_Max, Weight(66));

	EXPECT_GE(result.m_Collection[2].m_Min, Weight(66));
	EXPECT_GE(result.m_Collection[2].m_Max, Weight(72));
	EXPECT_LT(result.m_Collection[2].m_Max, Weight(74));
}