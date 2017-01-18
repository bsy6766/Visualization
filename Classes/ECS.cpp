#include "ECS.h"
#include "Utility.h"

using namespace ECS;

int Entity::idCounter = 0;

int Entity::maxEntitySize = DEFAULT_MAX_ENTITY;

Entity::Entity() : alive(true)
{
	this->id = idCounter;
	Entity::idCounter++;

	this->components.resize(static_cast<int>(DEFAULT_MAX_COMPONENT), nullptr);
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

Component::Component(const int id) : id(id) {}

const int Component::getId()
{
	return this->id;
}


System::System(const std::initializer_list<int>& ids)
{
	for (auto id : ids)
	{
		this->usingComponentIds[id] = 1;
	}
}

System::~System()
{
	this->usingComponentIds.reset();
}