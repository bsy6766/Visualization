#include "CirclePackingScene.h"
#include "MainScene.h"

USING_NS_CC;

CirclePackingScene* CirclePackingScene::createScene()
{
	CirclePackingScene* newCirclePackingScene = CirclePackingScene::create();
	return newCirclePackingScene;
}

bool CirclePackingScene::init()
{
	if (!CCScene::init())
	{
		return false;
	}

	// Uncomment this to activate update(float) function
	//this->scheduleUpdate();

	initImages();

	return true;
}

void CirclePackingScene::initImages()
{
	int size = static_cast<int>(IMAGE_INDEX::MAX_SIZE);
	for (int i = 0; i < size; i++)
	{
		this->images.push_back(new cocos2d::Image());
		this->images.back()->autorelease();
		this->images.back()->retain();

		auto INDEX = static_cast<IMAGE_INDEX>(i);

		switch (INDEX)
		{
		case CirclePackingScene::DEAULT:
		{
			this->images.back()->initWithImageFile("Images/2017.png");
		}
			break;
		case CirclePackingScene::CAT:
			break;
		case CirclePackingScene::MAX_SIZE:
			break;
		default:
			break;
		}
	}
}

void CirclePackingScene::onEnter()
{
	cocos2d::CCScene::onEnter();

	initInputListeners();
}

void CirclePackingScene::update(float delta)
{

}

void CirclePackingScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(CirclePackingScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(CirclePackingScene::onMouseDown, this);
	this->mouseInputListener->onMouseUp = CC_CALLBACK_1(CirclePackingScene::onMouseUp, this);
	this->mouseInputListener->onMouseScroll = CC_CALLBACK_1(CirclePackingScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(CirclePackingScene::onKeyPressed, this);
	this->keyInputListener->onKeyReleased = CC_CALLBACK_2(CirclePackingScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void CirclePackingScene::onMouseMove(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void CirclePackingScene::onMouseDown(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void CirclePackingScene::onMouseUp(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void CirclePackingScene::onMouseScroll(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getScrollX();
	//float y = mouseEvent->getScrollY();
}

void CirclePackingScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}
}

void CirclePackingScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

}

void CirclePackingScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void CirclePackingScene::onExit()
{
	cocos2d::CCScene::onExit();
	// Uncomment this if you are using initInputListeners()
	releaseInputListeners(); 
}