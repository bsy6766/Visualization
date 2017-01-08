#ifndef ENTITY_H
#define ENTITY_H

#include "cocos2d.h"
#include <vector>
#include "Component.h"

namespace ECS
{
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
		T getComponent(ECS::COMPONENT_ID componentEnum)
		{
			return dynamic_cast<T>(components[componentEnum]);
		}

		// 1000 by default. 
		static int maxEntitySize;
	};
}

#endif