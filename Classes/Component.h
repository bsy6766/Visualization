#ifndef COMPONENT_H
#define COMPONENT_H

#include "cocos2d.h"

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
		BOUNDING_BOX,
		BOUNDARY,
		QTREE_OBJECT,
		MAX_COMPONENT
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

	class DirectionVector : public Component
	{
	public:
		DirectionVector();
		~DirectionVector() = default;
		DirectionVector(DirectionVector const&) = delete;
		void operator=(DirectionVector const&) = delete;

		cocos2d::Vec2 dirVec;
		void setNewDirVec();
	};

	class Sprite : public Component
	{
	public:
		Sprite(cocos2d::Node& parent, const std::string& spriteName);
		~Sprite();
		Sprite(Sprite const&) = delete;
		void operator=(Sprite const&) = delete;

		cocos2d::Sprite* sprite;
	};

	class Boundary : public Component
	{
	public:
		Boundary(const cocos2d::Rect& boundary);
		~Boundary() = default;
		Boundary(Boundary const&) = delete;
		void operator=(Boundary const&) = delete;

		cocos2d::Rect boundary;
	};

	class QTreeObject : public Component
	{
	public:
		QTreeObject();
		~QTreeObject();
		QTreeObject(QTreeObject const&) = delete;
		void operator=(QTreeObject const&) = delete;

		//std::list<int> visitied;
		std::vector<int> visitied;
		float speed;
		bool tracking;
	};
}

#endif