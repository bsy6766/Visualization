#include "EarClippingScene.h"

USING_NS_CC;

EarClippingScene* EarClippingScene::createScene()
{
	EarClippingScene* newEarClippingScene = EarClippingScene::create();
	return newEarClippingScene;
}

bool EarClippingScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}

	// Uncomment this to activate update(float) function
	//this->scheduleUpdate();

	return true;
}

void EarClippingScene::onEnter()
{
	cocos2d::Scene::onEnter();
	// Uncomment this to enable mouse and keyboard event listeners
	//initInputListeners();
}

void EarClippingScene::update(float delta)
{

}

void EarClippingScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(EarClippingScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(EarClippingScene::onMouseDown, this);
	this->mouseInputListener->onMouseUp = CC_CALLBACK_1(EarClippingScene::onMouseUp, this);
	this->mouseInputListener->onMouseScroll = CC_CALLBACK_1(EarClippingScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(EarClippingScene::onKeyPressed, this);
	this->keyInputListener->onKeyReleased = CC_CALLBACK_2(EarClippingScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void EarClippingScene::onMouseMove(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void EarClippingScene::onMouseDown(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void EarClippingScene::onMouseUp(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void EarClippingScene::onMouseScroll(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getScrollX();
	//float y = mouseEvent->getScrollY();
}

void EarClippingScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

}

void EarClippingScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

}

void EarClippingScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void EarClippingScene::onExit()
{
	cocos2d::Scene::onExit();
	// Uncomment this if you are using initInputListeners()
	//releaseInputListeners(); 
}