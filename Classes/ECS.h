#ifndef ECS_H
#define ECS_H

// containers
#include <vector>
#include <list>
#include <deque>
#include <unordered_map>
#include <unordered_set>	
#include <map>	
// Util
#include <memory>				// unique_ptr
#include <functional>			// Error handling
#include <bitset>				// Signature
#include <limits>
#include <stdexcept>
#include <cmath>				// power of 2
#include <iostream>
#include <initializer_list>
#include <chrono>
// Component
#include <typeinfo>
#include <typeindex>

namespace ECS
{
	// Classes
	class Entity;
	class Component;
	class System;
	class Manager;

	// Const
	// Maximum number of component id.
	const unsigned int MAX_C_ID = std::numeric_limits<unsigned int>::max();
	// Invalid component id
	const unsigned int INVALID_C_ID = MAX_C_ID;

	// Maximum number of component index
	const unsigned int MAX_C_INDEX = std::numeric_limits<unsigned int>::max();
	// Invalid component index
	const unsigned int INVALID_C_INDEX = MAX_C_INDEX;

	// Maximum number of component unique id
	const unsigned int MAX_C_UNIQUE_ID = 256;
	// Invalid component unique id
	const unsigned int INVALID_C_UNIQUE_ID = MAX_C_UNIQUE_ID;

	// Maximum number of entity id
	const unsigned long MAX_E_ID = std::numeric_limits<unsigned long>::max();
	// Invalid entity id
	const unsigned long INVALID_E_ID = MAX_E_ID;

	// maximum number of system id
	const unsigned int MAX_S_ID = std::numeric_limits<unsigned int>::max();
	// Invalid component id
	const unsigned int INVALID_S_ID = MAX_S_ID;

	// Default pool string name
	const std::string DEFAULT_ENTITY_POOL_NAME = "DEFAULT";
	// Default EntityPool size
	const unsigned int DEFAULT_ENTITY_POOL_SIZE = 2048;
	// Default ComponentPool size
	const unsigned int DEFAULT_COMPONENT_POOL_SIZE = 4096;

	// Typedefs
	typedef unsigned long E_ID;							// Entity ID. 
	typedef unsigned int E_INDEX;						// Entity Index
	typedef unsigned int C_ID;							// Component ID
	typedef unsigned int C_UNIQUE_ID;					// Component unique ID
	typedef unsigned int C_INDEX;						// Component Index
	typedef unsigned int S_ID;							// System id
	typedef std::bitset<MAX_C_UNIQUE_ID> Signature;

	// Custom deleter for unique_ptr.
	// By making this, user can't call destructor(delete) on any instances.
	template<class T> class Deleter
	{
		friend std::unique_ptr<ECS::Manager, Deleter>;
		friend std::unique_ptr<ECS::Entity, Deleter>;
		friend std::unique_ptr<ECS::Component, Deleter>;
		friend std::unique_ptr<ECS::System, Deleter>;
	private:
		void operator()(T* t) { delete t; }
	};

	/**
	*	@class Component
	*	@brief Base class for all components.
	*	@note Derive this class to create component.
	*
	*	Component class is base class for all component classes you derive.
	*	Unlike Entity or EntityPool, Components are created on user side rather than in manager class.
	*	Therefore, there are no way that manager can tell which components exists untill they see one.
	*	New derived components won't have unique id until they get added to entity.
	*/
	class Component
	{
		friend class Manager;
		friend class Deleter<Component>;

		friend bool operator==(const Component& a, const Component& b)
		{
			return ((a.id == b.id) &&
					(a.uniqueId == b.uniqueId) &&
					(a.index == b.index) &&
					(a.ownerId == b.ownerId));
		}		
		friend bool operator!=(const Component& a, const Component& b)
		{
			return ((a.id != b.id) ||
				(a.uniqueId != b.uniqueId) ||
				(a.index != b.index) ||
				(a.ownerId != b.ownerId));
		}
	protected:
		// Protected constructor. Can't create bass class from out side
		Component();

		void* operator new(size_t sz) throw (std::bad_alloc)
		{
			void* mem = std::malloc(sz);
			if (mem)
				return mem;
			else
				throw std::bad_alloc();
		}
		void operator delete(void* ptr) throw()
		{
			std::free(ptr);
		}
	private:
		// Component Id
		C_ID id;
		// Index 
		C_INDEX index;
		// counter
		static C_UNIQUE_ID uniqueIdCounter;
		// ID counter. Starts from 0
		static void wrapUniqueIdCounter();
		// Unique id number of this component type
		C_UNIQUE_ID uniqueId;
		// Owner of this component
		E_ID ownerId;
	public:
		// Public virtual destructor. Default.
		virtual ~Component() = default;

		// Disable all other constructors.
		Component(const Component& arg) = delete;					// Copy constructor
		Component(const Component&& arg) = delete;					// Move constructor
		Component& operator=(const Component& arg) = delete;		// Assignment operator
		Component& operator=(const Component&& arg) = delete;		// Move operator

		// Get component Id
		const C_ID getId();
		// Get unique id
		const C_UNIQUE_ID getUniqueId();
		// Get owner entity id of this component
		const E_ID getOwnerId();
	};

	class System
	{
		friend class Manager;
		friend class Deleter<System>;
	protected:
		/**
		*	@name System
		*	@brief Default constructor
		*/
		System(const int priority);
		/**
		*	@name System
		*	@brief Constructor for system.
		*	@param componentUniqueIds Component unique ids that this system uses.
		*	@param entityPoolNames Entity Pool names that this system will query. Automatically queries default pool. @see disableDefaultEntityPool to disable.
		*/
		System(const int priority, std::initializer_list<C_UNIQUE_ID> componentUniqueIds, std::initializer_list<std::string> entityPoolNames);

		void* operator new(size_t sz) throw (std::bad_alloc)
		{
			void* mem = std::malloc(sz);
			if (mem)
				return mem;
			else
				throw std::bad_alloc();
		}
		void operator delete(void* ptr) throw()
		{
			std::free(ptr);
		}
	private:
		static S_ID idCounter;
		S_ID id;
		Signature signature;
		bool queriesDefaultPool;
		std::list<std::string> entityPoolNames;
		int priority;
		bool active;
	public:
		virtual ~System() = default;

		const S_ID getId();
		const Signature getSignature();
		const int getPriority();

		void disbaleDefafultEntityPool();
		void enableDefaultEntityPool();

		const bool addEntityPoolName(const std::string& entityPoolName);
		const bool removeEntityPoolName(const std::string& entityPoolName);

		template<class T> const bool addComponentType()
		{
			auto m = ECS::Manager::getInstance();
			auto cUniqueId = m->getComponentUniqueId(typeid(T));
			try
			{
				this->signature.test(cUniqueId);
				this->signature[cUniqueId] = 1;
				return true;
			}
			catch (const std::out_of_range& oor)
			{
				return false;
			}
		}
		template<class T> const bool removeComponentType()
		{
			auto m = ECS::Manager::getInstance();
			auto cUniqueId = m->getComponentUniqueId(typeid(T));
			try
			{
				this->signature.test(cUniqueId);
				this->signature[cUniqueId] = 0;
				return true;
			}
			catch (const std::out_of_range& oor)
			{
				return false;
			}
		}

		void deactivate();
		void activate();
		const bool isActive();

		virtual void update(const float delta, std::vector<ECS::Entity*>& entities) = 0;
	};
    
	/**
	*	@class Manager
	*	@brief The manager class that manages entire ECS.
	*	@note This is singleton class. The instance automtically released on end of program.
	*/
	class Manager
	{
		friend class Deleter<Manager>;
		friend class System;
	private:
		// Private constructor. Call getInstance for access.
		Manager();
		// Private constructor. Call deleteInstance to delete instance.
		~Manager();
		// Disable all other constructors.
		Manager(const Manager& arg) = delete;						// Copy constructor
		Manager(const Manager&& arg) = delete;						// Move constructor
		Manager& operator=(const Manager& arg) = delete;			// Assignment operator
		Manager& operator=(const Manager&& arg) = delete;			// Move operator

		// Singleton instance
		static std::unique_ptr<Manager, ECS::Deleter<Manager>> instance;

		// ==================================== ENTITY ====================================
		struct EntityPool
		{
			std::vector<std::unique_ptr<ECS::Entity, ECS::Deleter<ECS::Entity>>> pool;
			std::deque<unsigned int> nextIndicies;
			int idCounter;
		};
		// Entities
		std::unordered_map<std::string, std::unique_ptr<EntityPool>> entityPools;
		// wraps id counter if reaches max
		void wrapEntityPoolIdCounter(const std::string& poolName);
		// ================================================================================

		// =================================== COMPONENT ==================================
		struct ComponentPool
		{
			std::vector<std::unique_ptr<ECS::Component, ECS::Deleter<ECS::Component>>> pool;
			std::deque<unsigned int> nextIndicies;
			unsigned int idCounter;
		};
		// Components
		//std::unordered_map<C_UNIQUE_ID, std::unique_ptr<ComponentPool>> components;
		std::vector<std::unique_ptr<ComponentPool>> components;
		// Component ID Map. class type_index <---> CID
		std::unordered_map<std::type_index, C_UNIQUE_ID> C_UNIQUE_IDMap;
		// Wraps id counter if reaches max
		void wrapComponentUniqueIdCounter(const C_UNIQUE_ID cUniqueId);

		const C_UNIQUE_ID getComponentUniqueId(const std::type_info& t);
		const C_UNIQUE_ID registerComponent(const std::type_info& t);
		const bool deleteComponent(Component*& c, const std::type_info& t);
		const bool hasComponent(Entity* e, const std::type_info& t);
		const bool hasComponent(Entity* e, const std::type_info& t, Component* c);
		Component* getComponent(Entity* e, const std::type_info& t);
		std::vector<Component*> getComponents(Entity* e, const std::type_info& t);
		const bool addComponent(Entity* e, const std::type_info& t, Component* c);
		const bool removeComponent(Entity* e, const std::type_info& t, const C_ID componentId);
		const bool removeComponent(Entity* e, const std::type_info& t, Component* c);
		const bool removeComponents(Entity* e, const std::type_info& t);
		// ================================================================================
		
		// ==================================== SYSTEM ====================================
		std::map<int/*priority*/, std::unique_ptr<ECS::System, ECS::Deleter<ECS::System>>> systems;
		// System ID Map. class type_index <---> SID
		std::unordered_map<std::type_index, S_ID> S_IDMap;
		// Get SID from type info
		const S_ID getSystemId(const std::type_info& t);

		const S_ID registerSystem(const std::type_info& t);
		const bool deleteSystem(System*& s, const std::type_info& t);
		// ================================================================================
	public:
		// Get instance.
		static Manager* getInstance();
		// Delete instance.
		static void deleteInstance();
		// Check if manager is valid
		static bool isValid();
		// Update function. Call this every tick.
		void update(const float delta);

		// Creates new EntityPool
		const bool createEntityPool(const std::string& name, const int maxSize = ECS::DEFAULT_ENTITY_POOL_SIZE);
		// Deletes entitty pool
		const bool deleteEntityPool(const std::string& name);
		// Resize entity pool
		const bool resizeEntityPool(const std::string& entityPoolName, const unsigned int size);
		// Check if there is pool with same name
		const bool hasEntityPoolName(const std::string& name);
		// Get size of entity Pool
		const unsigned int getEntityPoolSize(const std::string& name = ECS::DEFAULT_ENTITY_POOL_NAME);
		// Get alive entity count of entity pool
		const unsigned int getAliveEntityCountInEntityPool(const std::string& name = ECS::DEFAULT_ENTITY_POOL_NAME);
		// Check is number is power of 2
		const bool isPowerOfTwo(const unsigned int n);
		// Round up the number to power of 2
		void roundToNearestPowerOfTwo(unsigned int & n);

		// Creates entity and adds to entity pool
		Entity* createEntity(const std::string& poolName = ECS::DEFAULT_ENTITY_POOL_NAME);
		// Kill entity
		const bool killEntity(ECS::Entity* e);
		// Get entity by id. Returns nullptr if anything is invalid
        Entity* getEntityById(const E_ID entityId);
		// Get all entity in pool
		void getAllEntitiesInPool(std::vector<ECS::Entity*>& entities, const std::string& poolName = ECS::DEFAULT_ENTITY_POOL_NAME);
		// Get all entities that system updates
		template<class T>void getAllEntitiesForSystem(std::vector<ECS::Entity*>& entities)
		{
			unsigned int totalSize = 0;
			auto system = this->getSystem<T>();
			if (system != nullptr)
			{
				for (auto poolName : system->entityPoolNames)
				{
					unsigned int size = this->getAliveEntityCountInEntityPool(poolName);
					totalSize += size;
				}

				entities.clear();
				entities.reserve(totalSize);

				for (auto poolName : system->entityPoolNames)
				{
					std::vector<ECS::Entity*> eVec;
					this->getAllEntitiesInPool(eVec, poolName);
					entities.insert(entities.end(), eVec.begin(), eVec.end());
				}
			}
		}
		// Move entity to Entity pool
		const bool moveEntityToEntityPool(ECS::Entity*& entity, const std::string& entityPoolName);

		// Create component.
		template<class T> T* createComponent()
		{
			T* t = new T();
			const C_UNIQUE_ID cUniqueId = this->registerComponent(typeid(T));
			if (cUniqueId == INVALID_C_UNIQUE_ID)
			{
				delete t;
				return nullptr;
			}
			else
			{
				t->uniqueId = cUniqueId;
				return t;
			}
		}
		// Delete component.
		template<class T> const bool deleteComponent(T*& component)
		{
			ECS::Component* c = component;
			bool ret = this->deleteComponent(c, typeid(T));
			if (ret)
			{
				component = nullptr;
			}
			return ret;
		}
        // Check if entity has this type of component
        template<class T> const bool hasComponent(Entity* e)
        {
            return this->hasComponent(e, typeid(T));
        }
		// Check if entity has specific component
		template<class T> const bool hasComponent(Entity* e, Component* c)
		{
			return this->hasComponent(e, typeid(T), c);
		}
        // Get component of this type on entity. Will only return first one if there are more than one smae types of component
        template<class T> T* getComponent(Entity* e)
        {
            return static_cast<T*>(this->getComponent(e, typeid(T)));
        }
        // Get all componets of this type on enttiy.
        template<class T> std::vector<T*> getComponents(Entity* e)
        {
            std::vector<T*> ret;
            std::vector<Component*> component = this->getComponents(e, typeid(T));
            
			for(auto c : component)
            {
				ret.push_back(static_cast<T*>(c));
            }
            
            return ret;
        }
        // Add new component to entity.
        template<class T> const bool addComponent(Entity* e)
        {
            return this->addComponent(e, typeid(T), createComponent<T>());
        }
		// Add existing component to entity
        template<class T>  const bool addComponent(Entity* e, Component* c)
        {
            return this->addComponent(e, typeid(T), c);
        }
		// Checks if entity has entity component of this type with id
		template<class T> const bool removeComponent(Entity* e, const C_ID componentId)
		{
			return this->removeComponent(e, typeid(T), componentId);
		}
		// Remove specific component from entity
		template<class T> const bool removeComponent(Entity* e, Component* c)
		{
			return this->removeComponent(e, typeid(T), c);
		}
        // Remove all compoennts of type
        template<class T> const bool removeComponents(Entity* e)
        {
            return this->removeComponents(e, typeid(T));
        }
		// Get number of components of type that manager has
		template<class T> const unsigned int getComponentCount()
		{
			C_UNIQUE_ID cUniqueId = this->getComponentUniqueId(typeid(T));
			if (cUniqueId != ECS::INVALID_C_UNIQUE_ID)
			{
				const unsigned int poolSize = this->components.at(cUniqueId)->pool.size();
				const unsigned int nextIndiciesSize = this->components.at(cUniqueId)->nextIndicies.size();
				return poolSize - nextIndiciesSize;
			}
			else
			{
				return 0;
			}
		}
		
		// Creates new system and adds to manager
		template<class T> T* createSystem()
		{
			T* t = new T();
			const S_ID systemId = this->registerSystem(typeid(T));
			if (systemId == ECS::INVALID_S_ID)
			{
				delete t;
				return nullptr;
			}
			else
			{
				t->id = systemId;

				auto find_it = this->systems.find(t->priority);
				if (find_it != this->systems.end())
				{
					// Already have other system with same priority.
					delete t;
					return nullptr;
				}

				this->systems.insert(std::pair<int, std::unique_ptr<ECS::System, ECS::Deleter<ECS::System>>>(t->priority, std::unique_ptr<ECS::System, ECS::Deleter<ECS::System>>(t, ECS::Deleter<ECS::System>())));

				return t;
			}
		}
		// Deletes system
		template<class T> const bool deleteSystem(T*& system)
		{
			ECS::System* s = system;
			bool ret = this->deleteSystem(s, typeid(T));
			if (ret)
			{
				system = nullptr;
			}
			return ret;
		}
		// Check if manager has this type of system
		template<class T> const bool hasSystem()
		{
			const S_ID sId = this->getSystemId(typeid(T));

			for (auto& system : this->systems)
			{
				if (system.second->id == sId)
				{
					return true;
				}
			}

			return false;
		}
		// Check if manager has specific system
		template<class T> const bool hasSystem(System* system)
		{
			if (system == nullptr || system->id == ECS::INVALID_S_ID)
			{
				return false;
			}

			for (auto& s : this->systems)
			{
				if (s.second->id == system->id)
				{
					return true;
				}
			}

			return false;
		}
		// Get system
		template<class T> T* getSystem()
		{
			if (this->hasSystem<T>())
			{
				try
				{
					return static_cast<T*>(
						this->systems.at(
							this->getSystemId(typeid(T))).get());
				}
				catch (const std::out_of_range& oor)
				{
					return nullptr;
				}
			}
			else
			{
				return nullptr;
			}
		}
		// Get system update order
		std::map<int, S_ID> getSystemUpdateOrder();

		// Clear manager and resets. Everything gets wiped
		void clear();

		// Print component information. For debug
		void printComponentsInfo();
	};
    
    /**
     *	@class Entity
     *	@brief Entity is simply an pack of numbers.
     *	@note Entity doesn't carries Components.
     */
    class Entity
    {
    private:
        friend class Manager;
		friend class EntityPool;
        friend class Deleter<Entity>;		
    private:
        // Constructor
        Entity();
        
        // User can't call delete on entity. Must call kill.
        ~Entity() = default;
        
        // Disable all other constructors.
        Entity(const Entity& arg) = delete;								// Copy constructor
        Entity(const Entity&& arg) = delete;							// Move constructor
        Entity& operator=(const Entity& arg) = delete;					// Assignment operator
        Entity& operator=(const Entity&& arg) = delete;					// Move operator
        
        // Signature.
        Signature signature;
        
        // Component Index map
        std::unordered_map<C_UNIQUE_ID, std::unordered_set<C_INDEX>> componentIndicies;
        
        // ID of entity.
        E_ID id;
        
        // Index of entity pool for fast access. This is fixed.
        E_INDEX index;

		// Pool name that this entity lives
		std::string entityPoolName;
        
        // If entity is only visible if it's alive. Dead entities will not be queried or accessible.
        bool alive;

		// If entity is in sleep, it doesn't get updated by manager. 
		bool sleep;
        
        // Revive this entity and get ready to use
        void revive(const E_ID newId);
        
        // get C_INDEX by C_UNIQUE_ID
        void getComponentIndiicesByUniqueId(const C_UNIQUE_ID cId, std::unordered_set<C_INDEX>& cIndicies);

    public:
        // Kill entity. Once this is called, this entity will not be functional anymore.
        void kill();
        
        // Get entity Id
        const E_ID getId();

		// Get EntityPool name that this entity lives
		const std::string getEntityPoolName();
        
        // Check if this entity is alive
        const bool isAlive();

		// Get signature
		const Signature getSignature();
        
        // Check if Entity has Component
        template<class T>
        const bool hasComponent()
        {
            Manager* m = Manager::getInstance();
            return m->hasComponent<T>(this);
        }

		template<class T>
		const bool hasComponent(Component* c)
		{
			Manager* m = Manager::getInstance();
			return m->hasComponent<T>(this, c);
		}
        
        template<class T>
        T* getComponent()
        {
            Manager* m = Manager::getInstance();
            return m->getComponent<T>(this);
        }
        
        template<class T>
        std::vector<T*> getComponents()
        {
            Manager* m = Manager::getInstance();
            return m->getComponents<T>(this);
        }
        
        template<class T>
        const bool addComponent()
        {
            Manager* m = Manager::getInstance();
            return m->addComponent<T>(this);
        }

        template<class T>
        const bool addComponent(Component* c)
        {
            Manager* m = Manager::getInstance();
            return m->addComponent<T>(this, c);
        }

		template<class T>
		const bool removeComponent(const C_ID componentId)
		{
			Manager* m = Manager::getInstance();
			return m->removeComponent<T>(this, componentId);
		}

		template<class T>
		const bool removeComponent(Component* c)
		{
			Manager* m = Manager::getInstance();
			return m->removeComponent<T>(this, c);
		}
        
        template<class T>
        const bool removeComponents()
        {
            Manager* m = Manager::getInstance();
            return m->removeComponents<T>(this);
        }
    };
};
#endif
