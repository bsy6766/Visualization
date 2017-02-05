#include "ECS.h"
#include <cassert>
#include <string>

using namespace ECS;

std::unique_ptr<Manager, ECS::Deleter<Manager>> ECS::Manager::instance = nullptr;

ECS::Manager::Manager()
{
	// Create default entity pool
	this->entityPools.insert(std::pair<std::string, std::unique_ptr<ECS::Manager::EntityPool>>(ECS::DEFAULT_ENTITY_POOL_NAME, std::unique_ptr<ECS::Manager::EntityPool>(new ECS::Manager::EntityPool())));

	for (unsigned int i = 0; i < ECS::DEFAULT_ENTITY_POOL_SIZE; i++)
	{
		this->entityPools.begin()->second->pool.push_back(std::unique_ptr<ECS::Entity, ECS::Deleter<ECS::Entity>>(new ECS::Entity(), ECS::Deleter<ECS::Entity>()));
		this->entityPools.begin()->second->pool.back()->entityPoolName = ECS::DEFAULT_ENTITY_POOL_NAME;
		this->entityPools.begin()->second->pool.back()->index = i;
		this->entityPools.begin()->second->nextIndicies.push_back(i);
	}

	this->entityPools.begin()->second->idCounter = 0;
}

ECS::Manager::~Manager()
{
	// Unique ptr will release all for us
	this->entityPools.clear();
	this->components.clear();
	this->systems.clear();
	this->C_UNIQUE_IDMap.clear();
}

Manager* ECS::Manager::getInstance()
{
	if (ECS::Manager::instance == nullptr)
	{
		ECS::Manager::instance = std::unique_ptr<Manager, ECS::Deleter<Manager>>(new Manager(), ECS::Deleter<Manager>());
	}

	return ECS::Manager::instance.get();
}

void ECS::Manager::deleteInstance()
{
	if (ECS::Manager::instance != nullptr)
	{
		instance->clear();
		Manager* managerPtr = ECS::Manager::instance.release();
		ECS::Manager::instance = nullptr;
		if (managerPtr != nullptr)
		{
			delete managerPtr;
		}
		managerPtr = nullptr;
	}
}

bool ECS::Manager::isValid()
{
	return ECS::Manager::instance != nullptr;
}

void ECS::Manager::update(const float delta)
{
	// Main update.

	// Update system by priority.
	for (auto& system : this->systems)
	{
		if (system.second->active)
		{
			std::vector<ECS::Entity*> entities;
			for (auto poolName : system.second->entityPoolNames)
			{
				if (this->hasEntityPoolName(poolName))
				{
					if (poolName == ECS::DEFAULT_ENTITY_POOL_NAME)
					{
						if (system.second->queriesDefaultPool == false)
						{
							continue;
						}
					}

					for (auto& entity : this->entityPools[poolName]->pool)
					{
						if (entity->alive)
						{
							const Signature cmpSig = entity->signature & system.second->signature;
							if (cmpSig == system.second->signature)
							{
								entities.push_back(entity.get());
							}
						}
					}
				}
			}

			system.second->update(delta, entities);
		}
	}

}

void ECS::Manager::wrapEntityPoolIdCounter(const std::string& poolName)
{
	if (this->hasEntityPoolName(poolName))
	{
		if (this->entityPools.at(poolName)->idCounter >= ECS::MAX_E_ID)
		{
			this->entityPools.at(poolName)->idCounter = 0;
		}
	}
}

void ECS::Manager::wrapComponentUniqueIdCounter(const C_UNIQUE_ID cUniqueId)
{
	try
	{
		if (this->components.at(cUniqueId)->idCounter >= ECS::MAX_C_UNIQUE_ID)
		{
			this->components.at(cUniqueId)->idCounter = 0;
		}
	}
	catch (...)
	{
		return;
	}
}

const bool ECS::Manager::hasEntityPoolName(const std::string & name)
{
	// Check if there is a pool with same name.
	return this->entityPools.find(name) != this->entityPools.end();
}

const unsigned int ECS::Manager::getEntityPoolSize(const std::string & name)
{
	auto find_it = this->entityPools.find(name);
	if(find_it != this->entityPools.end())
	{
		return find_it->second->pool.size();
	}
	else
	{
		return 0;
	}
}

const unsigned int ECS::Manager::getAliveEntityCountInEntityPool(const std::string & name)
{
	unsigned int count = 0;
	auto find_it = this->entityPools.find(name);
	if (find_it != this->entityPools.end())
	{
		for (auto& e : find_it->second->pool)
		{
			if (e->alive)
			{
				count++;
			}
		}

		return count;
	}
	else
	{
		return 0;
	}
}

const bool ECS::Manager::isPowerOfTwo(const unsigned int n)
{
	return ((n != 0) && !(n & (n - 1)));
}

void ECS::Manager::roundToNearestPowerOfTwo(unsigned int& n)
{
	n = pow(2, ceil(log(n) / log(2)));
}

const bool ECS::Manager::createEntityPool(const std::string& name, const int maxSize)
{
	if (maxSize <= 0)
	{
		return false;
	}

	if (name.empty() || name == DEFAULT_ENTITY_POOL_NAME)
	{
		// Pool name can't be empty or default
		return false;
	}

	if (this->hasEntityPoolName(name))
	{
		// There is a pool with same name
		return false;
	}

	unsigned int poolSize = maxSize;

	if (this->isPowerOfTwo(poolSize) == false)
	{
		this->roundToNearestPowerOfTwo(poolSize);
	}

	// Create new
	ECS::Manager::EntityPool* newPool = new ECS::Manager::EntityPool();

	for (unsigned int i = 0; i < poolSize; i++)
	{
		newPool->pool.push_back(std::unique_ptr<ECS::Entity, ECS::Deleter<ECS::Entity>>(new ECS::Entity(), ECS::Deleter<ECS::Entity>()));
		newPool->nextIndicies.push_back(i);
		newPool->pool.back()->entityPoolName = name;
		newPool->pool.back()->index = i;
	}

	newPool->idCounter = 0;

	this->entityPools.insert(std::pair<std::string, std::unique_ptr<ECS::Manager::EntityPool>>(name, std::unique_ptr<ECS::Manager::EntityPool>(newPool)));
	return true;
}

const bool ECS::Manager::deleteEntityPool(const std::string& name)
{
	if (name.empty() || name == DEFAULT_ENTITY_POOL_NAME)
	{
		// Can't delete with empty pool name or default name
		return false;
	}

	auto it = this->entityPools.begin();
	for (; it != this->entityPools.end(); )
	{
		if ((*it).first == name)
		{
			this->entityPools.erase(it);
			return true;
		}
	}

	return false;
}

const bool ECS::Manager::resizeEntityPool(const std::string& entityPoolName, const unsigned int size)
{
	if (!this->hasEntityPoolName(entityPoolName))
	{
		return false;
	}

	unsigned int newSize = size;
	if (!this->isPowerOfTwo(newSize))
	{
		this->roundToNearestPowerOfTwo(newSize);
	}

	const unsigned int curSize = this->entityPools.at(entityPoolName)->pool.size();

	if (curSize < newSize)
	{
		// expand
		for (unsigned int i = curSize; i < newSize; i++)
		{
			this->entityPools.at(entityPoolName)->pool.push_back(std::unique_ptr<ECS::Entity, ECS::Deleter<ECS::Entity>>(new ECS::Entity(), ECS::Deleter<ECS::Entity>()));
			this->entityPools.at(entityPoolName)->pool.back()->entityPoolName = ECS::DEFAULT_ENTITY_POOL_NAME;
			this->entityPools.at(entityPoolName)->pool.back()->index = i;
			this->entityPools.at(entityPoolName)->nextIndicies.push_back(i);
		}

		assert(this->entityPools.at(entityPoolName)->pool.size() == newSize);
	}
	else if (curSize > newSize)
	{
		this->entityPools.at(entityPoolName)->nextIndicies.clear();
		this->entityPools.at(entityPoolName)->pool.resize(newSize);

		for (unsigned int i = 0; i < this->entityPools.at(entityPoolName)->pool.size(); i++)
		{
			if (this->entityPools.at(entityPoolName)->pool.at(i)->alive == false)
			{
				this->entityPools.at(entityPoolName)->nextIndicies.push_back(i);
			}
		}
	}

	return true;
}

ECS::Entity* ECS::Manager::createEntity(const std::string& poolName)
{
	if (poolName.empty())
	{
		// Pool name can't be empty
		return nullptr;
	}

	auto find_it = this->entityPools.find(poolName);

	if (find_it == this->entityPools.end())
	{
		return nullptr;
	}

	if (find_it->second->nextIndicies.empty())
	{
		// queue is empty. Pool is full
		return nullptr;
	}
	else
	{
		unsigned int index = find_it->second->nextIndicies.front();
		find_it->second->nextIndicies.pop_front();

		// Assume index is always valid.
		find_it->second->pool.at(index)->revive(find_it->second->idCounter);
		find_it->second->idCounter++;
		this->wrapEntityPoolIdCounter(poolName);
		return find_it->second->pool.at(index).get();
	}

	// failed to find pool
	return nullptr;
}

ECS::Entity* ECS::Manager::getEntityById(const E_ID entityId)
{
	if (entityId == ECS::INVALID_E_ID)
	{
		return nullptr;
	}

	// Iterate all EntityPool and find it
	for (auto& entityPool : this->entityPools)
	{
		for (auto& entity : entityPool.second->pool)
		{
			if (entity->id == entityId)
			{
				return entity.get();
			}
		}
	}

	// Entity not found
	return nullptr;
}

void ECS::Manager::getAllEntitiesInPool(std::vector<ECS::Entity*>& entities, const std::string & poolName)
{
	if (this->hasEntityPoolName(poolName))
	{
		for (auto& entity : this->entityPools[poolName]->pool)
		{
			if (entity->alive)
			{
				entities.push_back(entity.get());
			}
		}
	}
}

const bool ECS::Manager::moveEntityToEntityPool(ECS::Entity*& entity, const std::string& entityPoolName)
{
	if (entity == nullptr) return false;
	if (entity->id == ECS::INVALID_E_ID) return false;

	if (this->hasEntityPoolName(entityPoolName) && this->hasEntityPoolName(entity->entityPoolName))
	{
		ECS::Entity* target = this->createEntity(entityPoolName);
		if (target == nullptr)
		{
			return false;
		}

		for (auto& e : this->entityPools[entity->entityPoolName]->pool)
		{
			if (e->id == entity->id)
			{
				target->signature = e->signature;
				target->componentIndicies = e->componentIndicies;
				target->sleep = e->sleep;

				// Reassing owner for components
				for (auto pair : e->componentIndicies)
				{
					const C_UNIQUE_ID cUniqueId = pair.first;
					for (auto cIndex : pair.second)
					{
						this->components.at(cUniqueId)->pool.at(cIndex)->ownerId = target->id;
					}
				}

				e->signature = 0;
				e->alive = false;
				e->componentIndicies.clear();
				e->sleep = false;
				e->id = ECS::INVALID_E_ID;
				this->entityPools[e->entityPoolName]->nextIndicies.push_front(e->index);

				entity = nullptr;

				entity = target;

				return true;
			}
		}
	}

	return false;
}

const bool ECS::Manager::killEntity(ECS::Entity* e)
{
	if (e == nullptr) return false;
	if (e->id == ECS::INVALID_E_ID) return false;
	if (e->entityPoolName.empty()) return false;

	if (this->hasEntityPoolName(e->entityPoolName))
	{
		assert(e->index != ECS::MAX_E_ID);

		// Wipe everything
		for (auto componentIndicies : this->entityPools[e->entityPoolName]->pool.at(e->index)->componentIndicies)
		{
			const C_UNIQUE_ID cUnqiueId = componentIndicies.first;
			for (auto cIndex : componentIndicies.second)
			{
				if (this->components.at(cUnqiueId)->pool.at(cIndex) != nullptr)
				{
					if (this->components.at(cUnqiueId)->pool.at(cIndex)->ownerId == e->id)
					{
						Component* c = this->components.at(cUnqiueId)->pool.at(cIndex).release();
						this->components.at(cUnqiueId)->pool.at(cIndex) = nullptr;
						this->components.at(cUnqiueId)->nextIndicies.push_front(cIndex);
						delete c;
						c = nullptr;
					}
				}
			}
		}

		this->entityPools[e->entityPoolName]->nextIndicies.push_front(e->index);
		this->entityPools[e->entityPoolName]->pool.at(e->index)->componentIndicies.clear();
		this->entityPools[e->entityPoolName]->pool.at(e->index)->id = ECS::INVALID_E_ID;
		this->entityPools[e->entityPoolName]->pool.at(e->index)->alive = false;
		this->entityPools[e->entityPoolName]->pool.at(e->index)->sleep = false;
		this->entityPools[e->entityPoolName]->pool.at(e->index)->signature = 0;

		return true;
	}
	else
	{
		return false;
	}
}

const C_ID ECS::Manager::getComponentUniqueId(const std::type_info& t)
{
	const std::type_index typeIndex = std::type_index(t);
	try
	{
		return this->C_UNIQUE_IDMap.at(typeIndex);
	}
	catch (const std::out_of_range& oor)
	{
		return ECS::INVALID_C_UNIQUE_ID;
	}
}

const S_ID ECS::Manager::getSystemId(const std::type_info & t)
{
	const std::type_index typeIndex = std::type_index(t);
	auto find_it = this->S_IDMap.find(typeIndex);
	if (find_it != this->S_IDMap.end())
	{
		return find_it->second;
	}
	else
	{
		return INVALID_S_ID;
	}
}

const C_UNIQUE_ID ECS::Manager::registerComponent(const std::type_info& t)
{
	C_UNIQUE_ID cUniqueID = this->getComponentUniqueId(t);
	if (cUniqueID == ECS::INVALID_C_UNIQUE_ID)
	{
		// This is new type of component.
		cUniqueID = Component::uniqueIdCounter++;
		if (cUniqueID == ECS::MAX_C_UNIQUE_ID)
		{
			// Reached maximum number of component type. 
			Component::uniqueIdCounter--;
			return ECS::INVALID_C_UNIQUE_ID;
		}

		// Add new unique id with type_index key to map
		this->C_UNIQUE_IDMap.insert(std::pair<std::type_index, C_UNIQUE_ID>(std::type_index(t), cUniqueID));

		// Add new component pool
		auto newPool = new ECS::Manager::ComponentPool();
		newPool->idCounter = 0;
		newPool->nextIndicies.clear();
		newPool->pool.clear();
		this->components.push_back(std::unique_ptr<ECS::Manager::ComponentPool>(std::unique_ptr<ECS::Manager::ComponentPool>(newPool)));
	}

	return cUniqueID;
}

const bool ECS::Manager::deleteComponent(Component*& c, const std::type_info& t)
{
	// Check component
	if (c == nullptr)
	{
		return false;
	}

	if (c->uniqueId == ECS::INVALID_C_UNIQUE_ID)
	{
		// Unique id is invalid
		delete c;
		c = nullptr;
		return true;
	}
	else
	{
		if (c->ownerId == ECS::INVALID_E_ID)
		{
			// Doesn't have owner
			delete c;
			c = nullptr;
			return true;
		}
		else
		{
			// Get owner
			ECS::Entity* e = this->getEntityById(c->ownerId);
			if (e == nullptr)
			{
				// Owner doesn't exists
				delete c;
				c = nullptr;
				return true;
			}
			else
			{
				// have owner. Check if manager has this component
				C_INDEX index = ECS::INVALID_C_INDEX;
				for (auto& component : this->components[c->uniqueId]->pool)
				{
					if (c == component.get())
					{
						// save index
						index = c->index;

						// Remove from signature
						e->signature[c->uniqueId] = 0;
						// Remove from indcies vector
						e->componentIndicies[c->uniqueId].erase(index);
						
						// Delete the component
						delete component.release();

						// Make it null
						component = nullptr;

						c = nullptr;
						return true;
					}
				}

				if (index == ECS::INVALID_C_INDEX)
				{
					// Failed to find same component
					delete c;
					c = nullptr;
					return true;
				}

				return true;
			}
		}
	}
}

const bool ECS::Manager::hasComponent(Entity* e, const std::type_info& t)
{
	if (e == nullptr) return false;

	const ECS::C_UNIQUE_ID cUniqueId = this->getComponentUniqueId(t);

	if (cUniqueId == ECS::INVALID_C_UNIQUE_ID)
	{
		// This type of component is unknown.
		return false;
	}
	else
	{
		Signature& s = e->signature;

		try
		{
			s.test(cUniqueId);
			if (s[cUniqueId] == true)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		catch (...)
		{
			return false;
		}
	}
}

const bool ECS::Manager::hasComponent(Entity* e, const std::type_info& t, Component* c)
{
	if (e == nullptr) return false;
	if (c == nullptr) return false;
	if (c->id == ECS::INVALID_C_ID) return false;
	if (c->index == ECS::INVALID_C_INDEX) return false;
	if (c->uniqueId != this->getComponentUniqueId(t)) return false;

	if (hasComponent(e, t))
	{
		for (auto cIndex : e->componentIndicies[this->getComponentUniqueId(t)])
		{
			if (c->index == cIndex)
			{
				return true;
			}
		}

		return false;
	}
	else
	{
		return false;
	}
}

Component* ECS::Manager::getComponent(Entity* e, const std::type_info& t)
{
	if (e == nullptr) return nullptr;

	// Get unique id
	const ECS::C_UNIQUE_ID cUniqueId = this->getComponentUniqueId(t);
	if (cUniqueId == ECS::INVALID_C_UNIQUE_ID)
	{
		// This type of component is unknown.
		return nullptr;
	}
	else
	{
		// This type of component is already known. We have unique id and pool ready.
		std::unordered_set<C_INDEX>& cIndicies = e->componentIndicies.at(cUniqueId);
		// Get C_INDEXs
		//e->getComponentIndiicesByUniqueId(cUniqueId, cIndicies);

		if (cIndicies.empty())
		{
			// Doesn't have any. hmm..
			return nullptr;
		}
		else
		{
			// Because this is getComponent not getComponents, return first one
			const C_INDEX firstIndex = (*cIndicies.begin());

			Component* c = this->components[cUniqueId]->pool.at(firstIndex).get();
			if (c->ownerId == e->id)
			{
				return c;
			}
			else
			{
				return nullptr;
			}
		}
	}
}

std::vector<Component*> ECS::Manager::getComponents(Entity* e, const std::type_info& t)
{
	std::vector<Component*> ret;

	if (e == nullptr) return ret;

	const ECS::C_UNIQUE_ID cUniqueId = this->getComponentUniqueId(t);
	if (cUniqueId == ECS::INVALID_C_UNIQUE_ID)
	{
		// This type of component is unknown.
		return ret;
	}
	else
	{
		// This type of component is already known. We have unique id and pool ready.
		std::unordered_set<C_INDEX> cIndicies;
		// Get C_INDEXs
		e->getComponentIndiicesByUniqueId(cUniqueId, cIndicies);

		if (cIndicies.empty())
		{
			// Doesn't have any. hmm..
			return ret;
		}
		else
		{
			// Return all components
			for (auto& component : this->components.at(cUniqueId)->pool)
			{
				if (component != nullptr)
				{
					ret.push_back(component.get());
				}
			}

			return ret;
		}
	}
}

const bool ECS::Manager::addComponent(Entity* e, const std::type_info& t, Component* c)
{
	if (e == nullptr) return false;

	// Reject null component
	if (c == nullptr) return false;

	// Get component unique id
	C_UNIQUE_ID cUniqueId = c->uniqueId;
	if (cUniqueId == ECS::INVALID_C_UNIQUE_ID)
	{
		// Bad component type
		return false;
	}

	// Check if manager already has the same entity. Reject if there is same one
	for (auto& component : this->components.at(cUniqueId)->pool)
	{
		if (component != nullptr)
		{
			if (component->getId() == c->getId())
			{
				return false;
			}
		}
	}

	// Component is valid.
	// Add component
	this->components.at(cUniqueId)->pool.push_back(std::unique_ptr<Component, Deleter<Component>>(c, Deleter<Component>()));

	// Get index. pool size - 1
	const C_INDEX cIndex = this->components.at(cUniqueId)->pool.size() - 1;

	// Add index to entity
	e->componentIndicies[cUniqueId].insert(cIndex);

	// update signature
	e->signature[cUniqueId] = 1;

	// Update component indicies
	auto find_it = e->componentIndicies.find(cUniqueId);
	if (find_it == e->componentIndicies.end())
	{
		e->componentIndicies.insert(std::pair<C_UNIQUE_ID, std::unordered_set<C_INDEX>>(cUniqueId, std::unordered_set<C_INDEX>()));
	}
	e->componentIndicies[cUniqueId].insert(cIndex);

	// Assign unique id
	c->uniqueId = cUniqueId;
	// Assign id
	c->id = this->components.at(cUniqueId)->idCounter++;
	// Store index
	c->index = cIndex;
	// Set owner id
	c->ownerId = e->getId();
	// wrap counter 
	this->wrapComponentUniqueIdCounter(cUniqueId);

	return true;
}

const bool ECS::Manager::removeComponent(Entity* e, const std::type_info& t, const C_ID componentId)
{
	if (e == nullptr) return false;

	// Reject null component
	if (componentId == ECS::INVALID_C_ID) return false;

	// Get component unique id
	C_UNIQUE_ID cUniqueId = this->getComponentUniqueId(t);
	if (cUniqueId == ECS::INVALID_C_UNIQUE_ID)
	{
		// Bad component type
		return false;
	}

	// Find component
	unsigned int index = 0;
	for (auto& component : this->components[cUniqueId]->pool)
	{
		if (component == nullptr)
		{
			continue;
		}

		if (component->id == componentId)
		{
			// Found the component. 
			if (component->ownerId == e->id)
			{
				// Owner match
				Component* c = component.release();
				component = nullptr;
				this->components[cUniqueId]->nextIndicies.push_front(index);
				delete c;
				e->componentIndicies.at(cUniqueId).erase(index);

				if (e->componentIndicies.at(cUniqueId).empty())
				{
					e->signature[cUniqueId] = 0;
				}

				return true;
			}
			else
			{
				return false;
			}
		}

		index++;
	}

	return false;
}

const bool ECS::Manager::removeComponent(Entity* e, const std::type_info& t, Component* c)
{
	if (e == nullptr) return false;

	if (c == nullptr) return false;
	if (c->id == ECS::INVALID_C_ID) return false;
	if (c->index == ECS::INVALID_C_INDEX) return false;
	if (c->uniqueId != this->getComponentUniqueId(t)) return false;
	if (c->ownerId != e->id) return false;

	// Get component unique id
	C_UNIQUE_ID cUniqueId = c->uniqueId;

	if (e->signature[cUniqueId] == 0)
	{
		return false;
	}

	if (this->components[cUniqueId]->pool.at(c->index) != nullptr)
	{
		auto comp = this->components[cUniqueId]->pool.at(c->index).release();
		this->components[cUniqueId]->pool.at(c->index) = nullptr;
		delete comp;
		this->components[cUniqueId]->nextIndicies.push_front(c->index);
		e->componentIndicies.at(cUniqueId).erase(c->index);

		if (e->componentIndicies.at(cUniqueId).empty())
		{
			e->signature[cUniqueId] = 0;
		}
	}
	else
	{
		return false;
	}

	return true;
}

const bool ECS::Manager::removeComponents(Entity* e, const std::type_info& t)
{
	if (e == nullptr) return false;

	// Get component unique id
	C_UNIQUE_ID cUniqueId = this->getComponentUniqueId(t);
	if (cUniqueId == ECS::INVALID_C_UNIQUE_ID)
	{
		// Bad component type
		return false;
	}

	if (e->signature[cUniqueId] == 0)
	{
		return false;
	}

	std::unordered_set<C_INDEX> cIndicies;
	e->getComponentIndiicesByUniqueId(cUniqueId, cIndicies);

	if (cIndicies.empty())
	{
		return false;
	}
	else
	{
		for (C_INDEX index : cIndicies)
		{
			if (this->components[cUniqueId]->pool.at(index) != nullptr)
			{
				Component* c = this->components[cUniqueId]->pool.at(index).release();
				this->components[cUniqueId]->pool.at(index) = nullptr;
				delete c;
			}
		}

		e->signature[cUniqueId] = 0;
		e->componentIndicies.erase(cUniqueId);
	}

	return true;
}

const S_ID ECS::Manager::registerSystem(const std::type_info & t)
{
	S_ID systemId = this->getSystemId(t);
	if (systemId == ECS::INVALID_S_ID)
	{
		systemId = System::idCounter++;
		if (systemId == ECS::MAX_S_ID)
		{
			System::idCounter--;
			return ECS::INVALID_S_ID;
		}

		this->S_IDMap.insert(std::pair<std::type_index, S_ID>(std::type_index(t), systemId));
	}
	else
	{
		// Reject same system going in
		for (auto& system : this->systems)
		{
			if (system.second->getId() == systemId)
			{
				return ECS::INVALID_S_ID;
			}
		}
	}

	return systemId;
}

const bool ECS::Manager::deleteSystem(System*& s, const std::type_info& t)
{
	if (s == nullptr)
	{
		return false;
	}

	if (s->getId() == ECS::INVALID_S_ID)
	{
		delete s;
		s = nullptr;
		return true;
	}
	else
	{
		auto it = this->systems.begin();
		for (; it != this->systems.end(); )
		{ 
			if ((*it).second->getId() == s->getId())
			{
				// found
				this->systems.erase(it);
				s = nullptr;
				return true;
			}
		}

		// not found
		delete s;
		s = nullptr;
		return true;
	}

	return false;
}

const bool ECS::Manager::hasSystem(const std::type_info& t)
{
    const S_ID sId = this->getSystemId(t);
    
    for (auto& system : this->systems)
    {
        if (system.second->getId() == sId)
        {
            return true;
        }
    }
    
    return false;
}

const bool ECS::Manager::hasSystem(ECS::System *s, const std::type_info &t)
{
    if (s == nullptr || s->getId() == ECS::INVALID_S_ID)
    {
        return false;
    }
    
    for (auto& system : this->systems)
    {
        if(system.second->getId() == s->getId())
        {
            return true;
        }
    }
    
    return false;
}

ECS::System* ECS::Manager::getSystem(const std::type_info &t)
{
    const S_ID sId = this->getSystemId(t);
    for(auto& system :  this->systems)
    {
        if(system.second->getId() == sId)
        {
            return system.second.get();
        }
    }
    
    return nullptr;
}

std::map<int, S_ID> ECS::Manager::getSystemUpdateOrder()
{
	std::map<int, S_ID> order;

	for (auto& system : this->systems)
	{
		order.insert(std::pair<int, S_ID>(system.first, system.second->getId()));
	}

	return order;
}

void ECS::Manager::clear()
{
	ECS::Manager::EntityPool* defaultPool = this->entityPools.at(ECS::DEFAULT_ENTITY_POOL_NAME).release();
	this->entityPools.clear();

	defaultPool->nextIndicies.clear();

	const unsigned int size = defaultPool->pool.size();

	for (unsigned int i = 0; i < size; i++)
	{
		defaultPool->pool.at(i)->id = ECS::INVALID_E_ID;
		defaultPool->pool.at(i)->componentIndicies.clear();
		defaultPool->pool.at(i)->signature = 0;
		defaultPool->pool.at(i)->index = i;
		defaultPool->pool.at(i)->alive = false;
		defaultPool->nextIndicies.push_back(i);
	}

	defaultPool->idCounter = 0;

	this->entityPools.insert(std::pair<std::string, std::unique_ptr<ECS::Manager::EntityPool>>(ECS::DEFAULT_ENTITY_POOL_NAME, std::unique_ptr<ECS::Manager::EntityPool>(defaultPool)));

	this->components.clear();
	this->C_UNIQUE_IDMap.clear();

	this->systems.clear();
	this->S_IDMap.clear();

	System::idCounter = 0;
	Component::uniqueIdCounter = 0;
}

void ECS::Manager::printComponentsInfo()
{
	std::cout << std::endl;
	std::cout << "ECS::Printing Components informations" << std::endl;
	std::cout << "Total Component types: " << this->C_UNIQUE_IDMap.size() << std::endl;
	std::cout << "Types -------------------------------" << std::endl;

	for (auto type : this->C_UNIQUE_IDMap)
	{
		const unsigned int size = this->components.at(type.second)->pool.size();
		int aliveCount = 0;
		for (auto& c : this->components.at(type.second)->pool)
		{
			if (c != nullptr)
			{
				aliveCount++;
			}
		}

		std::cout << "Name: " << type.first.name() << ", Unique ID: " << type.second << ", Count/Size: " << aliveCount << "/" << size << std::endl;
		std::cout << "-- Component details. Unique ID: " << type.second << " --" << std::endl;
		for (unsigned int i = 0; i < size; i++)
		{
			if (this->components.at(type.second)->pool.at(i) == nullptr)
			{
				std::cout << "__EMPTY__" << std::endl;
			}
			else
			{
				std::cout << "ID: " << this->components.at(type.second)->pool.at(i)->getId() << ", Owner ID = " << this->components.at(type.second)->pool.at(i)->ownerId << std::endl;
			}
		}
		std::cout << "-------------------------------------" << std::endl;
	}

	std::cout << "-------------------------------------" << std::endl;
	std::cout << std::endl;
}

//============================================================================================

ECS::Entity::Entity()
: alive(false)
, sleep(false)
, id(ECS::INVALID_E_ID)
, index(-1)
, signature(0)
, entityPoolName(std::string())
{}

void ECS::Entity::getComponentIndiicesByUniqueId(const C_UNIQUE_ID cUniqueId, std::unordered_set<C_INDEX>& cIndicies)
{
	try
	{
		//for (auto index : this->componentIndicies.at(cUniqueId))
		//{
		//	cIndicies.insert(index);
		//}
		cIndicies = this->componentIndicies.at(cUniqueId);
	}
	catch (const std::out_of_range& oor)
	{
		// Doesn't exist. return.
		return;
	}
}

void ECS::Entity::revive(const E_ID newId)
{
	this->alive = true;
	this->sleep = false;
	this->id = newId;
}

void ECS::Entity::kill()
{
	ECS::Manager* m = Manager::getInstance();
	m->killEntity(this);
}

const E_ID ECS::Entity::getId()
{
	return this->id;
}

const std::string ECS::Entity::getEntityPoolName()
{
	return this->entityPoolName;
}

const bool ECS::Entity::isAlive()
{
	return this->alive;
}

const Signature ECS::Entity::getSignature()
{
	return this->signature;
}

//============================================================================================

C_ID ECS::Component::uniqueIdCounter = 0;

Component::Component() 
: id(INVALID_C_ID) 
, index(INVALID_C_INDEX)
, uniqueId(INVALID_C_UNIQUE_ID)
, ownerId(INVALID_E_ID)
{}

void ECS::Component::wrapUniqueIdCounter()
{
	if (Component::uniqueIdCounter >= INVALID_C_UNIQUE_ID)
	{
		Component::uniqueIdCounter = 0;
	}
}

const C_ID ECS::Component::getId()
{
	return this->id;
}

const C_UNIQUE_ID ECS::Component::getUniqueId()
{
	return this->uniqueId;
}

const E_ID ECS::Component::getOwnerId()
{
	return this->ownerId;
}

//============================================================================================

S_ID ECS::System::idCounter = 0;

ECS::System::System(const int priority)
: id(ECS::INVALID_S_ID)
, signature(0)
, entityPoolNames({ ECS::DEFAULT_ENTITY_POOL_NAME })
, priority(priority)
, active(true)
, queriesDefaultPool(true)
{}

ECS::System::System(const int priority, std::initializer_list<C_UNIQUE_ID> componentUniqueIds, std::initializer_list<std::string> entityPoolNames)
: id(ECS::INVALID_S_ID)
, signature(0)
, entityPoolNames(entityPoolNames)
, priority(priority)
, active(true)
, queriesDefaultPool(true)
{
	for (auto cId : componentUniqueIds)
	{
		try
		{
			signature.test(cId);
			signature[cId] = 1;
		}
		catch (const std::out_of_range& oor)
		{
			continue;
		}
	}

	for (auto poolName : this->entityPoolNames)
	{
		if (poolName == ECS::DEFAULT_ENTITY_POOL_NAME)
		{
			return;
		}
	}

	this->entityPoolNames.push_back(ECS::DEFAULT_ENTITY_POOL_NAME);
}

const S_ID ECS::System::getId()
{
	return this->id;
}

const Signature ECS::System::getSignature()
{
	return this->signature;
}

const int ECS::System::getPriority()
{
	return this->priority;
}

void ECS::System::disbaleDefafultEntityPool()
{
	this->queriesDefaultPool = false;
}

void ECS::System::enableDefaultEntityPool()
{
	this->queriesDefaultPool = true;
}

const bool ECS::System::addEntityPoolName(const std::string& entityPoolName)
{
	auto find_it = std::find(this->entityPoolNames.begin(), this->entityPoolNames.end(), entityPoolName);
	if (find_it == this->entityPoolNames.end())
	{
		this->entityPoolNames.push_back(entityPoolName);
		return true;
	}
	else
	{
		return false;
	}
}

const bool ECS::System::removeEntityPoolName(const std::string & entityPoolName)
{
	auto it = this->entityPoolNames.begin();
	for (; it != this->entityPoolNames.end();)
	{
		if ((*it) == entityPoolName)
		{
			this->entityPoolNames.erase(it);
			return true;
		}
	}

	return false;
}

void ECS::System::deactivate()
{
	this->active = false;
}

void ECS::System::activate()
{
	this->active = true;
}

const bool ECS::System::isActive()
{
	return this->active;
}
