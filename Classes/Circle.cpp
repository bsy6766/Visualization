#include "Circle.h"

float Circle::MAX_RADIUS = 60.0f;
float Circle::GROWTH_SPEED = 2.0f;

Circle::Circle(const cocos2d::Vec2& position, const float radius, const cocos2d::Color4F color) : position(position), radius(radius), color(color), growing(false), alive(false) {}

void Circle::update(const float delta)
{
	if (this->growing)
	{
		this->radius += delta * Circle::GROWTH_SPEED;
		if (this->radius > Circle::MAX_RADIUS)
		{
			this->radius = Circle::MAX_RADIUS;
			this->growing = false;
		}
	}
}

void Circle::activate(const cocos2d::Vec2 & position, const float radius, const cocos2d::Color4F color)
{
	this->alive = true;
	this->growing = true;
	this->position = position;
	this->radius = radius;
	this->color = color;
}