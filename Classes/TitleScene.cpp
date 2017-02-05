#include "TitleScene.h"
#include "MainScene.h"

USING_NS_CC;

TitleScene* TitleScene::createScene()
{
	TitleScene* newTitleScene = TitleScene::create();
	return newTitleScene;
}

bool TitleScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

    this->logo = cocos2d::Sprite::create("logo/cocos2dx_portrait.png");
	this->logo->setPosition(winSize * 0.5f);
	this->logo->setOpacity(0);
	this->addChild(this->logo);

	this->elapsedTime = 0;

	return true;
}

void TitleScene::onEnter()
{
	cocos2d::Scene::onEnter();
	initInputListeners();

	this->logo->runAction(cocos2d::FadeIn::create(0.5f));
}

void TitleScene::update(float delta)
{
	if (this->elapsedTime > this->time)
	{
		this->unscheduleUpdate();
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}
	else
	{
		this->elapsedTime += delta;

	}
}

void TitleScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(TitleScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(TitleScene::onKeyPressed, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void TitleScene::onMouseDown(cocos2d::Event* event) 
{
	cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
}

void TitleScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
}

void TitleScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void TitleScene::onExit()
{
	cocos2d::Scene::onExit();
	releaseInputListeners(); 
}
