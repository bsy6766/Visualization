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

	ECS::Entity::idCounter = 0;

	// Uncomment this to activate update(float) function
	this->scheduleUpdate();

	auto winSize = cocos2d::Director::getInstance()->getVisibleSize();

	this->displayBoundary = cocos2d::Rect(0, 0, winSize.height, winSize.height);

	this->backLabel = cocos2d::Label::createWithTTF("BACK(ESC)", "fonts/Marker Felt.ttf", 30);
	this->backLabel->setPosition(cocos2d::Vec2(winSize.width - 75.0f, 20.0f));
	this->addChild(this->backLabel);

	this->weightLabel = cocos2d::Label::createWithTTF("WEIGHTS", "fonts/Marker Felt.ttf", 20);
	this->weightLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->weightLabel->setPosition(cocos2d::Vec2(winSize.height + 10.0f, winSize.height - 20.0f));
	this->addChild(this->weightLabel);

	this->alignmentWeightLabel = cocos2d::Label::createWithTTF("ALIGNMENT", "fonts/Marker Felt.ttf", 20);
	this->alignmentWeightLabel->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
	this->alignmentWeightLabel->setPosition(cocos2d::Vec2(winSize.height + 10.0f, winSize.height - 40.0f));
	this->addChild(this->alignmentWeightLabel);

	this->leftAlignmentButton = cocos2d::ui::Button::create("leftButton.png", "leftSelectedButton.png", "leftDisabledButton.png");
	this->leftAlignmentButton->addClickEventListener(CC_CALLBACK_1(FlockingScene::onButtonPressed, this));
	this->leftAlignmentButton->setTitleText("Button");
	this->leftAlignmentButton->setActionTag(ACTION_TAG::ALIGNMENT_LEFT);
	this->leftAlignmentButton->setPosition(cocos2d::Vec2(winSize.height + 200.0f, winSize.height - 40.0f));
	this->addChild(this->leftAlignmentButton);


	this->areaNode = cocos2d::Node::create();
	this->areaNode->setPosition(cocos2d::Vec2::ZERO);
	this->areaNode->retain();
	this->addChild(this->areaNode);

	this->lastTrackingBoidId = -1;

	this->rangeChecker = cocos2d::Sprite::createWithSpriteFrameName("boidRangeChecker.png");
	this->rangeChecker->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	this->rangeChecker->setVisible(false);
	this->rangeChecker->setOpacity(128);
	this->rangeChecker->setScale(ECS::FlockingObject::SIGHT_RADIUS * 2.0f / 100.0f);
	this->areaNode->addChild(rangeChecker);

	this->qTreeLineNode = QTreeLineNode::createNode();
	this->qTreeLineNode->setPosition(cocos2d::Vec2::ZERO);
	this->qTreeLineNode->retain();
	this->addChild(this->qTreeLineNode);

	// Limit max entity to 100 in this case
	ECS::Entity::maxEntitySize = 100;

	this->pause = false;

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
	if (!pause)
	{
		resetQTreeAndPurge();
		updateFlockingAlgorithm(delta);
	}
}

void FlockingScene::resetQTreeAndPurge()
{
	this->quadTree->clear();

	auto it = this->entities.begin();
	for (; it != this->entities.end();)
	{
		// Remove if entities is dead
		if ((*it)->alive == false)
		{
			delete (*it);
			it = this->entities.erase(it);
			continue;
		}

		if (pause)
		{
			// if simulation is paused, don't update entitie's position
			it++;
			continue;
		}

		// Re-insert to quadtree
		this->quadTree->insert((*it));

		// Reset color to white. Only boids
		if ((*it)->getComponent<ECS::FlockingObject*>(FLOCKING_OBJECT)->type == ECS::FlockingObject::TYPE::BOID)
		{
			(*it)->getComponent<ECS::Sprite*>(SPRITE)->sprite->setColor(cocos2d::Color3B::WHITE);
		}

		// next
		it++;
	}

	this->quadTree->showLines();
}

void FlockingScene::updateFlockingAlgorithm(const float delta)
{
	for (auto entity : entities)
	{
		auto entityFlockingObjComp = entity->getComponent<ECS::FlockingObject*>(FLOCKING_OBJECT);
		if (entityFlockingObjComp->type == ECS::FlockingObject::TYPE::BOID)
		{
			auto entitySpriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
			std::list<Entity*> nearEntities;

			cocos2d::Rect queryingArea = cocos2d::Rect::ZERO;
			float pad = ECS::FlockingObject::SIGHT_RADIUS;
			queryingArea.origin = (entitySpriteComp->sprite->getPosition() - cocos2d::Vec2(pad, pad));
			queryingArea.size = cocos2d::Vec2(pad * 2.0f, pad * 2.0f);

			this->quadTree->queryAllEntities(queryingArea, nearEntities);

			std::list<Entity*> nearBoids;
			std::list<Entity*> nearAvoids;

			for (auto nearEntity : nearEntities)
			{
				auto nearEntityFlockingObjComp = nearEntity->getComponent<ECS::FlockingObject*>(FLOCKING_OBJECT);
				auto nearBoidSpriteComp = nearEntity->getComponent<ECS::Sprite*>(SPRITE);
				auto entityPos = entitySpriteComp->sprite->getPosition();
				auto nearBoidPos = nearBoidSpriteComp->sprite->getPosition();
				float distance = nearBoidPos.distance(entityPos);
				if (nearEntityFlockingObjComp->type == ECS::FlockingObject::TYPE::BOID)
				{
					if (distance <= ECS::FlockingObject::SIGHT_RADIUS)
					{
						nearBoids.push_back(nearEntity);
						if (entityFlockingObjComp->tracking)
						{
							nearBoidSpriteComp->sprite->setColor(cocos2d::Color3B::GREEN);
						}
					}
					/*
					else
					{
						if (entityFlockingObjComp->tracking)
						{
							nearBoidSpriteComp->sprite->setColor(cocos2d::Color3B::RED);
						}
					}
					*/
				}
				else if (nearEntityFlockingObjComp->type == ECS::FlockingObject::TYPE::OBSTACLE)
				{
					if (distance <= ECS::FlockingObject::AVOID_RADIUS)
					{
						nearAvoids.push_back(nearEntity);
					}
				}
			}

			auto entityDirVecComp = entity->getComponent<ECS::DirectionVector*>(DIRECTION_VECTOR);

			cocos2d::Vec2 targetDirVec = entityDirVecComp->dirVec;

			cocos2d::Vec2 finalVec = cocos2d::Vec2::ZERO;
			cocos2d::Vec2 avoidVec = cocos2d::Vec2::ZERO;
			if (!nearAvoids.empty())
			{
				avoidVec = this->getAvoid(entity, nearAvoids);
				finalVec += (avoidVec * ECS::FlockingObject::AVOID_WEIGHT);
			}

			if (!nearBoids.empty())
			{
				cocos2d::Vec2 alignmentVec = this->getAlignment(entity, nearBoids) * ECS::FlockingObject::ALIGNMENT_WEIGHT;
				cocos2d::Vec2 cohesionVec = this->getCohesion(entity, nearBoids) * ECS::FlockingObject::COHENSION_WEIGHT;
				cocos2d::Vec2 separationVec = this->getSeparation(entity, nearBoids) * ECS::FlockingObject::SEPARATION_WEIGHT;
				finalVec += (alignmentVec + cohesionVec + separationVec);
			}

			finalVec.normalize();
			targetDirVec = finalVec;

			if (entityDirVecComp->smoothSteer)
			{
				// Steer boid's direction smooothly
				auto newDirVec = entityDirVecComp->dirVec;
				auto diffVec = targetDirVec - newDirVec;
				diffVec *= (delta * ECS::FlockingObject::steerSpeed);
				entityDirVecComp->dirVec += diffVec;
			}
			else
			{
				// Steer instantly
				entityDirVecComp->dirVec = targetDirVec;
			}

			auto movedDir = entityDirVecComp->dirVec * ECS::FlockingObject::movementSpeed;
			auto newPos = entitySpriteComp->sprite->getPosition() + movedDir;
			entitySpriteComp->sprite->setPosition(newPos);

			float angle = entityDirVecComp->getAngle();
			entitySpriteComp->sprite->setRotation(-angle);

			if (!this->displayBoundary.containsPoint(newPos))
			{
				entitySpriteComp->wrapPositionWithInBoundary(this->displayBoundary);
			}

			if (entityFlockingObjComp->tracking)
			{
				entitySpriteComp->sprite->setColor(cocos2d::Color3B::BLUE);
				this->rangeChecker->setPosition(entitySpriteComp->sprite->getPosition());
			}
		}
	}
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
	const int initialEntityCount = 100;
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
	/*
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
	*/

	// attach component and return
	auto dirVecComp = new ECS::DirectionVector();
	dirVecComp->smoothSteer = true;
	newEntity->components[DIRECTION_VECTOR] = dirVecComp;
	auto spriteComp = new ECS::Sprite(*this->areaNode, "boidEntity.png");
	const float angle = dirVecComp->getAngle();
	//cocos2d::log("id = %d, dirvec= (%f, %f), angle = %f", newEntity->id, dirVecComp->dirVec.x, dirVecComp->dirVec.y, angle);
	spriteComp->rotateToDirVec(-angle);
	spriteComp->setRandomPosInBoundary(this->displayBoundary);
	newEntity->components[SPRITE] = spriteComp;
	newEntity->components[FLOCKING_OBJECT] = new ECS::FlockingObject(ECS::FlockingObject::TYPE::BOID);

	return newEntity;
}

ECS::Entity * FlockingScene::createNewObstacleEntity(const cocos2d::Vec2& pos)
{
	Entity* newEntity = new Entity();
	auto spriteComp = new ECS::Sprite(*this->areaNode, "circle.png");
	spriteComp->sprite->setColor(cocos2d::Color3B::RED);
	spriteComp->sprite->setPosition(pos);
	newEntity->components[SPRITE] = spriteComp;
	newEntity->components[FLOCKING_OBJECT] = new ECS::FlockingObject(ECS::FlockingObject::TYPE::OBSTACLE);
	return newEntity;
}

const cocos2d::Vec2 FlockingScene::getAlignment(Entity* boid, std::list<Entity*>& nearBoids)
{
	cocos2d::Vec2 sumDirVec = cocos2d::Vec2::ZERO;

	const cocos2d::Vec2 boidPos = boid->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition();

	for (auto nearBoid : nearBoids)
	{
		auto dirVecComp = nearBoid->getComponent<ECS::DirectionVector*>(DIRECTION_VECTOR);
		sumDirVec += dirVecComp->dirVec;
	}

	const float countF = static_cast<float>(nearBoids.size());
	sumDirVec.x /= countF;
	sumDirVec.y /= countF;
	sumDirVec.normalize();
	return sumDirVec;
}

const cocos2d::Vec2 FlockingScene::getCohesion(Entity* boid, std::list<Entity*>& nearBoids)
{
	cocos2d::Vec2 sumPosVec = cocos2d::Vec2::ZERO;

	const cocos2d::Vec2 boidPos = boid->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition();

	for (auto nearBoid : nearBoids)
	{
		auto nearBoidSpriteComp = nearBoid->getComponent<ECS::Sprite*>(SPRITE);
		sumPosVec += nearBoidSpriteComp->sprite->getPosition();
	}

	const float countF = static_cast<float>(nearBoids.size());
	sumPosVec.x /= countF;
	sumPosVec.y /= countF;

	cocos2d::Vec2 cohesionVec = sumPosVec - boidPos;

	cohesionVec.normalize();

	return cohesionVec;
}

const cocos2d::Vec2 FlockingScene::getSeparation(Entity* boid, std::list<Entity*>& nearBoids)
{
	cocos2d::Vec2 sumDistVec = cocos2d::Vec2::ZERO;

	const cocos2d::Vec2 boidPos = boid->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition();

	for (auto nearBoid : nearBoids)
	{
		auto nearBoidSpriteComp = nearBoid->getComponent<ECS::Sprite*>(SPRITE);
		sumDistVec += (nearBoidSpriteComp->sprite->getPosition() - boidPos);
	}

	const float countF = static_cast<float>(nearBoids.size());
	sumDistVec.x /= countF;
	sumDistVec.y /= countF;

	sumDistVec *= -1;

	sumDistVec.normalize();

	return sumDistVec;
}

const cocos2d::Vec2 FlockingScene::getAvoid(Entity* boid, std::list<Entity*>& nearAvoids)
{
	int count = 0;
	cocos2d::Vec2 sumDistVec = cocos2d::Vec2::ZERO;

	const cocos2d::Vec2 boidPos = boid->getComponent<ECS::Sprite*>(SPRITE)->sprite->getPosition();

	for (auto nearAvoid : nearAvoids)
	{
		auto nearAvoidSpriteComp = nearAvoid->getComponent<ECS::Sprite*>(SPRITE);
		auto nearAvoidPos = nearAvoidSpriteComp->sprite->getPosition();
		cocos2d::Vec2 distVec = boidPos - nearAvoidPos;
		sumDistVec += distVec;
		count++;
	}

	if (count > 0)
	{
		const float countF = static_cast<float>(count);
		sumDistVec.x /= countF;
		sumDistVec.y /= countF;
		sumDistVec.normalize();
	}

	return sumDistVec;
}

void FlockingScene::onButtonPressed(cocos2d::Ref * sender)
{
	assert(true);
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

	if (mouseButton == 0)
	{
		// Left click
		if (this->displayBoundary.containsPoint(point))
		{
			// In display boundary
			for (auto entity : this->entities)
			{
				auto spriteComp = entity->getComponent<ECS::Sprite*>(SPRITE);
				if (spriteComp->sprite->getBoundingBox().containsPoint(point))
				{
					auto entityFlockingObjComp = entity->getComponent<ECS::FlockingObject*>(FLOCKING_OBJECT);

					// Clicked boid sprite
					if (entity->id == this->lastTrackingBoidId)
					{
						// Already tracking this boid. Disble tracking
						entityFlockingObjComp->tracking = false;
						this->lastTrackingBoidId = -1;
						this->rangeChecker->setVisible(false);
						return;
					}
					
					// New entity to track
					entityFlockingObjComp->tracking = true;

					if (this->lastTrackingBoidId != -1)
					{
						// Already tracking entity
						for (auto lastEntity : this->entities)
						{
							if (lastEntity->id == this->lastTrackingBoidId)
							{
								// Disable tracking on last tracking entitiy
								auto comp = lastEntity->getComponent<ECS::FlockingObject*>(FLOCKING_OBJECT);
								comp->tracking = false;
								break;
							}
						}
					}

					// Set this entity to new tracking entity
					this->lastTrackingBoidId = entity->id;

					this->rangeChecker->setVisible(true);
					break;
				}
			}
		}
	}
	else if (mouseButton == 2)
	{
		// Middle click
		if (this->displayBoundary.containsPoint(point))
		{
			this->entities.push_back(createNewObstacleEntity(point));
		}
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

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_SPACE)
	{
		// Terminate 
		this->pause = !this->pause;
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW)
	{
		float curAngle = this->entities.front()->getComponent<ECS::Sprite*>(SPRITE)->sprite->getRotation();
		curAngle += 10.0f;
		this->entities.front()->getComponent<ECS::Sprite*>(SPRITE)->sprite->setRotation(curAngle);
		cocos2d::log("curAngle = %f", curAngle);
	}

	if (keyCode == cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
	{
		float curAngle = this->entities.front()->getComponent<ECS::Sprite*>(SPRITE)->sprite->getRotation();
		curAngle -= 10.0f;
		this->entities.front()->getComponent<ECS::Sprite*>(SPRITE)->sprite->setRotation(curAngle);
		cocos2d::log("curAngle = %f", curAngle);
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