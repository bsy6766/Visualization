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
		QuadTree* quadTree;
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
}

#endif