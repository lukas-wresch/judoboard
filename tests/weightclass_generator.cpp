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



TEST(WeightclassGenerator, Test1)
{
	initialize();

	std::vector<int> weightsSlots = {30, 35, 43, 46, 48, 53, 60, 63, 64, 65, 67, 69, 69, 69, 72, 77, 81};

	Generator gen;
	gen.m_Min  = 2;
	gen.m_Max  = 5;
	gen.m_Diff = 9;
	auto result = gen.split(weightsSlots);



	gen.m_Min  = 1;
	gen.m_Max  = 1;
	gen.m_Diff = 10;
	result = gen.split(weightsSlots);


	gen.m_Min  = 1;
	gen.m_Max  = 2;
	gen.m_Diff = 30;
	result = gen.split(weightsSlots);
}