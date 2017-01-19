#ifndef CIRCLE_H
#define CIRCLE_H

#include "cocos2d.h"

class Circle
{
public:
	Circle(const cocos2d::Vec2& position, const float radius, const cocos2d::Color4F color = cocos2d::Color4F::WHITE);
	~Circle() = default;

	cocos2d::Vec2 position;
	float radius;
	cocos2d::Color4F color;

	bool growing;
	bool alive;

	static float MAX_RADIUS;
	static float GROWTH_SPEED;

	void update(const float delta);
	void activate(const cocos2d::Vec2& position, const float radius, const cocos2d::Color4F color);
};

#endif