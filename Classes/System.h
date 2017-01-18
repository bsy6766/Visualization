#ifndef SYSTEM_H
#define SYSTEM_H

#include <bitset>
#include "Component.h"
#include "ECS.h"
#include <list>

/**
*	Note
*
*	Since this project is just for visualization, I declared everything
*	in public field to make easier to access from outside. Not considering
*	any encapsulation or whatnot.
*
*	System for this project is kind of useless because it doesn't 
*/

namespace ECS
{
	class FlockingSystem : public System
	{
	public:
		FlockingSystem();
		~FlockingSystem();

		std::list<ECS::Entity*> boids;

		void update(const float delta);
	};
}

#endif