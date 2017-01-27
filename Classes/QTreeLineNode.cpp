#include "QTreeLineNode.h"

USING_NS_CC;

QTreeLineNode* QTreeLineNode::createNode()
{
	QTreeLineNode* newNode = QTreeLineNode::create();
	return newNode;
}

bool QTreeLineNode::init()
{
    if (!cocos2d::Node::init())
	{
		return false;
	}
	
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	this->lineDrawNode = cocos2d::DrawNode::create();
	this->addChild(this->lineDrawNode);

	// Draw box
	this->lineDrawNode->setLineWidth(2.0f);
	// Because 0,0 is actually out of screen
	cocos2d::Vec2 leftBottom = cocos2d::Vec2(1, 1);
	const float lineLength = winSize.height - 1;
	cocos2d::Vec2 rightTop = cocos2d::Vec2(lineLength, lineLength);
	// Left bottom to right bottom
	this->lineDrawNode->drawLine(leftBottom, cocos2d::Vec2(rightTop.x, leftBottom.x), cocos2d::Color4F::ORANGE);
	// Left bottom to left top
	this->lineDrawNode->drawLine(leftBottom, cocos2d::Vec2(leftBottom.x, rightTop.y), cocos2d::Color4F::ORANGE);
	// Right top to left Top
	this->lineDrawNode->drawLine(rightTop, cocos2d::Vec2(leftBottom.x, rightTop.y), cocos2d::Color4F::ORANGE);
	// Right top to right bottom
	this->lineDrawNode->drawLine(rightTop, cocos2d::Vec2(rightTop.x, leftBottom.x), cocos2d::Color4F::ORANGE);

	return true;
}