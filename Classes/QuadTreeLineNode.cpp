#include "QuadTreeLineNode.h"

USING_NS_CC;

QuadTreeLineNode* QuadTreeLineNode::createNode()
{
	QuadTreeLineNode* newNode = QuadTreeLineNode::create();
	return newNode;
}

bool QuadTreeLineNode::init()
{
    if (!cocos2d::Node::init())
	{
		return false;
	}

	this->dispalyBoundaryDrawNode = cocos2d::DrawNode::create();
	this->dispalyBoundaryDrawNode->setLineWidth(2.0f);
	this->addChild(this->dispalyBoundaryDrawNode);

	this->quadTreeSubDivisionDrawNode = cocos2d::DrawNode::create();
	this->quadTreeSubDivisionDrawNode->setLineWidth(1.0f);
	this->addChild(this->quadTreeSubDivisionDrawNode);

	return true;
}

void QuadTreeLineNode::drawDisplayBoundaryBox(const cocos2d::Rect & displayBoundary)
{
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	// Draw box
	// Because 0,0 is actually out of screen
	cocos2d::Vec2 leftBottom = cocos2d::Vec2(1, 1);
	const float lineLength = winSize.height - 1;
	cocos2d::Vec2 rightTop = cocos2d::Vec2(lineLength, lineLength);
	// Left bottom to right bottom
	this->dispalyBoundaryDrawNode->drawLine(leftBottom, cocos2d::Vec2(rightTop.x, leftBottom.x), cocos2d::Color4F::ORANGE);
	// Left bottom to left top
	this->dispalyBoundaryDrawNode->drawLine(leftBottom, cocos2d::Vec2(leftBottom.x, rightTop.y), cocos2d::Color4F::ORANGE);
	// Right top to left Top
	this->dispalyBoundaryDrawNode->drawLine(rightTop, cocos2d::Vec2(leftBottom.x, rightTop.y), cocos2d::Color4F::ORANGE);
	// Right top to right bottom
	this->dispalyBoundaryDrawNode->drawLine(rightTop, cocos2d::Vec2(rightTop.x, leftBottom.x), cocos2d::Color4F::ORANGE);
}
