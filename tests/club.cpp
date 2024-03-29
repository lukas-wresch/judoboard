#include "tests.h"



TEST(Club, Basic)
{
	initialize();
	Club c("name");

	c.SetShortName("short");

	EXPECT_EQ(c.GetName(), "name");
	EXPECT_EQ(c.GetShortName(), "short");
	//EXPECT_EQ(c.GetLevel(), -1);
	EXPECT_EQ(c.GetParent(), nullptr);
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



TEST(Club, IsChildOf)
{
	initialize();
	Application app;

	auto inter = new Judoboard::Association("International", nullptr);

	auto de = new Judoboard::Association("Deutschland", inter);

	auto dn = new Judoboard::Association("Deutschland-Nord", de);
	auto ds = new Judoboard::Association(u8"Deutschland-S\u00fcd", de);

	auto nord  = new Judoboard::Association("Nord", dn);
	auto west  = new Judoboard::Association("West", dn);
	auto nost  = new Judoboard::Association("Nordost", dn);
	auto sued  = new Judoboard::Association(u8"S\u00fcd", ds);
	auto swest = new Judoboard::Association(u8"S\u00fcdwest", ds);

	auto nieder  = new Judoboard::Association("Niedersachsen", nord);
	auto hamburg = new Judoboard::Association("Hamburg", nord);
	auto berlin  = new Judoboard::Association("Berlin", nost);
	auto nrw     = new Judoboard::Association("Nordrhein-Westfalen", west);

	app.GetDatabase().AddAssociation(nieder);
	app.GetDatabase().AddAssociation(hamburg);
	app.GetDatabase().AddAssociation(berlin);
	app.GetDatabase().AddAssociation(nrw);

	EXPECT_TRUE(nieder->IsChildOf(*nord));
	EXPECT_TRUE(nieder->IsChildOf(*dn));
	EXPECT_TRUE(nieder->IsChildOf(*de));
	EXPECT_TRUE(nieder->IsChildOf(*inter));
	EXPECT_FALSE(nieder->IsChildOf(*west));
	EXPECT_FALSE(nieder->IsChildOf(*ds));
	EXPECT_FALSE(nieder->IsChildOf(*swest));

	EXPECT_TRUE(nrw->IsChildOf(*west));
	EXPECT_TRUE(nrw->IsChildOf(*dn));
	EXPECT_TRUE(nrw->IsChildOf(*de));
	EXPECT_TRUE(nrw->IsChildOf(*inter));
	EXPECT_FALSE(nrw->IsChildOf(*swest));
}