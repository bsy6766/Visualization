#ifndef SYSTEM_H
#define SYSTEM_H

#include "Component.h"
#include "ECS.h"
#include "QuadTree.h"
#include "cocos2d.h"

namespace ECS
{
	class QuadTreeSystem : public System
	{
	public:
		QuadTreeSystem();
		~QuadTreeSystem() = default;

		// Quad tree
		std::unique_ptr<QuadTree> quadTree;
		// Display boundary
		cocos2d::Rect displayBoundary;

		// Flags
		bool duplicationCheck;
		bool showGrid;
		bool collisionResolve;

		// Counters
		int collisionChecksCount;
		int collisionCheckWithOutRepeatCount;

		// Tracking entity
		ECS::E_ID lastTrackingEntityID;

		// Initialize quad tree
		void initQuadTree(cocos2d::Rect& boundary);
		// Update quad tree after updating entity position
		void rebuildQuadTree(std::vector<ECS::Entity*>& entities);

		// Update entitiy position
		void updateEntityPosition(const float delta, std::vector<ECS::Entity*>& entities);
		// Check if entity goes out of boundary
		void checkBoundary(ECS::Sprite& spriteComp, bool& flipX, bool& flipY);
		// Flip entity's direction
		void flipDirVec(const bool flipX, const bool flipY, cocos2d::Vec2& dirVec);
		// Check if entities collide each other
		void checkCollision(std::vector<ECS::Entity*>& entities);
		// Resolve collisions between entities
		void resolveCollisions(ECS::Sprite& entitySpriteComp, ECS::Sprite& nearEntitySpriteComp, ECS::DirectionVector& entityDirVecComp, ECS::DirectionVector& nearEntityDirVecComp);
		// Draw quad tree lines
		void drawQuadTreelines();

		// Mouse down
		bool updateMouseDown(const int mouseButton, const cocos2d::Vec2& point);
		
		// Update system
		virtual void update(const float delta, std::vector<ECS::Entity*>& entities) override;
	};

	class FlockingSystem : public System
	{
	public:
		FlockingSystem();
		~FlockingSystem() = default;

		// Quad tree
		std::unique_ptr<QuadTree> quadTree;
		// Display boundary
		cocos2d::Rect displayBoundary;

		// Flags
		bool smoothSteering;

		// Tracking
		int lastTrackingBoidId;

		// Initialize quad tree
		void initQuadTree(cocos2d::Rect& boundary);
		// Update quad tree after updating entity position
		void rebuildQuadTree(std::vector<ECS::Entity*>& entities);

		// Mouse
		bool updateMouseDown(const int mouseButton, cocos2d::Vec2& point);

		// Flocking algorithm	
		// Update flocking algorithm
		void updateFlockingAlgorithm(const float delta, std::vector<ECS::Entity*>& entities);
		/**
		*	Get Alignment vector
		*	Iterate through near boids and checks distance.
		*	If distance between boid and near boids are close enough,
		*	sum near boids' direction vector and get average then normalize
		*/
		const cocos2d::Vec2 getAlignment(ECS::Entity* boid, std::list<Entity*>& nearBoids);
		/**
		*	Get Cohesion
		*	Iterate thorugh near boids and checks distance.
		*	If distance between boid and near boids are close enough,
		*	sum near boids' position vector and get average then nomarlize.
		*	In this case, we want direction vector not position, so compute
		*	direction vector based on boid's position
		*/
		const cocos2d::Vec2 getCohesion(ECS::Entity* boid, std::list<Entity*>& nearBoids);
		/**
		*	Get Separation
		*	Iterate through near boids and checks distance
		*	If distance between boid and near boids are close enough,
		*	sum the distance between boid and near boids and get average,
		*	negate the direction, then normalize.
		*/
		const cocos2d::Vec2 getSeparation(ECS::Entity* boid, std::list<Entity*>& nearBoids);
		/**
		*	Get Avoid
		*	This isn't core flocking algorithm. Alignment, Cohesion and
		*	Separation are the core but this is a simple addition to
		*	algorithm to make boid avoid obstacles.
		*	It's similar to Separation, but weighted by distance
		*/
		const cocos2d::Vec2 getAvoid(ECS::Entity* boid, std::list<Entity*>& nearAvoids);

		// Update system
		virtual void update(const float delta, std::vector<ECS::Entity*>& entities) override;
	};
}

#endif