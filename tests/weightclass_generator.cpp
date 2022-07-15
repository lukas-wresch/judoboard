#include "tests.h"



TEST(WeightclassGenerator, Median)
{
	initialize();
	
	std::vector<std::pair<int, int>> v = { {10, 0}, {20, 0}, {30, 0} };
	EXPECT_EQ(Generator::calculateMedian(v, 0, 1), 10);
	EXPECT_EQ(Generator::calculateMedian(v, 0, 2), 15);
	EXPECT_EQ(Generator::calculateMedian(v, 0, 3), 20);

	EXPECT_EQ(Generator::calculateMedian(v, 1, 1), 0);
	EXPECT_EQ(Generator::calculateMedian(v, 1, 2), 20);
	EXPECT_EQ(Generator::calculateMedian(v, 1, 3), 25);

	EXPECT_EQ(Generator::calculateMedian(v, 2, 1), 0);
	EXPECT_EQ(Generator::calculateMedian(v, 2, 2), 0);
	EXPECT_EQ(Generator::calculateMedian(v, 2, 3), 30);

	std::vector<std::pair<int, int>> v2 = { {10, 0}, {20, 0}, {30, 0}, {40, 0} };
	EXPECT_EQ(Generator::calculateMedian(v2, 0, 1), 10);
	EXPECT_EQ(Generator::calculateMedian(v2, 0, 2), 15);
	EXPECT_EQ(Generator::calculateMedian(v2, 0, 3), 20);
	EXPECT_EQ(Generator::calculateMedian(v2, 0, 4), 25);
}