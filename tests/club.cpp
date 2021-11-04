#include "tests.h"



TEST(Club, Basic)
{
	initialize();
	Club c("name");

	EXPECT_TRUE(c.GetName() == "name");
}



TEST(Club, ImportExport)
{
	for (int i = 0; i < 1000 * 10; i++)
	{
		Club c1(GetRandomName());

		ZED::CSV csv;
		c1 >> csv;

		Club c2(csv);

		EXPECT_TRUE(c1.GetName() == c2.GetName());
	}
}