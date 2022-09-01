#include "tests.h"



TEST(Weightclass, Eligable_NoMaxWeight)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(10, 0);

	EXPECT_TRUE(w.IsElgiable(j1));
}



TEST(Weightclass, CorrectOrder2)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(10, 100);

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));

	w.GenerateSchedule();
	ASSERT_EQ(w.GetSchedule().size(), 1);

	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::White)->GetUUID(), j2.GetUUID());
	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());
}



TEST(Weightclass, CorrectOrder2_BO3)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(10, 100);
	w.IsBestOfThree(true);

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));

	w.GenerateSchedule();
	ASSERT_EQ(w.GetSchedule().size(), 3);

	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::White)->GetUUID(), j2.GetUUID());
	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::White)->GetUUID(), j1.GetUUID());
	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::White)->GetUUID(), j2.GetUUID());
	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());
}



TEST(Weightclass, CorrectOrder3)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(10, 100);

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));
	EXPECT_TRUE(w.AddParticipant(&j3));

	w.GenerateSchedule();
	ASSERT_EQ(w.GetSchedule().size(), 3);

	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::White)->GetUUID(), j3.GetUUID());
	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::White)->GetUUID(), j2.GetUUID());
	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::White)->GetUUID(), j3.GetUUID());
	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());
}



TEST(Weightclass, CorrectOrder4)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(10, 100);

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));
	EXPECT_TRUE(w.AddParticipant(&j3));
	EXPECT_TRUE(w.AddParticipant(&j4));

	w.GenerateSchedule();
	ASSERT_EQ(w.GetSchedule().size(), 6);

	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::White)->GetUUID(), j3.GetUUID());
	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::White)->GetUUID(), j4.GetUUID());
	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::White)->GetUUID(), j4.GetUUID());
	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[3]->GetFighter(Fighter::White)->GetUUID(), j3.GetUUID());
	EXPECT_EQ(w.GetSchedule()[3]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[4]->GetFighter(Fighter::White)->GetUUID(), j2.GetUUID());
	EXPECT_EQ(w.GetSchedule()[4]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[5]->GetFighter(Fighter::White)->GetUUID(), j4.GetUUID());
	EXPECT_EQ(w.GetSchedule()[5]->GetFighter(Fighter::Blue )->GetUUID(), j3.GetUUID());
}



TEST(Weightclass, CorrectOrder5)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j5(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(10, 100);

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));
	EXPECT_TRUE(w.AddParticipant(&j3));
	EXPECT_TRUE(w.AddParticipant(&j4));
	EXPECT_TRUE(w.AddParticipant(&j5));

	w.GenerateSchedule();
	ASSERT_EQ(w.GetSchedule().size(), 10);

	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::White)->GetUUID(), j4.GetUUID());
	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::White)->GetUUID(), j5.GetUUID());
	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::White)->GetUUID(), j3.GetUUID());
	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[3]->GetFighter(Fighter::White)->GetUUID(), j4.GetUUID());
	EXPECT_EQ(w.GetSchedule()[3]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[4]->GetFighter(Fighter::White)->GetUUID(), j5.GetUUID());
	EXPECT_EQ(w.GetSchedule()[4]->GetFighter(Fighter::Blue )->GetUUID(), j3.GetUUID());

	EXPECT_EQ(w.GetSchedule()[5]->GetFighter(Fighter::White)->GetUUID(), j2.GetUUID());
	EXPECT_EQ(w.GetSchedule()[5]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[6]->GetFighter(Fighter::White)->GetUUID(), j4.GetUUID());
	EXPECT_EQ(w.GetSchedule()[6]->GetFighter(Fighter::Blue )->GetUUID(), j3.GetUUID());

	EXPECT_EQ(w.GetSchedule()[7]->GetFighter(Fighter::White)->GetUUID(), j5.GetUUID());
	EXPECT_EQ(w.GetSchedule()[7]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[8]->GetFighter(Fighter::White)->GetUUID(), j3.GetUUID());
	EXPECT_EQ(w.GetSchedule()[8]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[9]->GetFighter(Fighter::White)->GetUUID(), j5.GetUUID());
	EXPECT_EQ(w.GetSchedule()[9]->GetFighter(Fighter::Blue )->GetUUID(), j4.GetUUID());
}



TEST(Weightclass, CorrectOrder6)
{
	initialize();

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j3(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j4(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j5(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j6(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	Weightclass w(10, 100);

	EXPECT_TRUE(w.AddParticipant(&j1));
	EXPECT_TRUE(w.AddParticipant(&j2));
	EXPECT_TRUE(w.AddParticipant(&j3));
	EXPECT_TRUE(w.AddParticipant(&j4));
	EXPECT_TRUE(w.AddParticipant(&j5));
	EXPECT_TRUE(w.AddParticipant(&j6));

	w.GenerateSchedule();
	ASSERT_EQ(w.GetSchedule().size(), 15);

	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::White)->GetUUID(), j4.GetUUID());
	EXPECT_EQ(w.GetSchedule()[0]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::White)->GetUUID(), j5.GetUUID());
	EXPECT_EQ(w.GetSchedule()[1]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::White)->GetUUID(), j6.GetUUID());
	EXPECT_EQ(w.GetSchedule()[2]->GetFighter(Fighter::Blue )->GetUUID(), j3.GetUUID());

	EXPECT_EQ(w.GetSchedule()[3]->GetFighter(Fighter::White)->GetUUID(), j4.GetUUID());
	EXPECT_EQ(w.GetSchedule()[3]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[4]->GetFighter(Fighter::White)->GetUUID(), j5.GetUUID());
	EXPECT_EQ(w.GetSchedule()[4]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[5]->GetFighter(Fighter::White)->GetUUID(), j6.GetUUID());
	EXPECT_EQ(w.GetSchedule()[5]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[6]->GetFighter(Fighter::White)->GetUUID(), j3.GetUUID());
	EXPECT_EQ(w.GetSchedule()[6]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[7]->GetFighter(Fighter::White)->GetUUID(), j6.GetUUID());
	EXPECT_EQ(w.GetSchedule()[7]->GetFighter(Fighter::Blue )->GetUUID(), j4.GetUUID());

	EXPECT_EQ(w.GetSchedule()[8]->GetFighter(Fighter::White)->GetUUID(), j5.GetUUID());
	EXPECT_EQ(w.GetSchedule()[8]->GetFighter(Fighter::Blue )->GetUUID(), j3.GetUUID());

	EXPECT_EQ(w.GetSchedule()[9]->GetFighter(Fighter::White)->GetUUID(), j2.GetUUID());
	EXPECT_EQ(w.GetSchedule()[9]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[10]->GetFighter(Fighter::White)->GetUUID(), j5.GetUUID());
	EXPECT_EQ(w.GetSchedule()[10]->GetFighter(Fighter::Blue )->GetUUID(), j4.GetUUID());

	EXPECT_EQ(w.GetSchedule()[11]->GetFighter(Fighter::White)->GetUUID(), j6.GetUUID());
	EXPECT_EQ(w.GetSchedule()[11]->GetFighter(Fighter::Blue )->GetUUID(), j1.GetUUID());

	EXPECT_EQ(w.GetSchedule()[12]->GetFighter(Fighter::White)->GetUUID(), j3.GetUUID());
	EXPECT_EQ(w.GetSchedule()[12]->GetFighter(Fighter::Blue )->GetUUID(), j2.GetUUID());

	EXPECT_EQ(w.GetSchedule()[13]->GetFighter(Fighter::White)->GetUUID(), j6.GetUUID());
	EXPECT_EQ(w.GetSchedule()[13]->GetFighter(Fighter::Blue )->GetUUID(), j5.GetUUID());

	EXPECT_EQ(w.GetSchedule()[14]->GetFighter(Fighter::White)->GetUUID(), j4.GetUUID());
	EXPECT_EQ(w.GetSchedule()[14]->GetFighter(Fighter::Blue )->GetUUID(), j3.GetUUID());
}



TEST(Weightclass, ExportImport)
{
	initialize();

	ZED::Core::RemoveFile("tournaments/deleteMe.yml");

	Tournament t("deleteMe");
	t.EnableAutoSave(false);

	Judoka j1(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);
	Judoka j2(GetFakeFirstname(), GetFakeLastname(), 50, Gender::Male);

	auto w = new Weightclass(10, 100);

	EXPECT_TRUE(w->AddParticipant(&j1));
	EXPECT_TRUE(w->AddParticipant(&j2));

	w->GenerateSchedule();
	ASSERT_EQ(w->GetSchedule().size(), 1);

	t.AddMatchTable(w);


	{
		YAML::Emitter yaml;
		yaml << YAML::BeginMap;
		*w >> yaml;
		yaml << YAML::EndMap;

		Weightclass group2(YAML::Load(yaml.c_str()), &t);

		EXPECT_EQ(group2.GetSchedule().size(), w->GetSchedule().size());
		EXPECT_EQ(group2.ToHTML(),  w->ToHTML());
		EXPECT_EQ(group2.GetUUID(), w->GetUUID());
	}
}