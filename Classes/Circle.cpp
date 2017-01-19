#include "Circle.h"

float Circle::maxRadius = 60.0f;
float Circle::growthSpeed = 2.0f;
int Circle::idCounter = 0;

Circle::Circle(const cocos2d::Vec2& position, const float radius, const cocos2d::Color4F color) : position(position), radius(radius), color(color), growing(false), alive(false), id(idCounter++) {}

void Circle::update(const float delta)
{
	if (this->growing)
	{
		this->radius += delta * Circle::growthSpeed;
		if (this->radius > Circle::maxRadius)
		{
			this->radius = Circle::maxRadius;
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

void Circle::deactivate()
{
	this->alive = false;
	this->growing = false;
	this->position = cocos2d::Vec2::ZERO;
	this->radius = 0;
	this->color = cocos2d::Color4F::WHITE;
}
