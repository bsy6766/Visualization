#include "Component.h"
#include "Utility.h"
#include "Entity.h"

using namespace ECS;

Component::Component(const int id) : id(id) {}

const int Component::getId()
{
	return this->id;
}



DirectionVector::DirectionVector() : Component(DIRECTION_VECTOR)
{
	setNewDirVec();
}

void DirectionVector::setNewDirVec()
{
	//create new dir vec no matter what
	float angle = Utility::Random::randomReal<float>(0.1f, 359.9f);
	float radianAngle = angle * M_PI / 180.0f;

	this->dirVec = cocos2d::Vec2(cosf(radianAngle), sinf(radianAngle));
}



Sprite::Sprite(cocos2d::Node& parent, const std::string& spriteName) : Component(SPRITE)
{
	this->sprite = cocos2d::Sprite::createWithSpriteFrameName(spriteName);
	this->sprite->setScaleX(Utility::Random::randomReal<float>(0.25f, 1.0f));
	this->sprite->setScaleY(Utility::Random::randomReal<float>(0.25f, 1.0f));
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



Boundary::Boundary(const cocos2d::Rect& boundary) : Component(BOUNDARY), boundary(boundary) {}



QTreeObject::QTreeObject() : Component(QTREE_OBJECT), tracking(false)
{
	this->speed = Utility::Random::randomReal<float>(20.0f, 100.0f);
	this->visitied.resize(Entity::maxEntitySize, 0);
}

QTreeObject::~QTreeObject()
{
	this->visitied.clear();
}