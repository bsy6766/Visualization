#include "FlockingScene.h"
#include "MainScene.h"

USING_NS_CC;

FlockingScene* FlockingScene::createScene()
{
	FlockingScene* newFlockingScene = FlockingScene::create();
	return newFlockingScene;
}

bool FlockingScene::init()
{
	if (!CCScene::init())
	{
		return false;
	}

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	this->displayBoundary = cocos2d::Rect(0, 0, winSize.height, winSize.height);

	this->backLabel = cocos2d::Label::createWithTTF("BACK(ESC)", "fonts/Marker Felt.ttf", 30);
	this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 75.0f, 20.0f));
	this->addChild(this->backLabel);

	this->areaNode = cocos2d::Node::create();
	this->areaNode->setPosition(cocos2d::Vec2::ZERO);
	this->areaNode->retain();
	this->addChild(this->areaNode);

	this->qTreeLineNode = QTreeLineNode::createNode();
	this->qTreeLineNode->setPosition(cocos2d::Vec2::ZERO);
	this->qTreeLineNode->retain();
	this->addChild(this->qTreeLineNode);

	// Limit max entity to 100 in this case
	ECS::Entity::maxEntitySize = 100;

	return true;
}

void FlockingScene::onEnter()
{
	cocos2d::CCScene::onEnter();

	initInputListeners();

	initEntitiesAndQTree();
}

void FlockingScene::update(float delta)
{
	/*
	auto it = this->boids.begin();
	for (; it != this->boids.end();)
	{
		if ((*it)->alive == false)
		{
			delete (*it);
			it = this->boids.erase(it);
			continue;
		}

		std::list<Boids*> nearBoids;
		int boidId = (*it)->id;

		for (auto nearBoid : this->boids)
		{
			if (boidId != nearBoid->id)
			{
				if ((*it)->sprite->getPosition().distance(nearBoid->sprite->getPosition()) < Boids::SIGHT_RADIUS)
				{
					// in range
					nearBoids.push_back(nearBoid);
				}
			}
		}
	}
	*/
}

void FlockingScene::initInputListeners()
{
	this->mouseInputListener = EventListenerMouse::create();
	this->mouseInputListener->onMouseMove = CC_CALLBACK_1(FlockingScene::onMouseMove, this);
	this->mouseInputListener->onMouseDown = CC_CALLBACK_1(FlockingScene::onMouseDown, this);
	this->mouseInputListener->onMouseUp = CC_CALLBACK_1(FlockingScene::onMouseUp, this);
	this->mouseInputListener->onMouseScroll = CC_CALLBACK_1(FlockingScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->mouseInputListener, this);

	this->keyInputListener = EventListenerKeyboard::create();
	this->keyInputListener->onKeyPressed = CC_CALLBACK_2(FlockingScene::onKeyPressed, this);
	this->keyInputListener->onKeyReleased = CC_CALLBACK_2(FlockingScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(this->keyInputListener, this);
}

void FlockingScene::initEntitiesAndQTree()
{
	// Create 40 entities at start
	const int initialEntityCount = 40;
	for (int i = 0; i < initialEntityCount; i++)
	{
		auto newEntity = createNewEntity();
		if (newEntity != nullptr)
		{
			this->entities.push_back(newEntity);
		}
	}

	// Store lineNode pointer to quadtree for visualization
	QTree::lineNode = this->qTreeLineNode;
	// Init quadtree with initial boundary
	this->quadTree = new QTree(this->displayBoundary, 0);
}

ECS::Entity * FlockingScene::createNewEntity()
{
	Entity* newEntity = new Entity();
	if (newEntity->id >= Entity::maxEntitySize)
	{
		auto size = this->entities.size();
		if (size == Entity::maxEntitySize)
		{
			// Limit entities by 1000
			delete newEntity;
			return nullptr;
		}
		else
		{
			// We can spawn more entities but id exceeded 1000. reassign it.
			//reassignEntityIds();
			newEntity->id = size;
			Entity::idCounter = size + 1;
		}
	}

	// attach component and return
	newEntity->components[DIRECTION_VECTOR] = new ECS::DirectionVector();
	newEntity->components[SPRITE] = new ECS::Sprite(*this->areaNode, "quadTreeEntityBox.png");
	return newEntity;
}

void FlockingScene::onMouseMove(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	if (this->backLabel->getBoundingBox().containsPoint(cocos2d::Vec2(x, y)))
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

void FlockingScene::onMouseDown(cocos2d::Event* event) 
{
	auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	int mouseButton = mouseEvent->getMouseButton();
	float x = mouseEvent->getCursorX();
	float y = mouseEvent->getCursorY();

	auto point = cocos2d::Vec2(x, y);

	if (this->backLabel->getBoundingBox().containsPoint(point))
	{
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));

		return;
	}
}

void FlockingScene::onMouseUp(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//0 = left, 1 = right, 2 = middle
	//int mouseButton = mouseEvent->getMouseButton();
	//float x = mouseEvent->getCursorX();
	//float y = mouseEvent->getCursorY();
}

void FlockingScene::onMouseScroll(cocos2d::Event* event) 
{
	//auto mouseEvent = static_cast<EventMouse*>(event);
	//float x = mouseEvent->getScrollX();
	//float y = mouseEvent->getScrollY();
}

void FlockingScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{
	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		// Terminate 
		cocos2d::Director::getInstance()->replaceScene(cocos2d::TransitionFade::create(0.5f, MainScene::create(), cocos2d::Color3B::BLACK));
	}
}

void FlockingScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event) 
{

}

void FlockingScene::releaseInputListeners()
{
	if(this->mouseInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->mouseInputListener);
	if(this->keyInputListener != nullptr)
		_eventDispatcher->removeEventListener(this->keyInputListener);
}

void FlockingScene::onExit()
{
	cocos2d::CCScene::onExit();

	releaseInputListeners(); 
}