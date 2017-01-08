#include "QTreeLineNode.h"

USING_NS_CC;

QTreeLineNode* QTreeLineNode::createNode()
{
	QTreeLineNode* newNode = QTreeLineNode::create();
	return newNode;
}

bool QTreeLineNode::init()
{
	if (!CCNode::init())
	{
		return false;
	}
	
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	// Add line on boundaries
	auto lineTop = cocos2d::Sprite::createWithSpriteFrameName("dot.png");
	lineTop->setScaleX(winSize.height);
	lineTop->setPosition(cocos2d::Vec2(winSize.height * 0.5f, winSize.height - 1.0f));
	lineTop->setColor(cocos2d::Color3B::ORANGE);
	this->addChild(lineTop);

	auto lineBottom = cocos2d::Sprite::createWithSpriteFrameName("dot.png");
	lineBottom->setScaleX(winSize.height);
	lineBottom->setPosition(cocos2d::Vec2(winSize.height * 0.5f, 1.0f));
	lineBottom->setColor(cocos2d::Color3B::ORANGE);
	this->addChild(lineBottom);

	auto lineLeft = cocos2d::Sprite::createWithSpriteFrameName("dot.png");
	lineLeft->setScaleY(winSize.height);
	lineLeft->setPosition(cocos2d::Vec2(1.0f, winSize.height * 0.5f));
	lineLeft->setColor(cocos2d::Color3B::ORANGE);
	this->addChild(lineLeft);

	auto lineRight = cocos2d::Sprite::createWithSpriteFrameName("dot.png");
	lineRight->setScaleY(winSize.height);
	lineRight->setPosition(cocos2d::Vec2(winSize.height - 1.0f, winSize.height * 0.5f));
	lineRight->setColor(cocos2d::Color3B::ORANGE);
	this->addChild(lineRight);

	return true;
}