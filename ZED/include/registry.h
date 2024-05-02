#pragma once
#include <functional>
#include "core.h"
#include "entity.h"



namespace ZED
{
	template<typename T>
	class SimpleIndex;
	class Index;


	class Registry
	{
	public:
		Entity CreateEntity();
		void   RemoveEntity();

		void AddIndex(Index* NewIndex);

	private:
		std::vector<Entity> m_Entities;//Entity database
		std::vector<Index*> m_Indicies;

		//Events
		//std::function<void(Entity)> m_OnEntity_Create;
		//std::function<void(Entity)> m_OnEntity_Delete;

		//std::function<void(Entity, Component&)> m_OnComponent_Create;
		//std::function<void(Entity, Component&)> m_OnComponent_Delete;
	};


	class Index
	{
	public:
		virtual void OnEntityCreate(Entity& Ent) = 0;
		virtual void OnEntityDelete(Entity& Ent) = 0;

		virtual void OnComponentCreate(Component& Comp) = 0;
		virtual void OnComponentDelete(Component& Comp) = 0;

	protected:
		Index() = default;

	private:
	};


	template<typename T>
	class SimpleIndex : public Index
	{
	public:
		void OnComponentCreate(Component& Comp) {
			m_Components.emplace_back(&Comp);
		}
		void OnComponentDelete(Component& Comp) {
			for (auto it = m_Components.begin(); it != m_Components.end(); ++it)
				if (*it == Comp)
				{
					m_Components.erase(it);
					return;
				}
		}

	private:
		std::vector<T*> m_Components;
	};
}