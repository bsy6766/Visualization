#ifndef COMPONENT_H
#define COMPONENT_H

#include "cocos2d.h"
#include "ECS.h"

/**
*	Note
*
*	Since this project is just for visualization, I declared everything
*	in public field to make easier to access from outside. Not considering
*	any encapsulation or whatnot.
*/

namespace ECS
{
	enum COMPONENT_ID
	{
		DIRECTION_VECTOR = 0,
		SPRITE,
		QTREE_OBJECT,
		FLOCKING_OBJECT
	};


	class DirectionVector : public Component
	{
	public:
		DirectionVector();
		~DirectionVector() = default;
		DirectionVector(DirectionVector const&) = delete;
		void operator=(DirectionVector const&) = delete;

		// Direction vector
		cocos2d::Vec2 dirVec;
		// Smooth steer
		bool smoothSteer;
		// Sets new direction vector
		void setNewDirVec();
		// Get angle from direction vector. 0 starts from right side (1, 0).
		const float getAngle();
	};

	class Sprite : public Component
	{
	public:
		Sprite(cocos2d::Node& parent, const std::string& spriteName);
		~Sprite();
		Sprite(Sprite const&) = delete;
		void operator=(Sprite const&) = delete;

		cocos2d::Sprite* sprite;

		// Rotate sprite. Wrap angle if it's less than 0 or greater than 360
		void rotateToDirVec(float angle);
		// Set position to random place in boundary
		void setRandomPosInBoundary(const cocos2d::Rect& boundary);
		// Wrap sprite's position to boundary
		void wrapPositionWithInBoundary(const cocos2d::Rect& boundary);
	};

	class QTreeObject : public Component
	{
	public:
		QTreeObject();
		~QTreeObject();
		QTreeObject(QTreeObject const&) = delete;
		void operator=(QTreeObject const&) = delete;

		std::vector<int> visitied;
		float speed;
		bool tracking;
	};

	class FlockingObject : public Component
	{
	public:
		enum class TYPE
		{
			BOID,
			OBSTACLE
		};

	public:
		FlockingObject(const TYPE type);
		~FlockingObject() = default;
		FlockingObject(FlockingObject const&) = delete;
		void operator=(FlockingObject const&) = delete;
		
		static float movementSpeed;
		static float steerSpeed;
		bool tracking;

		static float SIGHT_RADIUS;
		static float COHENSION_WEIGHT;
		static float ALIGNMENT_WEIGHT;
		static float SEPARATION_WEIGHT;
		static float AVOID_RADIUS;
		static float AVOID_WEIGHT;

		TYPE type;
	};
}

#endif