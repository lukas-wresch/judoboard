#include "tests.h"



TEST(Club, Basic)
{
	initialize();
	Club c("name");

	c.SetShortName("short");

	EXPECT_EQ(c.GetName(), "name");
	EXPECT_EQ(c.GetShortName(), "short");
}



TEST(Club, ImportExport)
{
	for (int i = 0; i < 1000 * 10; i++)
	{
		Club c1(GetRandomName());
		c1.SetShortName(GetRandomName());

		YAML::Emitter yaml;
		c1 >> yaml;

		Club c2(YAML::Load(yaml.c_str()));

		EXPECT_EQ(c1.GetUUID(), c2.GetUUID());
		EXPECT_EQ(c1.GetName(), c2.GetName());
		EXPECT_EQ(c1.GetShortName(), c2.GetShortName());
	}
}