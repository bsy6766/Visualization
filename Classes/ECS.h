#ifndef ECS_H
#define ECS_H

#include <list>
#include <bitset>
#include <vector>

#define DEFAULT_MAX_ENTITY 2048
#define DEFAULT_MAX_COMPONENT 128
#define DEFAULT_MAX_SYSTEM 64

namespace ECS
{
	class Component;
	class System;
	class EntityManager;

	class Entity
	{
	public:
		Entity();
		~Entity();

		bool alive;

		static int idCounter;
		int id;

		std::vector<ECS::Component*> components;

		template<class T>
		T getComponent(int componentEnum)
		{
			return dynamic_cast<T>(components[componentEnum]);
		}

		// 2048 by default. 
		static int maxEntitySize;
	};

	class Component
	{
	private:
	public:
		Component(const int id);
		virtual ~Component() = default;
		Component(Component const&) = delete;
		void operator=(Component const&) = delete;

		int id;

		const int getId();
	};

	class System
	{
	public:
		System(const std::initializer_list<int>& ids);
		~System();

		std::bitset<DEFAULT_MAX_COMPONENT> usingComponentIds;
	};
}

#endif