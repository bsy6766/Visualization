#include "Entity.h"
#include "Utility.h"

using namespace ECS;

int Entity::idCounter = 0;

int Entity::maxEntitySize = 1000;

Entity::Entity() : alive(true)
{
	this->id = idCounter;
	Entity::idCounter++;

	this->components.resize(static_cast<int>(MAX_COMPONENT), nullptr);
}

Entity::~Entity()
{
	alive = false;
	id = -1;

	for (auto component : this->components)
	{
		if (component != nullptr)
		{
			delete component;
		}
	}

	this->components.clear();
}