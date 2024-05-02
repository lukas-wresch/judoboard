#pragma once
#include "core.h"



namespace ZED
{
	class Component;
	class Registry;


	class Entity
	{
	public:
		

	private:
		uint64_t m_ID = 0;//Invalid id

		std::vector<Component*> m_Components;

		const Registry* m_pRegistry;
	};
}