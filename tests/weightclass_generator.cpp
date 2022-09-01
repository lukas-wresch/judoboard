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