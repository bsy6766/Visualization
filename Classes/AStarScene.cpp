#include "AStarScene.h"

USING_NS_CC;

AStarScene* AStarScene::createScene()
{
	AStarScene* newAStarScene = AStarScene::create();
	return newAStarScene;
}

bool AStarScene::init()
{
	if (!cocos2d::Scene::init())
	{
		return false;
	}

    this->scheduleUpdate();
    
    // init display boundary box node which draws outer line of simulation display box
    this->displayBoundaryBoxNode = DisplayBoundaryBoxNode::createNode();
    this->displayBoundaryBoxNode->setPosition(cocos2d::Vec2::ZERO);
    this->displayBoundaryBoxNode->displayBoundary = cocos2d::Rect(0, 0, 650, 650);
    this->displayBoundaryBoxNode->drawDisplayBoundaryBox();
    this->displayBoundaryBoxNode->retain();
    this->displayBoundaryBoxNode->drawNode->setLocalZOrder(static_cast<int>(Z_ORDER::BOX));
    this->addChild(this->displayBoundaryBoxNode);
    
    // Init labels node
    this->labelsNode = LabelsNode::createNode();
    this->labelsNode->setSharedLabelPosition(LabelsNode::SHARED_LABEL_POS_TYPE::RECT_PACKING_SCENE);
    this->addChild(this->labelsNode);
    
    auto winSize = cocos2d::Director::getInstance()->getVisibleSize();
    // Starting pos
    float labelX = winSize.height - 10.0f;
    float labelY = winSize.height - 45.0f;
    
    // Set title
    this->labelsNode->initTitleStr("A* Pathfinding", cocos2d::Vec2(labelX, labelY));

	return true;
}

void AStarScene::onEnter()
{
	cocos2d::Scene::onEnter();
	initInputListeners();
}

void AStarScene::update(float delta)
{

}

void AStarScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(AStarScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(AStarScene::onMouseDown, this);
	this->mouseInputListener->onMouseUp = CC_CALLBACK_1(AStarScene::onMouseUp, this);
	this->mouseInputListener->onMouseScroll = CC_CALLBACK_1(AStarScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(AStarScene::onKeyPressed, this);
	this->keyInputListener->onKeyReleased = CC_CALLBACK_2(AStarScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void AStarScene::onMouseMove(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void AStarScene::onMouseDown(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void AStarScene::onMouseUp(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void AStarScene::onMouseScroll(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getScrollX();
	//float y = mouseEvent->getScrollY();
}

void AStarScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

}

void AStarScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

}

void AStarScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void AStarScene::onExit()
{
	cocos2d::Scene::onExit();
	releaseInputListeners(); 
}
