#include "tests.h"



TEST(ID, CheckUniqueness)
{
	initialize();

	const int size = 5000;
	ID* ids[size] = {};

	for (int i = 0; i < size; i++)
	{
		ids[i] = new ID;
	}

	for (int i = 0; i < size; i++)
		for (int j = i+1; j < size; j++)
	{
		ASSERT_NE(ids[i]->GetID(),   ids[j]->GetID());
		ASSERT_NE(ids[i]->GetUUID(), ids[j]->GetUUID());
	}

	for (int i = 0; i < size; i++)
		delete ids[i];
}