#include "Component.h"
#include "Utility.h"
#include "ECS.h"

using namespace ECS;


DirectionVector::DirectionVector() : Component(DIRECTION_VECTOR), dirVec(cocos2d::Vec2::ZERO),  smoothSteer(false)
{
	setNewDirVec();
}

void DirectionVector::setNewDirVec()
{
	//create new dir vec no matter what
	float angle = Utility::Random::randomReal<float>(0.0f, 359.9f);
	float radianAngle = angle * M_PI / 180.0f;

	this->dirVec = cocos2d::Vec2(cosf(radianAngle), sinf(radianAngle));
	this->dirVec.normalize();
}

const float ECS::DirectionVector::getAngle()
{
	const float radianAngle = acos(cocos2d::Vec2::dot(dirVec, cocos2d::Vec2(1, 0)));
	float angle = radianAngle * 180.0f / M_PI;
	if (this->dirVec.y < 0)
	{
		angle = 360.0f - angle;
	}
	return angle;
}



Sprite::Sprite(cocos2d::Node& parent, const std::string& spriteName) : Component(SPRITE)
{
	this->sprite = cocos2d::Sprite::createWithSpriteFrameName(spriteName);
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
	this->sprite->setPosition(winSize.height * 0.5f, winSize.height * 0.5f);
	this->sprite->retain();
	parent.addChild(this->sprite);
}

Sprite::~Sprite()
{
	this->sprite->release();
	this->sprite->removeFromParentAndCleanup(true);
}

void ECS::Sprite::rotateToDirVec(float angle)
{
	if (angle < 0) angle += 360.0f;
	else if (angle >= 360.0f) angle -= 360.0f;

	this->sprite->setRotation(angle);
}

void ECS::Sprite::setRandomPosInBoundary(const cocos2d::Rect & boundary)
{
	sprite->setPosition(
		cocos2d::Vec2(
			Utility::Random::randomReal<float>(
				boundary.getMinX(), 
				boundary.getMaxX()
			), 
			Utility::Random::randomReal<float>(
				boundary.getMinY(),
				boundary.getMaxY()
			)
		)
	);
}

void ECS::Sprite::wrapPositionWithInBoundary(const cocos2d::Rect & boundary)
{
	auto curPos = this->sprite->getPosition();

	if (curPos.x <= boundary.getMinX())
	{
		curPos.x = boundary.getMaxX();
	}
	else if (curPos.x >= boundary.getMaxX())
	{
		curPos.x = boundary.getMinX();
	}

	if (curPos.y <= boundary.getMinY())
	{
		curPos.y = boundary.getMaxY();
	}
	else if (curPos.y >= boundary.getMaxY())
	{
		curPos.y = boundary.getMinY();
	}

	this->sprite->setPosition(curPos);
}

QTreeData::QTreeData() : Component(QTREE_DATA), tracking(false)
{
	this->speed = Utility::Random::randomReal<float>(20.0f, 100.0f);
	this->visitied.resize(Entity::maxEntitySize, 0);
}

QTreeData::~QTreeData()
{
	this->visitied.clear();
}



float FlockingData::movementSpeed = 1.0f;
float FlockingData::steerSpeed = 2.0f;
float FlockingData::SIGHT_RADIUS = 30.0f;
float FlockingData::COHENSION_WEIGHT = 1.0f;
float FlockingData::ALIGNMENT_WEIGHT = 1.0f;
float FlockingData::SEPARATION_WEIGHT = 1.0f;
float FlockingData::AVOID_RADIUS = 50.0f;
float FlockingData::AVOID_WEIGHT = 2.0f;

FlockingData::FlockingData(const TYPE type) : Component(FLOCKING_DATA), tracking(false), type(type) {}



float CirclePackingData::maxRadius = 50.0f;
float CirclePackingData::growthSpeed = 25.0f;
float CirclePackingData::initialRadius = 5.0f;

CirclePackingData::CirclePackingData(const cocos2d::Vec2 & position, const float radius, const cocos2d::Color4F color) : Component(CIRCLE_PACKING_DATA), alive(false), growing(false), position(position), radius(radius), color(color) {}

void CirclePackingData::update(const float delta)
{
	if (this->growing)
	{
		this->radius += (delta * CirclePackingData::growthSpeed);
		if (this->radius > CirclePackingData::maxRadius)
		{
			this->radius = CirclePackingData::maxRadius;
			this->growing = false;
		}
	}
}

void CirclePackingData::activate(const cocos2d::Vec2 & position, const float radius, const cocos2d::Color4F color)
{
	this->alive = true;
	this->growing = true;
	this->position = position;
	this->radius = radius;
	this->color = color;
}

void CirclePackingData::deactivate()
{
	this->alive = false;
	this->growing = false;
	this->position = cocos2d::Vec2::ZERO;
	this->radius = 0;
	this->color = cocos2d::Color4F::WHITE;
}
