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

	// Init title LAbel
	const int titleSize = 35;
	this->titleLabel = cocos2d::Label::createWithTTF("Visualization", fontPath, titleSize);
	this->titleLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(this->titleLabel);

	// Init back label
	const int fontSize = 20;
	this->backLabel = cocos2d::Label::createWithTTF("BACK(ESC)", fontPath, fontSize);
	this->backLabel->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	this->addChild(this->backLabel);

	// Init fps label
	this->fpsLabel = cocos2d::Label::createWithTTF("FPS: " + std::to_string(cocos2d::Director::getInstance()->getFrameRate()), fontPath, fontSize);
	this->fpsLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->addChild(fpsLabel);
    
    // fps
    this->fps = 0;
    this->fpsElapsedTime = 0;
    
    return true;
}

const bool LabelsNode::isValidIndex(TYPE type, const int index)
{
    int size = 0;
    switch (type) {
        case TYPE::CUSTOM:
            size = static_cast<int>(this->customLabels.size());
            break;
        case TYPE::KEYBOARD:
            size = static_cast<int>(this->keyboardUsageLabels.size());
            break;
        case TYPE::MOUSE_OVER_AND_KEY:
            size = static_cast<int>(this->mouseOverAndKeyUsageLabels.size());
            break;
        case TYPE::MOUSE:
            size = static_cast<int>(this->mouseUsageLabels.size());
            break;
        default:
            break;
    }
    
    return (size > 0 && index >= 0 && index < size);
}

void LabelsNode::addLabel(LabelsNode::TYPE type, const std::string &str, const int fontSize)
{
    auto newLabel = cocos2d::Label::createWithTTF(str, fontPath, fontSize);
    newLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    
    cocos2d::Vec2 pos = cocos2d::Vec2::ZERO;
	float yOffSet = 0;
	float trim = 5.0f;
    
    switch (type) {
        case TYPE::CUSTOM:
        {
            pos  = this->customLabelStartPos;
			for (auto label : this->customLabels)
			{
				yOffSet += (label->getContentSize().height - trim);
			}
            
            this->customLabels.push_back(newLabel);
        }
            break;
        case TYPE::KEYBOARD:
        {
            pos = this->keyboardUsageLabelStartPos;
			for (auto label : this->keyboardUsageLabels)
			{
				yOffSet += (label->getContentSize().height - trim);
			}

            this->keyboardUsageLabels.push_back(newLabel);
        }
            break;
        case TYPE::MOUSE_OVER_AND_KEY:
        {
            pos = this->mouseOverAndKeyLabelStartPos;
			for (auto label : this->mouseOverAndKeyUsageLabels)
			{
				yOffSet += (label->getContentSize().height - trim);
			}
            
            this->mouseOverAndKeyUsageLabels.push_back(newLabel);
        }
            break;
        case TYPE::MOUSE:
        {
            pos = this->mouseUsageLabelStartPos;
			for (auto label : this->mouseUsageLabels)
			{
				yOffSet += (label->getContentSize().height - trim);
			}
            
            this->mouseUsageLabels.push_back(newLabel);
        }
            break;
        default:
            return;
            break;
    }
                                      
	pos.y -= yOffSet;
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
            case TYPE::MOUSE_OVER_AND_KEY:
            {
                this->mouseOverAndKeyUsageLabels.at(index)->setColor(color);
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
            case TYPE::MOUSE_OVER_AND_KEY:
            {
                this->mouseOverAndKeyUsageLabels.at(index)->stopAllActions();
                this->mouseOverAndKeyUsageLabels.at(index)->setScale(1.0f);
                this->mouseOverAndKeyUsageLabels.at(index)->runAction(this->labelAnimation->clone());
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

void LabelsNode::initTitleStr(const std::string& titleString, const cocos2d::Vec2& pos)
{
	this->titleLabel->setString(titleString + " Visualization");
	this->titleLabel->setPosition(pos);
}

void LabelsNode::setSharedLabelPosition(SHARED_LABEL_POS_TYPE type)
{
	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
	int fontSize = 20.0f;

	switch (type)
	{
	case SHARED_LABEL_POS_TYPE::QUADTREE_SCENE:
	case SHARED_LABEL_POS_TYPE::FLOCKING_SCENE:
	{
		float height = 40.0f;
		this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 70.0f, height));
		float labelX = winSize.height - 10.0f;
		this->fpsLabel->setPosition(cocos2d::Vec2(labelX, height));
	}
		break;
	case SHARED_LABEL_POS_TYPE::CIRCLE_PACKING_SCENE:
	{
		float height = 20.0f;
		this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 70.0f, height));
		float labelX = 130.0f;
		this->fpsLabel->setPosition(cocos2d::Vec2(labelX, height));
	}
	break;
	default:
		break;
	}
}

void LabelsNode::onExit()
{
    cocos2d::Node::onExit();
    this->labelAnimation->retain();
}





const std::string ButtonModifierNode::fontPath = "fonts/Rubik-Medium.ttf";

ButtonModifierNode* ButtonModifierNode::createNode()
{
    ButtonModifierNode* newNode = ButtonModifierNode::create();
    return newNode;
}

bool ButtonModifierNode::init()
{
    if (!cocos2d::Node::init())
    {
        return false;
    }
    
    this->buttonLabelStartPos = cocos2d::Vec2::ZERO;
    this->rightButtonXOffset = 0;
    this->leftButtonXOffset = 0;
    this->valueLabelXOffset = 0;
    
    return true;
}

const bool ButtonModifierNode::isValidIndex(TYPE type, const int index)
{
    int size = 0;
    switch (type) {
        case TYPE::LABEL:
            size = static_cast<int>(this->buttonLabels.size());
            break;
        case TYPE::VALUE:
            size = static_cast<int>(this->valueLabels.size());
            break;
        case TYPE::LEFT_BUTTON:
            size = static_cast<int>(this->leftButtons.size());
            break;
        case TYPE::RIGHT_BUTTON:
            size = static_cast<int>(this->rightButtons.size());
            break;
        default:
            break;
    }
    
    return (size > 0 && index >= 0 && index < size);
}

void ButtonModifierNode::addButton(const std::string& labelStr, const int fontSize, const float value, const std::string& leftButtonSpriteNamePrefix, const std::string& rightButtonSpriteNamePrefix, const std::string& buttonSpriteNameSuffix, const std::string& format, const int leftActionTag, const int rightActionTag, const cocos2d::ui::AbstractCheckButton::ccWidgetClickCallback& callback)
{
	const float pad = 3.0f; 

    // Butotn label
    auto newButtonLabel = cocos2d::Label::createWithTTF(labelStr, fontPath, fontSize);
    newButtonLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    auto labelPos = this->buttonLabelStartPos;
    labelPos.y -= (static_cast<float>(this->buttonLabels.size()) * (fontSize + pad));
    newButtonLabel->setPosition(labelPos);
    this->addChild(newButtonLabel);
    this->buttonLabels.push_back(newButtonLabel);
    
    // Value lable
    auto newValueLabel = cocos2d::Label::createWithTTF(std::to_string(value).substr(0, 3), fontPath, fontSize);
    auto valuePos = this->buttonLabelStartPos;
    valuePos.x += this->valueLabelXOffset;
    valuePos.y -= (static_cast<float>(this->valueLabels.size()) * (fontSize + pad));
    newValueLabel->setPosition(valuePos);
	newValueLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    this->addChild(newValueLabel);
    this->valueLabels.push_back(newValueLabel);
    
    const std::string suffix = buttonSpriteNameSuffix + format;
    
    // Left button
    auto leftButton = cocos2d::ui::Button::create(leftButtonSpriteNamePrefix + suffix, leftButtonSpriteNamePrefix + "Selected" + suffix, leftButtonSpriteNamePrefix + "Disabled" + suffix, cocos2d::ui::Widget::TextureResType::PLIST);
    auto leftButtonPos = this->buttonLabelStartPos;
    leftButtonPos.x += this->leftButtonXOffset;
    leftButtonPos.y -= (static_cast<float>(this->leftButtons.size()) * (fontSize + pad));
	leftButton->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    leftButton->setPosition(leftButtonPos);
    leftButton->addClickEventListener(callback);
    leftButton->setActionTag(leftActionTag);
    this->addChild(leftButton);
    this->leftButtons.push_back(leftButton);
    
    // Right button
    auto rightButton = cocos2d::ui::Button::create(rightButtonSpriteNamePrefix + suffix, rightButtonSpriteNamePrefix + "Selected" + suffix, rightButtonSpriteNamePrefix + "Disabled" + suffix, cocos2d::ui::Widget::TextureResType::PLIST);
    auto rightButtonPos = this->buttonLabelStartPos;
    rightButtonPos.x += this->rightButtonXOffset;
    rightButtonPos.y -= (static_cast<float>(this->rightButtons.size()) * (fontSize + pad));
	rightButton->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
    rightButton->setPosition(rightButtonPos);
    rightButton->addClickEventListener(callback);
    rightButton->setActionTag(rightActionTag);
    this->addChild(rightButton);
    this->rightButtons.push_back(rightButton);
}

void ButtonModifierNode::updateValue(const int index, const float value)
{
    if(isValidIndex(TYPE::VALUE, index))
    {
        this->valueLabels.at(index)->setString(std::to_string(value).substr(0, 3));
    }
}




SliderLabelNode* SliderLabelNode::createNode()
{
	SliderLabelNode* newNode = SliderLabelNode::create();
	return newNode;
}

bool SliderLabelNode::init()
{
	if (!cocos2d::Node::init())
	{
		return false;
	}

	return true;
}

void SliderLabelNode::addSlider(const std::string& labelStr, const std::string& sliderTextureName, const int percentage, const cocos2d::ui::Widget::ccWidgetClickCallback& callback)
{
	auto pos = this->sliderStartPos;
	float yOffset = 0;
	float size = static_cast<float>(this->sliderLabels.size());
	if (size != 0)
	{
		float prevSliderLabelSize = static_cast<float>(this->sliderLabels.back().label->getContentSize().height + this->sliderLabels.back().slider->getContentSize().height) - 20.0f;
		yOffset = size * prevSliderLabelSize;
	}

	//Slider Label
	auto newLabel = cocos2d::Label::createWithTTF("Simulation Speed", "fonts/Rubik-Medium.ttf", 25);
	newLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	pos.y -= yOffset;
	newLabel->setPosition(pos);
	this->addChild(newLabel);

	// Slider
	auto newSlider = cocos2d::ui::Slider::create();
	newSlider->loadBarTexture("sliderBar.png", cocos2d::ui::Widget::TextureResType::PLIST);
	newSlider->loadSlidBallTextures("sliderBallNormal.png", "sliderBallPressed.png", "sliderBallDisabled.png", cocos2d::ui::Widget::TextureResType::PLIST);
	newSlider->loadProgressBarTexture("sliderProgressBar.png", cocos2d::ui::Widget::TextureResType::PLIST);
	newSlider->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	auto labelPos = newLabel->getPosition();
	labelPos.y -= 20.0f;
	labelPos.x += 5.0f;
	newSlider->setPosition(labelPos);
	newSlider->setPercent(50);	// 50% equals to default speed
	newSlider->addClickEventListener(callback);
	this->addChild(newSlider);

	this->sliderLabels.push_back({ newLabel, newSlider });
}
