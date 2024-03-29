#include "tests.h"



TEST(Timer, Accuracy)
{
	initialize();
	for (int i = 0; i < 2; i++)
	{
		Timer t;

		EXPECT_FALSE(t.IsRunning());
		EXPECT_EQ(t.GetElapsedTime(), 0);

		t.Start();

		EXPECT_TRUE(t.IsRunning());

		ZED::Core::Pause(60 * 1000);

		t.Pause();

		EXPECT_FALSE(t.IsRunning());

		int eps = t.GetElapsedTime() - 60 * 1000;
		//EXPECT_TRUE(-10 < eps && eps < 10);
		EXPECT_LE(-10, eps);
		EXPECT_LE(eps, 10);

		t.Start();

		ZED::Core::Pause(60 * 1000);

		eps = t.GetElapsedTime() - 2 * 60 * 1000;
		//EXPECT_TRUE(-10 < eps && eps < 10);
		EXPECT_LE(-20, eps);
		EXPECT_LE(eps, 20);

		t.Pause();

		eps = t.GetElapsedTime() - 2 * 60 * 1000;
		//EXPECT_TRUE(-10 < eps && eps < 10);
		EXPECT_LE(-20, eps);
		EXPECT_LE(eps, 20);

		t.Start();
		t.Reset();

		ZED::Core::Pause(10 * 1000);

		eps = t.GetElapsedTime() - 10 * 1000;
		//EXPECT_TRUE(-10 < eps && eps < 10);
		EXPECT_LE(-10, eps);
		EXPECT_LE(eps, 10);
	}
}