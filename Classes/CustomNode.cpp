#include "CustomNode.h"

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

    this->drawNode = cocos2d::DrawNode::create();
    this->drawNode->setLineWidth(1.0f);
    this->addChild(this->drawNode);

	return true;
}





DisplayBoundaryBoxNode* DisplayBoundaryBoxNode::createNode()
{
    DisplayBoundaryBoxNode* newNode = DisplayBoundaryBoxNode::create();
    return newNode;
}

bool DisplayBoundaryBoxNode::init()
{
    if (!cocos2d::Node::init())
    {
        return false;
    }
    
    this->drawNode = cocos2d::DrawNode::create();
    this->drawNode->setLineWidth(2.0f);
    this->addChild(this->drawNode);
    
    return true;
}


void DisplayBoundaryBoxNode::drawDisplayBoundaryBox(cocos2d::Rect & displayBoundary)
{
    float squareLength = cocos2d::Director::getInstance()->getVisibleSize().height;
    float xDiff = squareLength - displayBoundary.size.width;
    float yDiff = squareLength - displayBoundary.size.width;
    cocos2d::Vec2 shift = cocos2d::Vec2(xDiff, yDiff) * 0.5f;
    
	// Draw box
	// Because 0,0 is actually out of screen
    cocos2d::Vec2 leftBottom = displayBoundary.origin + shift;
    this->drawNode->setLineWidth(2.0f);
    cocos2d::Vec2 rightTop = leftBottom + displayBoundary.size;
	// Left bottom to right bottom
	this->drawNode->drawLine(leftBottom, cocos2d::Vec2(rightTop.x, leftBottom.x), cocos2d::Color4F::ORANGE);
	// Left bottom to left top
	this->drawNode->drawLine(leftBottom, cocos2d::Vec2(leftBottom.x, rightTop.y), cocos2d::Color4F::ORANGE);
	// Right top to left Top
	this->drawNode->drawLine(rightTop, cocos2d::Vec2(leftBottom.x, rightTop.y), cocos2d::Color4F::ORANGE);
	// Right top to right bottom
	this->drawNode->drawLine(rightTop, cocos2d::Vec2(rightTop.x, leftBottom.x), cocos2d::Color4F::ORANGE);
    
    displayBoundary.origin += shift;
}




