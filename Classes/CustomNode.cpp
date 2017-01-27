#include "CustomNode.h"
#include "MainScene.h"

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

void DisplayBoundaryBoxNode::drawDisplayBoundaryBox()
{
    float squareLength = cocos2d::Director::getInstance()->getVisibleSize().height;
    float xDiff = squareLength - this->displayBoundary.size.width;
    float yDiff = squareLength - this->displayBoundary.size.width;
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
    
    this->displayBoundary.origin += shift;
}




const std::string LabelsNode::fontPath = "fonts/Rubik-Medium.ttf";

LabelsNode* LabelsNode::createNode()
{
    LabelsNode* newNode = LabelsNode::create();
    return newNode;
}

bool LabelsNode::init()
{
    if (!cocos2d::Node::init())
    {
        return false;
    }
    
    // Init animation action
    this->labelAnimation = cocos2d::Sequence::create(cocos2d::ScaleTo::create(0, 0.85f), cocos2d::DelayTime::create(0.25f), cocos2d::ScaleTo::create(0, 1.0f), nullptr);
    this->labelAnimation->retain();
    
    auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
    
    int fontSize = 20.0f;
    
    // Init label
    this->backLabel = cocos2d::Label::createWithTTF("BACK(ESC)", fontPath, fontSize);
    this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 70.0f, 40.0f));
    this->addChild(this->backLabel);
    
    float labelX = winSize.height - 10.0f;
    
    fpsLabel = cocos2d::Label::createWithTTF("FPS: " + std::to_string(cocos2d::Director::getInstance()->getFrameRate()), fontPath, fontSize);
    fpsLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    fpsLabel->setPosition(cocos2d::Vec2(labelX, 40.0f));
    this->addChild(fpsLabel);
    
    // fps
    this->fps = 0;
    this->fpsElapsedTime = 0;
    
    return true;
}

const bool LabelsNode::isValidIndex(TYPE type, const int index)
{
    bool ret = false;
    switch (type) {
        case TYPE::CUSTOM:
        {
            if(index >= 0 || index < static_cast<int>(this->customLabels.size()))
            {
                ret = true;
            }
        }
            break;
        case TYPE::KEYBOARD:
        {
            if(index >= 0 || index < static_cast<int>(this->keyboardUsageLabels.size()))
            {
                ret = true;
            }
        }
            break;
        case TYPE::MOUSE:
        {
            if(index >= 0 || index < static_cast<int>(this->mouseUsageLabels.size()))
            {
                ret = true;
            }
        }
            break;
        default:
            break;
    }
    
    return ret;
}

void LabelsNode::addLabel(LabelsNode::TYPE type, const std::string &str, const int fontSize)
{
    auto newLabel = cocos2d::Label::createWithTTF(str, fontPath, fontSize);
    newLabel->setAnchorPoint(cocos2d::Vec2(0, 1.0f));
    
    float size = 0;
    cocos2d::Vec2 pos = cocos2d::Vec2::ZERO;
    
    switch (type) {
        case TYPE::CUSTOM:
        {
            size = static_cast<float>(this->customLabels.size());
            pos  = this->customLabelStartPos;
            
            this->customLabels.push_back(newLabel);
        }
            break;
        case TYPE::KEYBOARD:
        {
            size = static_cast<float>(this->keyboardUsageLabels.size());
            pos = this->keyboardUsageLabelStartPos;
            
            this->keyboardUsageLabels.push_back(newLabel);
        }
            break;
        case TYPE::MOUSE:
        {
            size = static_cast<float>(this->mouseUsageLabels.size());
            pos = this->mouseUsageLabelStartPos;
            
            this->mouseUsageLabels.push_back(newLabel);
        }
            break;
        default:
            return;
            break;
    }
                                      
    pos.y -= (size * fontSize);
    if(size > 0)
    {
        pos.y -= 5.0f;
    }
    newLabel->setPosition(pos);
    this->addChild(newLabel);
}

void LabelsNode::updateLabel(const int index, const std::string& str)
{
    if(index >= 0 || index < static_cast<int>(this->customLabels.size()))
    {
        this->customLabels.at(index)->setString(str);
    }
}

void LabelsNode::setColor(TYPE type, const int index, cocos2d::Color3B color, const bool playAnimation)
{
    if(isValidIndex(type, index))
    {
        switch (type) {
            case TYPE::CUSTOM:
            {
                this->customLabels.at(index)->setColor(color);
            }
                break;
            case TYPE::KEYBOARD:
            {
                this->keyboardUsageLabels.at(index)->setColor(color);
                
            }
                break;
            case TYPE::MOUSE:
            {
                this->mouseUsageLabels.at(index)->setColor(color);
            }
                break;
            default:
                break;
        }
        
        if(playAnimation)
        {
            this->playAnimation(type, index);
        }
    }
}

void LabelsNode::updateFPSLabel(const float delta)
{
    this->fpsElapsedTime += delta;
    if (this->fpsElapsedTime > 1.0f)
    {
        this->fpsElapsedTime -= 1.0f;
        fps++;
        fpsLabel->setString("FPS: " + std::to_string(fps) + " (" + std::to_string(delta).substr(0, 5) + "ms)");
        fps = 0;
    }
    else
    {
        fps++;
    }
}

void LabelsNode::updateMouseHover(const cocos2d::Vec2 &mousePos)
{
    if (this->backLabel->getBoundingBox().containsPoint(mousePos))
    {
        this->backLabel->setScale(1.2f);
    }
    else
    {
        if (this->backLabel->getScale() > 1.0f)
        {
            this->backLabel->setScale(1.0f);
        }
    }
}

const bool LabelsNode::updateMouseDown(const cocos2d::Vec2 &mousePos)
{
    
    if (this->backLabel->getBoundingBox().containsPoint(mousePos))
    {
        cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
        return true;
    }
    
    return false;
}


void LabelsNode::playAnimation(TYPE type, const int index)
{
    if(isValidIndex(type, index))
    {
        switch (type) {
            case TYPE::CUSTOM:
            {
                this->customLabels.at(index)->stopAllActions();
                this->customLabels.at(index)->setScale(1.0f);
                this->customLabels.at(index)->runAction(this->labelAnimation->clone());
            }
                break;
            case TYPE::KEYBOARD:
            {
                this->keyboardUsageLabels.at(index)->stopAllActions();
                this->keyboardUsageLabels.at(index)->setScale(1.0f);
                this->keyboardUsageLabels.at(index)->runAction(this->labelAnimation->clone());
            }
                break;
            case TYPE::MOUSE:
            {
                this->mouseUsageLabels.at(index)->stopAllActions();
                this->mouseUsageLabels.at(index)->setScale(1.0f);
                this->mouseUsageLabels.at(index)->runAction(this->labelAnimation->clone());
            }
                break;
                
            default:
                break;
        }
    }
}

void LabelsNode::onExit()
{
    cocos2d::Node::onExit();
    this->labelAnimation->retain();
}
